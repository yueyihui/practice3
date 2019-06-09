#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void exec_child(char*, char*);

int main(int argc, const char *argv[])
{
    exec_child(argv[1], "PROCESS_A");
    exec_child(argv[1], "PROCESS_B");
    exec_child(argv[1], "PROCESS_C");
    return 0;
}

void exec_child(char* state, char* pName)
{
    pid_t pid = fork();
    if (pid == 0) {
        char* cwd = getcwd(NULL, 0);
        char* process = (char*) malloc(strlen(cwd) + strlen("/testebbchar"));
        sprintf(process, "%s/%s", cwd, "testebbchar");
        char* const argp[] = {process, state, pName, NULL};
        execvp(argp[0], argp);
        free(process);
    }
}
