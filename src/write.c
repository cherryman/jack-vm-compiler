#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "prog.h"
#include "write.h"

#define STR(x) #x

#define P(str) fputs(STR(str\n), fp)
#define F(str, ...) fprintf(fp, STR(str\n), __VA_ARGS__)
#define C(str) fputs("// " STR(str\n), fp);


static void write_preamble(FILE *fp, FileList *fl);
static void write_arithmetic(FILE *fp, RType op);
static void write_stack(FILE *fp, CommandType cmd, Memory mem, long num, char *fname);


void write_file_list(FILE *fp, FileList *fl) {

    write_preamble(fp, fl); // TODO

    FileList *it;
    for (it = fl; it; it = it->next) {

        TokenList *inst;
        for (inst = it->tl; inst; inst = inst->next) {
            switch (inst->cmd) {
                case PUSH:
                case POP:
                    write_stack(fp,
                            inst->cmd, inst->argv[0].mem, inst->argv[1].num,
                            it->name);
                    break;

                case ARITHMETIC:
                    write_arithmetic(fp, inst->argv[0].op);
                    break;

                default: /* NOP */
                    break;
            }
        }
    }

    free_file_list(fl);
}


void write_preamble(FILE *fp, FileList *fl) {

    static const struct {
        char *seg;
        int addr;
    } regs[] = {
        { "SP",   256  },
        { "LCL",  300  },
        { "ARG",  400  },
        { "THIS", 3000 },
        { "THAT", 3010 },
    };

    C(PREAMBLE BEGIN);

    for (int i = 0; i < sizeof(regs)/sizeof(regs[0]); ++i) {
        F(@%d, regs[i].addr);
        P(D=A);
        F(@%s, regs[i].seg);
        P(M=D);
    }

    C(PREAMBLE END);
}

void write_arithmetic(FILE *fp, RType op) {

    P();
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
        F(D=M%cD, opsym);
        F(@__COMPARE_TRUE_%ld__, JCOUNT);

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
        F(@__COMPARE_END_%ld__, JCOUNT);
        P(0;JMP);

        // If true
        F((__COMPARE_TRUE_%ld__), JCOUNT);
        P(@SP);
        P(A=M-1);
        P(M=-1);

        F((__COMPARE_END_%ld__), JCOUNT);
        ++JCOUNT;
    } else {
        F(M=M%cD, opsym);
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
            dofree = 1;
            seg = malloc(sizeof(char) * ((int) floor(log10(num)) + 2));

            sprintf(seg, "R%ld", num + 5);
            break;

        case STATIC:
            dofree = 1;
            seg = malloc(
                    // Format is fname.num
                    // strlen for fname, +2 for . and \0, the rest for number length
                    sizeof(char) * (strlen(fname) + 2 + (int) (floor(log10(num)) + 1)));

            sprintf(seg, "%s.%ld", fname, num);
            break;

        case CONSTANT:
            // Handled below
            /* NOP */
            break;
    }

    switch (cmd) {
        case PUSH:
            P();
            C(PUSH);
            // Load num
            if (deref || mem == CONSTANT) {
                F(@%ld, num);
                P(D=A);
            }

            // Load register and dereference if necessary
            if (mem != CONSTANT) {
                F(@%s, seg);

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
            P();
            C(POP);
            // Store ptr for later use
            if (deref) {
                F(@%ld, num);
                P(D=A);
                F(@%s, seg);
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
                F(@%s, seg);
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
