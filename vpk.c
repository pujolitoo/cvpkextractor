#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>

#if defined(_WIN32)
    #include<windows.h>
    #include<tchar.h>
    #define RECDIRS(x) CreateDirectory(x, NULL)

#else
    #define RECDIRS(x) mkdir(x)     
#endif


typedef struct{
    unsigned int CRC; // checksum
    char* path; //virtual path
    char* folder; // folder
    unsigned short preload;
    unsigned short index; // vpk file part index
    unsigned int offset; // offset from pos 0 of the vpk file
    unsigned int lenght; // file size
} VPKEntry_t;

typedef struct{
    VPKEntry_t* array;
    unsigned int count;
    size_t size;
} VPKEntryList_t;

static unsigned int files = 0;

static unsigned int runs = 0;


char *readChars(FILE* file)
{
    int offset = 0;
    char bytechar;
    char* final = (char*)malloc(1);
    final[0] = 0;
    while(fread(&bytechar, sizeof(char), 1, file))
    {
        final = (char*)realloc(final, offset+1);
        final[offset] = bytechar;
        if(bytechar == '\0')
        {
            break;
        }
        offset++;
    }

    printf("LEN: %u\n", strlen(final));

    if(strlen(final)==0)
    {
        final = NULL;
    }

    return final;
}


VPKEntryList_t *createArray()
{
    VPKEntryList_t* list = malloc(sizeof(VPKEntryList_t));
    list->count = 0;
    list->array = NULL;
    list->size = 0;
    return list;

}

void addToArray(VPKEntryList_t* array, VPKEntry_t data)
{
    size_t newsize = sizeof(data);
    if(array->array == NULL)
    {
        array->array = (VPKEntry_t*)malloc(newsize);
    }
    array->size += newsize;
    array->array = (VPKEntry_t*)realloc(array->array, array->size);
    printf("%u\n", newsize);
    if(array->array == NULL)
    {
        printf("ERROR ADDING TO ARRAY. %s\n", strerror(errno));
        exit(1);
    }
    array->array[array->count] = data;
    array->count++;
}

void deleteArray(VPKEntryList_t* list)
{
    free(list->array);
    free(list);
}


int mkdirs(char* fPath, char* currentPath)
{
    char* folder = (char*)calloc(255, 1);

    if(fPath[strlen(currentPath)]=='\0') return 0;

    for(int i = (int)strlen(currentPath); i < strlen(fPath); i++)
    {
        unsigned int offset = (i-strlen(currentPath));
        folder[offset] = fPath[i];
        if(folder[offset]=='/' || folder[offset] == '\\')
        {
            folder[offset+1] = '\0';
            break;
        }else if (folder[offset] == '\0')
        {
            break;
        }
    }
    strcat(currentPath, folder);
    RECDIRS(currentPath);
    free(folder);
    return mkdirs(fPath, currentPath);
}


char* getFile(char* path, VPKEntry_t entry)
{
    char* data = malloc(entry.lenght);
    char* fullPath = (char*)malloc(255);
    if (entry.index >= 0)
    {
        sprintf(fullPath, "%s00%u.vpk", path, entry.index);
    }
    else if(entry.index >= 10)
    {
        sprintf(fullPath, "%s0%u.vpk", path, entry.index);
    }else if (entry.index >= 100 && entry.index < 1000)
    {
        sprintf(fullPath, "%s%u.vpk", path, entry.index);
    }else{
        printf("limit exeeded");
        exit(1);
    }

    FILE* vpkEntry = fopen(fullPath, "rb");
    if(!vpkEntry)
    {
        printf("ERROR WHILE READING VPK FILE");
        exit(1);
    }
    fseek(vpkEntry, entry.offset, SEEK_SET);
    size_t readed = fread(data, 1, entry.lenght, vpkEntry);
    fclose(vpkEntry);
    free(fullPath);
    return data;
}

char *deleteExtension(char* source, char* extension)
{
    char* currentPath = (char*)malloc(strlen(source));
    strcpy(currentPath, source);
    currentPath[(strlen(currentPath)-strlen(extension))] = '\0';
    return currentPath;
}

int main(int argc, char** argv)
{

    if(argc < 2)
    {
        printf("Please enter a vpk path.");
        exit(0);
    }

    char* dirPath = argv[1];
    char* absoluteName = deleteExtension(argv[1], "dir.vpk");

    FILE* vpkfile;
    
    vpkfile = fopen(dirPath, "rb");

    unsigned int signature;

    unsigned int version;

    unsigned int treeSize;


    fread(&signature, sizeof(unsigned int), 1, vpkfile); // SIGNATURE
    fread(&version, sizeof(unsigned int), 1, vpkfile);  // VPK VERSION
    fread(&treeSize, sizeof(unsigned int), 1, vpkfile); // TreeSize
    fseek(vpkfile, sizeof(unsigned int)*7, SEEK_SET); // SKIPPING UNECESSARY STUFF FROM FILE

    char* extension;
    char* folder;
    char* filename;

    VPKEntryList_t* list = createArray();


    while(1) // extension
    {
        extension = readChars(vpkfile);
        printf("------EXTENSION READ START .%s-----------\n", extension);
        if(extension == NULL)
        {
            break;
        }
        while(1) // folders
        {
            folder = readChars(vpkfile);
            printf("------FOLDER READ START----------- %s\n", folder);
            if(folder == NULL)
            {
                break;
            }
            while(1) // Filename
            {

                printf("------FILE READ START-----------\n");
                filename = readChars(vpkfile);
                if(filename == NULL)
                {
                    break;
                }

                VPKEntry_t file;
                fread(&file.CRC, sizeof(unsigned int), 1, vpkfile);
                file.path = malloc(strlen(folder) + strlen(filename) + strlen(extension) +1);
                sprintf(file.path, "%s/%s.%s", folder, filename, extension);
                file.folder = malloc(strlen(folder)+1);
                sprintf(file.folder, "%s", folder);
                fread(&file.preload, 2, 1, vpkfile);
                fread(&file.index, sizeof(unsigned short), 1, vpkfile);
                fread(&file.offset, sizeof(unsigned int), 1, vpkfile);
                fread(&file.lenght, sizeof(unsigned int), 1, vpkfile);
                fseek(vpkfile, sizeof(unsigned short), SEEK_CUR);
                printf("SIZE: %u\n", sizeof(unsigned int)*3 + sizeof(unsigned short)*3);
                addToArray(list, file);
                printf("PATH: %s\n", file.path);

                files++;

                printf("------FILE READ END-----------\n");
                filename = NULL;

            }
            printf("------FOLDER READ END-----------\n");
            folder = NULL;
        }
        printf("------EXTENSION READ END-----------\n\n");
        extension = NULL;
    }

    fclose(vpkfile);

    printf("FILES READED: %u\n\n", list->count);

    printf("EXTRACTING...\n");


    printf("ANAME: %s\n", absoluteName);

    char* baseOutput = "./vpk_extracted/";
    for(int i = 0; i < list->count; i++)
    {

        char* outputPath = (char*)malloc(255);
        sprintf(outputPath, "%s%s", baseOutput, list->array[i].path);
        printf("%s\n", outputPath);

        char* folder = (char*)malloc(255);
        sprintf(folder, "%s%s", baseOutput, list->array[i].folder);

        char* currentPath = (char*)calloc(255, 1);

        mkdirs(folder, currentPath);


        //output file
        FILE* output = fopen(outputPath, "wb");
        char* data;
        if(list->array[i].preload != 0)
        {
            fwrite(list->array[i].preload, sizeof(list->array[i].preload), 1, output);
        }else if (list->array[i].lenght != 0)
        {
            data = getFile(absoluteName, list->array[i]);
            fwrite(data, sizeof(char), list->array[i].lenght, output);
        }

        fclose(output);
        free(outputPath);
        free(folder);
        free(currentPath);
        free(data);
    }

    getchar();

    return 0;
}