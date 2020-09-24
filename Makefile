CC = gcc

all: myshell

myshell: myshell.c lex.c
	flex lex.c
	$(CC) -w lex.yy.c myshell.c -o myshell -lfl
	
clean:
	$(RM) myshell *.o lex.yy.c
