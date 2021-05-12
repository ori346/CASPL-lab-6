#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>



int main(int argc,char **argv)
{
    int pipefd[2],cpid=0,status;
    char message[6]="hello\n";
    char answer[6];

    if(pipe(pipefd)<0)
    {
        perror("failed piping\n");
        _exit(1);
    };

    cpid = fork();
    if(cpid==-1)
    {
        perror("error forking\n");
        _exit(1);
    }
    else if(cpid==0)
    {
        write(pipefd[1],message,6);
        exit(0);
    }
    else
    {
        waitpid(cpid,&status,0);

        read(pipefd[0],answer,6);
        close(pipefd[0]);
        close(pipefd[1]);
        printf("the string got: %s",answer);
        exit(0);
    }

return 0;
}