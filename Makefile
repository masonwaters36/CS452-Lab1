CC = gcc

all: myshell

myshell: myshell.c lex.yy.c
	$(CC) -lfl myshell.c lex.yy.c -o myshell
	
clean:
	$(RM) myshell *.o *~
