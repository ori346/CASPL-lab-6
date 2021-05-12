all:myshell

myshell: task3.c
	gcc -m32 -g -c -o task3.o task3.c 
	gcc -m32 -g -c -o LineParser.o LineParser.c
	gcc -g -m32 -o myshell task3.o LineParser.o

.PHONY:clean
clean:
	rm *.o myshell
