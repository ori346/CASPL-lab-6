#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    char *command[] = {"ls", "-l", NULL};
    char *command2[] = {"tail", "-n", "2", NULL};
    int pipefd[2];
    int child1, child2, dup1, dup2, status1, status2;
    char debug = 0;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[1], "-d") == 0)
        {
            debug = 1;
        }
        else
            exit(1);
    }

    if (pipe(pipefd) < 0)
    {
        perror("error piping, exiting....\n");
        _exit(1);
    }

    if (debug)
        puts("parent_process>forking…\n");

    child1 = fork();
    if (debug && child1 != 0)
    {
        printf("parent_process>created process with id: %d\n", child1);
    }

    if (child1 == 0)
    {
        if (debug)
            puts("child1>redirecting stdout to the write end of the pipe\n");

        close(1); //close the stdoutput
        dup1 = dup(pipefd[1]);

        close(pipefd[1]);
        if (debug)
            fprintf(stderr, "going to execute cmd: %s\n", command[0]);
        if (execvp(command[0], command) < 0)
        {
            perror("execv faild on child 1");
        }
    }

    else
    {
        if (debug)
            puts("parent_process>closing the write end of the pipe…\n");
        close(pipefd[1]);

        if (debug)
            puts("parent_process>forking…\n");

        child2 = fork();
        if (debug && child2 != 0)
        {
            printf("parent_process>created process with id: %d\n", child2);
        }

        if (child2 == 0)
        {
            if (debug)
                puts("child2>redirecting stdin to the read end of the pipe\n");
            close(0); //close stdin
            dup2 = dup(pipefd[0]);
            close(pipefd[0]);
            if (debug)
                printf("going to execute cmd: %s\n", command2[0]);
            if (execvp(command2[0], command2) < 0)
            {
                perror("execv faild on child 2");
            }
        }
        else
        {
            if (debug)
                puts("parent_process>closing the read end of the pipe…\n");
            close(pipefd[0]);
            if (debug)

                puts("parent_process>waiting for child processes to terminate…\n");
            waitpid(child1, &status1, 0);
            waitpid(child2, &status2, 0);
            if (debug)

                puts("parent_process>exiting…\n");
        }
    }

    return 0;
}