#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include "LineParser.h"
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_LEN 2048

void execute(cmdLine *pCmdLine)
{
    int check;
    const char *input,*output;
    if(pCmdLine->inputRedirect!=NULL)
    {
        input =pCmdLine->inputRedirect;
        close(0);
        open(pCmdLine->inputRedirect,O_RDONLY,0777);

    }
    if(pCmdLine->outputRedirect!=NULL)
    {
        output=pCmdLine->outputRedirect;
        close(1);
        open(pCmdLine->outputRedirect,O_RDWR | O_CREAT,0777);

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
    int cpid, status, pOpperation = 0;
    int debug = 0;
    char buf[PATH_MAX];
    for (int i = 1; i < argc; i++)
    {
        if (!strncmp(argv[1], "-d", 2))
        {
            debug = 1;
        }
        else
        {
            printf("Ileagal argument");
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

        if (strncmp(input, "quit", 4) == 0)
        {
            _exit(0);
        }
        userCmd = parseCmdLines(input);
        if (strncmp(userCmd->arguments[0], "cd", 2) == 0)
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
        }
        freeCmdLines(userCmd);
    }
    return 0;
}