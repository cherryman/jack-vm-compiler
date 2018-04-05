#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "prog.h"
#include "write.h"

static int PC = 0;
#define STR(x) #x

//#define P(str) fputs(STR(str\n), fp)
//#define PF(str, ...) fprintf(fp, STR(str\n), __VA_ARGS__)
//#define P(str) fprintf(fp, "%d: "STR(str\n), PC++);
//#define PF(str, ...) fprintf(fp, "%d: "STR(str\n), PC++, __VA_ARGS__)
#define P(str)       fprintf(fp, STR(str)"\t// %d\n", PC++);
#define PF(str, ...) fprintf(fp, STR(str)"\t// %d\n", __VA_ARGS__, PC++)
#define C(str)       fputs  ("// " STR(str\n), fp)
#define CF(str, ...) fprintf(fp, "// " STR(str\n), __VA_ARGS__)
#define LF(str, ...) fprintf(fp, "("STR(str)")\n", __VA_ARGS__)
#define N()          fputs  ("\n", fp)

const static char *reg_save_list[4] = { "LCL", "ARG", "THIS", "THAT" }; // 4 elem
const static int reg_save_list_len = 4;

static void write_preamble(FILE *fp, FileList *fl);
static void write_arithmetic(FILE *fp, RType op);
static void write_stack(FILE *fp, CommandType cmd, Memory mem, long num, char *fname);
static void write_label(FILE *fp, char *label);
static void write_goto(FILE *fp, CommandType cmd, char *label);
static void write_fn(FILE *fp, char *name, int varc);
static void write_ret(FILE *fp);
static void write_call(FILE *fp, char *name, int argc);


void write_file_list(FILE *fp, FileList *fl) {

    char *curr_fn = NULL;
    char *label = NULL;

    write_preamble(fp, fl); // TODO

    FileList *it;
    for (it = fl; it; it = it->next) {

        TokenList *inst;
        for (inst = it->tl; inst; inst = inst->next) {

            N();

            const CmdArg *argv = inst->argv;
            switch (inst->cmd) {
                case PUSH:
                case POP:
                    write_stack(fp,
                            inst->cmd, argv[0].mem, argv[1].num,
                            it->name);
                    break;

                case ARITHMETIC:
                    write_arithmetic(fp, argv[0].op);
                    break;

                case LABEL:
                case GOTO:
                case IF:
                    if (curr_fn) {
                        label = malloc(sizeof(char) *
                                       strlen(curr_fn) + strlen(argv[0].name) + 3);
                        strcpy(label, curr_fn);
                        strcat(label, "$");
                    } else {
                        label = malloc(sizeof(char) * strlen(argv[0].name + 2));
                        strcpy(label, "null$");
                    }

                    strcat(label, argv[0].name);

                    if (inst->cmd == LABEL)
                        write_label(fp, label);
                    else
                        write_goto(fp, inst->cmd, label);
                    break;

                case FUNCTION:
                    curr_fn = argv[0].name;
                    write_fn(fp, curr_fn, argv[1].num);
                    break;

                case RETURN:
                    write_ret(fp);
                    break;

                case CALL:
                    write_call(fp, argv[0].name, argv[1].num);
                    break;

                default: /* NOP */
                    break;
            }
        }
    }

    free_file_list(fl);
}


void write_preamble(FILE *fp, FileList *fl) {

    //static const struct {
    //    char *seg;
    //    int addr;
    //} regs[] = {
    //    { "SP",   256  },
    //    { "LCL",  300  },
    //    { "ARG",  400  },
    //    { "THIS", 3000 },
    //    { "THAT", 3010 },
    //};

    C(PREAMBLE BEGIN);

    // Set stack pointer
    P(@256);
    P(D=A);
    P(@SP);
    P(M=D);

    N();
    //write_call(fp, "Sys.init", 0);
    P(@Sys.init);
    P(0;JMP);

    //for (int i = 0; i < sizeof(regs)/sizeof(regs[0]); ++i) {
    //    PF(@%d, regs[i].addr);
    //    P(D=A);
    //    PF(@%s, regs[i].seg);
    //    P(M=D);
    //}

    C(PREAMBLE END);
}

void write_arithmetic(FILE *fp, RType op) {

    C(ARITHMETIC INSTRUCTION);

    static long JCOUNT = 0;

    // Dereference
    P(@SP);
    P(AM=M-1);

    // Single operand operations
    if (op == NOT) {
        P(M=!M);
        P(@SP);
        P(M=M+1);
        return;
    }

    if (op == NEG) {
        P(M=-M);
        P(@SP);
        P(M=M+1);
        return;
    }

    // Pop next item
    P(D=M);
    P(A=A-1);

    char opsym = 0;
    int comp = 0;
    switch(op) {
        case ADD:   opsym = '+'; break;

        case EQ:
        case GT:
        case LT:    comp = 1;
        case SUB:   opsym = '-'; break;

        case AND:   opsym = '&'; break;
        case OR:    opsym = '|'; break;

        default: /* UNREACHABLE */
            break;
    }

    // Comparison operators
    if (comp) {
        PF(D=M%cD, opsym);
        PF(@__COMPARE_TRUE_%ld__, JCOUNT);

        switch (op) {
            case EQ: P(D;JEQ); break;
            case LT: P(D;JLT); break;
            case GT: P(D;JGT); break;
            default:           break;
        }

        // If false
        P(@SP);
        P(A=M-1);
        P(M=0);
        PF(@__COMPARE_END_%ld__, JCOUNT);
        P(0;JMP);

        // If true
        LF(__COMPARE_TRUE_%ld__, JCOUNT);
        P(@SP);
        P(A=M-1);
        P(M=-1);

        LF(__COMPARE_END_%ld__, JCOUNT);
        ++JCOUNT;
    } else {
        PF(M=M%cD, opsym);
    }
}

void write_stack(FILE *fp, CommandType cmd, Memory mem, long num, char *fname) {

    int deref = 0, dofree = 0;
    char *seg = NULL;

    switch (mem) {
        case ARGUMENT: deref = 1; seg = "ARG";  break;
        case LOCAL:    deref = 1; seg = "LCL";  break;
        case THIS:     deref = 1; seg = "THIS"; break;
        case THAT:     deref = 1; seg = "THAT"; break;
        case POINTER:
            if      (num == 0)    seg = "THIS";
            else if (num == 1)    seg = "THAT";
            break;

        case TEMP:
        case STATIC:
            dofree = 1;

            int len;
            if (num <= 0) // log not defined for 0
                len = 2;
            else
                len = (int) floor(log10(num) + 2);

            seg = malloc(sizeof(char) * len);

            if (mem == STATIC)
                sprintf(seg, "%s.%ld", fname, num);
            else if (mem == TEMP)
                sprintf(seg, "R%ld", num + 5);

            break;

        case CONSTANT:
            // Handled below
            /* NOP */
            break;
    }

    switch (cmd) {
        case PUSH:
            C(PUSH);
            // Load num
            if (deref || mem == CONSTANT) {
                PF(@%ld, num);
                P(D=A);
            }

            // Load register and dereference if necessary
            if (mem != CONSTANT) {
                PF(@%s, seg);

                if (deref)
                    P(A=M+D);

                P(D=M);
            }

            // Push
            P(@SP);
            P(A=M);
            P(M=D);
            P(@SP);
            P(M=M+1);
            break;

        case POP:
            C(POP);
            // Store ptr for later use
            if (deref) {
                PF(@%ld, num);
                P(D=A);
                PF(@%s, seg);
                P(D=M+D);

                P(@R13);
                P(M=D);
            }

            // Pop
            P(@SP);
            P(A=M-1);
            P(D=M);

            if (deref) {
                P(@R13);
                P(A=M);
                P(M=D);
            } else {
                PF(@%s, seg);
                P(M=D);
            }

            // Decrement stack
            P(@SP);
            P(M=M-1);
            break;

        default: /* UNREACHABLE */
            break;
    }

    if (dofree)
        free(seg);
}

void write_label(FILE *fp, char *label) {
    LF(%s, label);
}

void write_goto(FILE *fp, CommandType cmd, char *label) {
    C(GOTO);
    if (cmd == IF) {
        P(@SP);
        P(AM=M-1);
        P(D=M);

        PF(@%s, label);
        P(D; JNE);

    } else if (cmd == GOTO) {
        PF(@%s, label);
        P(0; JEQ);
    }
}

void write_fn(FILE *fp, char *name, int varc) {
    CF(==== BEGIN FN $%s DEF ====, name);

    // Function label
    LF(%s, name);

    // Inc SP
    if (varc) {
        PF(@%d, varc);
        P(D=A);
        P(@SP);
        P(M=D+M);
    }
}

void write_ret(FILE *fp) {
    C(RETURN);

    // Prepare frame
    P(@LCL);
    P(D=M);
    P(@R14);
    P(M=D);

    // Store return
    P(@5);
    P(D=D-A);
    P(A=D); // Deref the stored pointer
    P(D=M); // Store return addr
    P(@R15);
    P(M=D);

    // Pop computed value to ARG
    P(@SP);
    P(AM=M-1);
    P(D=M);
    P(@ARG);
    P(A=M);
    P(M=D);
    P(D=A+1); // Set stack pointer
    P(@SP);
    P(M=D);

    for (int i = reg_save_list_len - 1; i >= 0; --i) {
        P(@R14);
        P(AM=M-1);
        P(D=M);
        PF(@%s, reg_save_list[i]);
        P(M=D);
    }

    // Return
    P(@R15);
    P(A=M);
    P(0; JMP);

    C(==== END FN DEF ====);
}

void write_call(FILE *fp, char *name, int argc) {

    static long CLLCOUNT = 0;

    CF(CALL $%s, name);

    // Save return addr
    PF(@__CALL_COUNT_%ld__, CLLCOUNT);
    P(D=A);
    P(@SP);
    P(A=M);
    P(M=D); // SP not incremented, inc comes in for loop

    // Save registers
    for (int i = 0; i < reg_save_list_len; ++i) {
        PF(@%s, reg_save_list[i]);
        P(D=M);
        P(@SP);
        P(AM=M+1);
        P(M=D);
    }

    // Inc SP once again
    P(@SP);
    P(M=M+1);

    PF(@%d, argc + 5 /* Number of pushed regs */);
    P(D=A);
    P(@SP);
    P(D=M-D);
    P(@ARG);
    P(M=D);

    // Set LCL to current SP
    P(@SP);
    P(D=M);
    P(@LCL);
    P(M=D);

    // GOTO
    PF(@%s, name);
    P(0; JMP);
    LF(__CALL_COUNT_%ld__, CLLCOUNT++);
}
