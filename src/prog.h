typedef struct FileList {
    char *name;
    TokenList *tl;
    struct FileList *next;
} FileList;

FileList *new_file_list();
void free_file_list(FileList *fl);
void add_file(FileList *fl, char *name);
