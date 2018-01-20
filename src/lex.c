#include "lex.h"

#include <ctype.h>

#define BLOCK (1024)

TokenList *new_token_list() {
    TokenList *r = malloc(sizeof(TokenList));

    r->tok  = NULL;
    r->argc = 0;
    r->argv = NULL;
    r->next = NULL;

    return r;
}

TokenList *scan_stream(FILE *fp) {
    TokenList *r = new_token_list();

    TokenList *prev = NULL;
    TokenList *tl = r;

    int cc = 0;
    int cws = 16;
    char *cword = malloc(16 * sizeof(char));

    int c;
    while ((c = fgetc(fp)) != EOF) {

        // Remove comments
        if (c == '/') {
            if ((c = fgetc(fp)) == '/') {
                // Skip to end of line
                while (fgetc(fp) != '\n')
                    ; // nop

                ungetc('\n', fp);
                continue;
            }
        }

        if (isspace(c)) {

            if (c == '\n') {
                if (!tl || !(tl->tok)) {
                    continue;

                } else {
                    if (prev) {
                        prev->next = tl;
                    }
                    prev = tl;
                    tl = NULL;
                }
            }

            if (cc > 0) {

                // Resize and apply null terminator
                char *w = realloc(cword, cc + 1);
                w[cc + 1] = '\0';

                if (tl->tok) {
                    if (tl->argv) {
                        tl->argv = realloc(tl->argv, (tl->argc + 1) * sizeof(char*)); // Make space
                    } else {
                        tl->argv = malloc((tl->argc + 1) * sizeof(char*));
                    }

                    tl->argv[tl->argc] = w;
                    tl->argc++;

                } else {
                    tl->tok = w;
                }

                cc = 0;
                cword = malloc(16 * sizeof(char));
                cws = 16;
            }

            continue;
        }

        if (!tl) {
            tl = new_token_list();
        }

        if (cc >= cws) {
            cws += 16;
            cword = realloc(cword, cws);
        }

        cword[cc++] = c;
    }

    return r;
}
