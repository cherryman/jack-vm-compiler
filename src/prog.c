#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "prog.h"


FileList *new_file_list() {

    FileList *r = malloc(sizeof(FileList));

    r->name = NULL;
    r->tl   = NULL;
    r->next = NULL;

    return r;
}

void free_file_list(FileList *fl) {
    FileList *n;

    if (fl) {
        n = fl->next;

        free(fl->name);
        free_token_list(fl->tl);
        free(fl);

        if (n)
            free_file_list(n);
    }
}

void add_file(FileList *fl, char *name) {

    // Check if this is the last item
    if (!fl->next && !fl->name) {

        // Load token list
        FILE *fi = fopen(name, "r");

        if (!fi) {
            fprintf(stderr, "Failed to load file '%s'\n", name);
            exit(1);
        }

        fl->tl = scan_stream(fi);

        fclose(fi);

        // Load filename
        char *basename = NULL;
        char *ext = NULL;

        // Look for extension
        int base = 0;
        for (int i = 0; name[i] != '\0'; ++i)
            if (name[i] == '/') // NOTE: Windows path separator unsupported ATM
                base = i + 1;

        for (int i = base; name[i] != '\0'; ++i) {
            if (name[i] == '.') {

                int len = 0;

                // basename
                len = i - base;
                basename = malloc((len + 1) * sizeof(char));
                basename[len] = '\0';
                strncpy(basename, &name[base], len);

                // extension
                len = strlen(name) - (i + 1);
                ext = malloc((len + 1) * sizeof(char));
                ext[len] = '\0';
                strncpy(ext, &name[i+1], len);

                break;
            }
        }

        // TODO: fix extension checking
        if (!basename || !ext || (strcmp("vm", ext) != 0)) {
            fprintf(stderr,
                    "Invalid filename '%s' provided. Extension must be .vm\n", name);
            exit(1);
        }

        free(ext);

        fl->name = basename;

    } else {
        if (fl->next) {
            add_file(fl->next, name);
        } else {
            fl->next = new_file_list();
            add_file(fl->next, name);
        }
    }
}
