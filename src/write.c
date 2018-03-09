#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "write.h"

#define STR(x) #x

#define P(str) fputs(STR(str\n), fp)
#define F(str, ...) fprintf(fp, STR(str\n), __VA_ARGS__)
#define C(str) fputs("// " STR(str\n), fp);


void write_preamble(FILE *fp);
void write_arithmetic(FILE *fp, RType op);
void write_stack(FILE *fp, CommandType cmd, Memory mem, long num);


void write_token_list(FILE *fp, TokenList *tl) {

    write_preamble(fp);

    TokenList *it;
    for (it = tl; it; it = it->next) {
        switch (it->cmd) {
            case PUSH:
            case POP:
                write_stack(fp, it->cmd, it->argv[0].mem, it->argv[1].num);
                break;

            case ARITHMETIC:
                write_arithmetic(fp, it->argv[0].op);
                break;

            default: /* NOP */
                break;
        }
    }

    free_token_list(tl);
}


void write_preamble(FILE *fp) {

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

void write_stack(FILE *fp, CommandType cmd, Memory mem, long num) {

    int deref = 0;
    char *seg = malloc(4 * sizeof(char));
    switch (mem) {
        case ARGUMENT: deref = 1; seg = "ARG";  break;
        case LOCAL:    deref = 1; seg = "LCL";  break;
        case THIS:     deref = 1; seg = "THIS"; break;
        case THAT:     deref = 1; seg = "THAT"; break;
        case TEMP:                sprintf(seg, "R%ld", num + 5); break;
        case POINTER:
            if      (num == 0)    seg = "THIS";
            else if (num == 1)    seg = "THAT";
            break;

        case STATIC:   deref = 1; //sprintf(seg, "%s.%ld", fname, num);
            // TODO: IMPLEMENT
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
}
