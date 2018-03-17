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
        char *new_name = NULL;
        char *ext = NULL;

        // Look for extension
        for (int i = 0; name[i] != '\0'; ++i) {
            if (name[i] == '.') {

                // basename
                new_name = malloc((i + 1) * sizeof(char));
                new_name[i] = '\0';
                strncpy(new_name, name, i);

                // extension
                int len = strlen(name) - (i + 1);
                ext = malloc((len + 1) * sizeof(char));
                ext[len] = '\0';
                strncpy(ext, &name[i+1], len);

                break;
            }
        }

        if (!new_name || !ext || (strcmp("vm", ext) != 0)) {
            fprintf(stderr,
                    "Invalid filename '%s' provided. Extension must be .vm\n", name);
            exit(1);
        }

        free(ext);

        fl->name = new_name;

    } else {
        if (fl->next) {
            add_file(fl->next, name);
        } else {
            fl->next = new_file_list();
            add_file(fl->next, name);
        }
    }
}
