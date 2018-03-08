#include <stdio.h>

#include "lex.h"
#include "write.h"

#define STR(x) #x

#define P(str) fputs(STR(str\n), fp)
#define F(str, ...) fprintf(fp, STR(str\n), __VA_ARGS__)

/*#define DREF(p) \*/
    /*P(@p);      \*/
    /*P(A=M);*/


void write_arithmetic(FILE *fp, RType op);
void write_stack(FILE *fp, CommandType cmd, Memory mem, long num);


void write_token_list(FILE *fp, TokenList *tl) {

    // Preamble
    P(@256);
    P(D=A);
    P(@SP);
    P(M=D);

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


void write_arithmetic(FILE *fp, RType op) {

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
        // Store stack pointer
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

    switch (cmd) {

        case PUSH:
            if (mem == CONSTANT) {
                // Load constant
                F(@%ld, num);
                P(D=A);

                // Push
                P(@SP);
                P(A=M);
                P(M=D);

                P(@SP);
                P(M=M+1);
            }

            break;

        case POP:
            // TODO: IMPLEMENT
            break;

        default: /* UNREACHABLE */
            break;
    }
}
