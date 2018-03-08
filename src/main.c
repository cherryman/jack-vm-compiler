#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "write.h"


int main(int argc, char **argv) {

    FILE *fi;
    TokenList *tl;

    if (argc < 2) {
        fprintf(stderr, "Insufficient arguments");
        exit(1);
    }

    fi = fopen(argv[1], "r");
    if (!fi) {
        fprintf(stderr, "Failed to read file %s", argv[1]);
        exit(1);
    }

    tl = scan_stream(fi);
    fclose(fi);

    write_token_list(stdout, tl);

    return 0;
}
