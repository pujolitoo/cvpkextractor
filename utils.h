#include <string.h>

char *strremove(char *str, const char *sub) {
    char *p, *q, *r;
    if (*sub && (q = r = strstr(str, sub)) != NULL) {
        size_t len = strlen(sub);
        while ((r = strstr(p = r + len, sub)) != NULL) {
            memmove(q, p, r - p);
            q += r - p;
        }
        memmove(q, p, strlen(p) + 1);
    }
    return str;
}

char* removeLastN(char* str, size_t n ) {
    size_t strLen = strlen(str);
    str[n <= strLen ? strLen-n : 0] = '\0';
    return str;
}