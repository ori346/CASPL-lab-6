all:mypipeline

mypipeline: task1.c
	gcc -m32 -g -c -o task1.o task1.c 
	gcc -m32 -g -c -o LineParser.o LineParser.c
	gcc -g -m32 -o mypipeline task1.o LineParser.o
	rm *.o
.PHONY:clean
clean:
	rm  mypipeline
