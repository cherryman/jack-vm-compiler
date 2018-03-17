.POSIX:

CC	= cc
CFLAGS	= -Wall -Wpedantic -std=c99 -g -O2
LDFLAGS = -lm
OBJ	= src/main.o src/lex.o src/write.o src/prog.o
BIN	= jackvmc


.PHONY:	all clean test


all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

clean:
	-rm $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
