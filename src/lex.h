typedef enum {
    NONE,
    ARITHMETIC,
    PUSH,
    POP,
    LABEL,
    GOTO,
    IF,
    FUNCTION,
    RETURN,
    CALL,
} CommandType;

typedef enum {
    ARGUMENT,
    LOCAL,
    STATIC,
    CONSTANT,
    THIS,
    THAT,
    POINTER,
    TEMP,
} Memory;

typedef enum {
    ARG_NONE,
    ARG_CMD,
    ARG_MEMORY,
    ARG_NUM,
    ARG_NAME,
} CmdArgType;

typedef enum {
    ADD,
    SUB,
    NEG,
    EQ,
    GT,
    LT,
    AND,
    OR,
    NOT,
} RType;

typedef union {
    int num;
    char *name;
    Memory mem;
    RType op;
} CmdArg;

typedef struct TokenList {
    CommandType cmd;
    int argc;
    CmdArg *argv;
    struct TokenList *next;
} TokenList;


TokenList *new_token_list();
void free_token_list(TokenList *tl);
TokenList *scan_stream(FILE *fp);
