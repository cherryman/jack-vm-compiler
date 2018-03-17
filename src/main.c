#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "prog.h"
#include "write.h"


int main(int argc, char **argv) {

    FileList *fl = new_file_list();
    char *fname = NULL;
    FILE *fo;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            for (char *a = (argv[i] + 1); a && *(a) != '\0' ; ) {
                switch (*a) {
                    case 'o':
                        if (*(a + 1) != '\0') {
                            fname = a + 1;
                            a = NULL;

                        } else if (argv[i + 1] && argv[i + 1][0] != '-') {
                            fname = argv[++i];
                            a = NULL;

                        } else {
                            fprintf(stderr,
                                    "Error: -o option requires filename\n");
                            exit(1);
                        }

                        break;

                    case 'h':
                        printf(
                            "%s [OPTIONS] [FILES] ...\n"
                            "\n"
                            "Options:\n"
                            "   -h  Print this help.\n"
                            "   -o  Output file. Print to stdout if none provided.\n"

                            , argv[0]
                        );

                        exit(1);
                        break;

                    case '-':

                        if (*(a + 1) != '\0') {
                            fprintf(stderr,
                                    "Long arguments not supported\n");
                            a = NULL;

                        } else {
                            for (++i; i < argc; ++i)
                                add_file(fl, argv[i]);
                        }
                        break;

                    default:
                        fprintf(stderr,
                                "Invalid option -%c, ignoring\n", *a);
                        break;
                }

                if (a) ++a;
            }
        } else {
            add_file(fl, argv[i]);
        }
    }

    if (!fl->tl) {
        fprintf(stderr,
                "No input files given\n");
        exit(1);
    }

    if (fname) {
        fo = fopen(fname, "w"); // TODO: check if file exists

        if (!fo) {
            fprintf(stderr,
                    "Failed to open file '%s' for reading\n",
                    fname);
        }

    } else {
        fo = stdout;
    }

    write_file_list(fo, fl);

    fclose(fo);

    return 0;
}
