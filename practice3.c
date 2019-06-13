#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void exec_A(char*);
void exec_B(char*);

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        printf("please type practice3 A for STATE_A or B for STATE_B\n");
        return -1;
    } else if (!strcmp(argv[1], "A")) {
        exec_A("PROCESS_A");
        exec_A("PROCESS_B");
        exec_A("PROCESS_C");
    } else if (!strcmp(argv[1], "B")) {
        exec_B("PROCESS_A");
        exec_B("PROCESS_B");
        exec_B("PROCESS_C");
    } else {
        printf("please type practice3 A for STATE_A or B for STATE_B\n");
    }

    return 0;
}

void exec_A(char* pName)
{
    pid_t pid = fork();
    if (pid == 0) {
        char* cwd = getcwd(NULL, 0);
        char* process = (char*) malloc(strlen(cwd) + strlen("/testbanker_stateA"));
        sprintf(process, "%s/%s", cwd, "testbanker_stateA");
        char* const argp[] = {process, pName, NULL};
        execvp(argp[0], argp);
        free(process);
    }
}

void exec_B(char* pName)
{
    pid_t pid = fork();
    if (pid == 0) {
        char* cwd = getcwd(NULL, 0);
        char* process = (char*) malloc(strlen(cwd) + strlen("/testbanker_stateB"));
        sprintf(process, "%s/%s", cwd, "testbanker_stateB");
        char* const argp[] = {process, pName, NULL};
        execvp(argp[0], argp);
        free(process);
    }
}
