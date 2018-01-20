#include <stdio.h>
#include <stdlib.h>


typedef struct TokenList {
    char *tok;
    int argc;
    char **argv;
    struct TokenList *next;
} TokenList;


TokenList *scan_stream(FILE *fp);
