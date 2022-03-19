#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

typedef struct{
    unsigned int CRC; // checksum
    char* path; //virtual path
    unsigned int preload;
    unsigned int index; // vpk file part index
    unsigned int offset; // offset from pos 0 of the vpk file
    unsigned int lenght; // file size
} VPKEntry_t;

typedef struct{
    VPKEntry_t* array;
    unsigned int count;
} VPKEntryList_t;

void skipBytes(size_t offset, FILE* file)
{
    fread(NULL, offset, 1, file);
    if(ferror(file))
    {
        printf("There was some error skipping bytes.");
    }
}


char *readChars(FILE* file)
{
    int offset = 0;
    char bytechar;
    char* final = (char*)malloc(1);
    while(fread(&bytechar, sizeof(char), 1, file))
    {
        final[offset] = bytechar;
        realloc(final, offset+1);
        offset++;
        if(bytechar == '\0')
        {
            break;
        }
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

}

void addToArray(VPKEntryList_t* array, VPKEntry_t data)
{
    size_t newsize;
    if(array->array == NULL)
    {
        array->array = malloc(sizeof(data));
        newsize = sizeof(data);
    }else{
        newsize = sizeof(*array->array)+sizeof(data);
    }
    VPKEntry_t* vpkarray = array->array;
    vpkarray = realloc(array->array, newsize);
    if(vpkarray == NULL)
    {
        printf("ERROR ADDING TO ARRAY. %s\n", strerror(errno));
        exit(1);
    }
    array->count++;
    vpkarray[array->count-1] = data;
    array->array = vpkarray;
}

int main(int argc, char** argv)
{

    if(argc < 2)
    {
        printf("Please enter a vpk path.");
        exit(0);
    }

    FILE* vpkfile;
    
    vpkfile = fopen(argv[1], "rb");

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
        if(extension == NULL)
        {
            break;
        }
        while(1) // folders
        {
            folder = readChars(vpkfile);
            if(folder == NULL)
            {
                break;
            }
            while(1) // Filename
            {
                filename = readChars(vpkfile);
                if(filename == NULL)
                {
                    break;
                }

            

                VPKEntry_t file;
                fread(&file.CRC, sizeof(unsigned int), 1, vpkfile);
                file.path = malloc(strlen(folder) + strlen(filename) + strlen(extension) +1);
                sprintf(file.path, "%s/%s.%s", folder, filename, extension);
                fread(&file.preload, 2, 1, vpkfile);
                fread(&file.index, sizeof(unsigned int), 1, vpkfile);
                fread(&file.offset, sizeof(unsigned int), 1, vpkfile);
                fread(&file.lenght, sizeof(unsigned int), 1, vpkfile);
                skipBytes(2, vpkfile);
                addToArray(list, file);
                printf("PATH: %s\n", file.path);

            }
        }
    }

    fclose(vpkfile);
    

    printf("uint size: %u\n", sizeof(unsigned int));

    printf("Signature: %u\n", signature);

    printf("Version: %u\n", version);

    printf("Tree size: %u\n", treeSize);



    return 0;
}