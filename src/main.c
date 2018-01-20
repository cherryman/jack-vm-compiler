#include <stdio.h>
#include <stdlib.h>

#include "lex.h"


int main(int argc, char **argv) {

    if (argc < 2) {
        puts("Insufficient arguments");
        exit(1);
    }


    FILE *fp = fopen(argv[1], "r");

    if (!fp) {
        printf("Failed to read file %s", argv[1]);
        exit(1);
    }

    TokenList *tl = scan_stream(fp);

    for (TokenList *it = tl ; it; it = it->next) {
        printf("%s", it->tok);

        for (int i = 0; i < it->argc; ++i) {
            printf(" %s", it->argv[i]);
        }

        puts("");
    }

    return 0;
}
