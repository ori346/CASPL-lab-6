all:myshell

myshell: task2.c
	gcc -m32 -g -c -o task2.o task2.c 
	gcc -m32 -g -c -o LineParser.o LineParser.c
	gcc -g -m32 -o myshell task2.o LineParser.o

.PHONY:clean
clean:
	rm *.o myshell
