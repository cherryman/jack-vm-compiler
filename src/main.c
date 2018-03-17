#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "prog.h"
#include "write.h"


int main(int argc, char **argv) {

    FileList *fl = new_file_list();

    for (int i = 1; i < argc; ++i) {
        add_file(fl, argv[i]);
    }

    write_file_list(stdout, fl);

    return 0;
}
