.POSIX:

CC	= cc
CFLAGS	= -Wall -Wpedantic -std=c99
OBJ	= src/main.o src/lex.o


.PHONY:	all clean

all: vmtrans

vmtrans: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
