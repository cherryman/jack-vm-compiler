.POSIX:

CC	= cc
CFLAGS	= -Wall -Wpedantic -std=c99 -g -O2
LDFLAGS = -lm

SRC	= src/main.c src/lex.c src/write.c src/prog.c
OBJ	= $(SRC:.c=.o)
BIN	= jackvmc


.PHONY:	all clean test


all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

clean:
	-rm $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
