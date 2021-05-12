#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "LineParser.h"
#include <fcntl.h>

#define MAX_LEN 2048
#define arrayLen 10

void execute(cmdLine *pCmdLine)
{
    int check;
    if (pCmdLine->inputRedirect != NULL)
    {
        close(0);
        open(pCmdLine->inputRedirect, O_RDONLY, 0777);
    }
    if (pCmdLine->outputRedirect != NULL)
    {
        close(1);
        open(pCmdLine->outputRedirect, O_RDWR | O_CREAT, 0777);
    }

    check = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
    if (check == -1)
    {
        perror("Error, execution failed\n");
        freeCmdLines(pCmdLine);
        _exit(1);
    }
    exit(0);
}

int main(int argc, char **argv)
{
    int cpid, status, pOpperation = 0, pipeStatus = 0, pipefd[2];
    int debug = 0;

    char *history[arrayLen];
    int historyIndex = 0;
    for (int i = 0; i < arrayLen; i++)
    {
        history[i] = calloc(256, 1);
    }

    char buf[PATH_MAX];
    for (int i = 1; i < argc; i++)
    {
        if (!strncmp(argv[1], "-d", 2))
        {
            debug = 1;
        }
        else
        {
            printf("Ilegal argument");
            exit(1);
        }
    }
    char input[MAX_LEN];
    cmdLine *userCmd;

    while (1)
    {
        getcwd(buf, PATH_MAX);
        fputs(buf, stdout);
        fputc('\n', stdout);
        fgets(input, MAX_LEN, stdin);
        

        if (input[0] == '!')
        {
            int num = atoi(input + 1);
            if (num >= historyIndex)
            {
                fprintf(stderr, "invalid index! %d %d\n", num, historyIndex);
                continue;
            }
            else
            {
                strncpy(history[historyIndex], history[num], 256);
                printf("commend: %s\n",history[num]);
                strcpy(input, history[num]);
                historyIndex++;
            }
        }
        else
        {
            strncpy(history[historyIndex], input, 256);
            historyIndex++;
        }

        userCmd = parseCmdLines(input);
        if (userCmd->next != NULL)
        {
            pipeStatus = 1;
            historyIndex--;
            pipe(pipefd);
        }

        if (strncmp(input, "quit", 4) == 0)
        {
            for(int i= 0 ; i < arrayLen;i++)
            {
                free(history[i]);
            }
            freeCmdLines(userCmd);
            _exit(0);
        }
        
       
        if (strncmp(userCmd->arguments[0], "history", 7) == 0)
        {
            for (int i = 0; i < arrayLen; i++)
            {
                if (history[i][0] == 0)
                {
                    break;
                }
                else
                {
                    printf("%d) %s", i, history[i]);
                }
            }
            freeCmdLines(userCmd);
        }

        else if (strncmp(userCmd->arguments[0], "cd", 2) == 0)
        {
            if (debug)
            {
                fprintf(stderr, "PID: %d Executing command: %s\n", getpid(), userCmd->arguments[0]);
            }
            pOpperation = chdir(userCmd->arguments[1]);
            if (pOpperation == -1)
            {
                perror("Error, execution failed\n");
                freeCmdLines(userCmd);
                _exit(0);
            }
            freeCmdLines(userCmd);
        }
        else if (pipeStatus)
        {
            int child1, child2, dup1, dup2, status1, status2;
            child1 = fork();
            if (debug && child1 != 0)
            {
                printf("parent_process>created process with id: %d\n", child1);
            }

            if (child1 == 0)
            {
                if (debug)
                    puts("child1>redirecting stdout to the write end of the pipe\n");

                close(1);
                dup1 = dup(pipefd[1]);

                close(pipefd[1]);
                if (debug)
                    fprintf(stderr, "going to execute cmd: %s\n", userCmd->arguments[0]);
                execute(userCmd);
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
                    close(0);
                    dup2 = dup(pipefd[0]);
                    close(pipefd[0]);
                    if (debug)
                        printf("going to execute cmd: %s\n", userCmd->next->arguments[0]);
                    execute(userCmd->next);
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
                    pipeStatus = 0;
                    historyIndex++;
                    freeCmdLines(userCmd);

                    if (debug)

                        puts("parent_process>exiting…\n");
                }
            }
        }

        else
        {
            cpid = fork();
            if (cpid == -1)
            {
                perror("Error, fork failed\n");
                _exit(-1);
            }
            else if (cpid == 0)
            {
                if (debug)
                {
                    fprintf(stderr, "PID: %d Executing command: %s\n", getpid(), userCmd->arguments[0]);
                }
                execute(userCmd);
            }

            else
            {
                if (userCmd->blocking)
                {
                    waitpid(cpid, &status, 0);
                }
            }
            freeCmdLines(userCmd);
        }
    }

    for (int j = 0; j < arrayLen; j++)
    {
        free(history[j]);
    }
    

    return 0;
}