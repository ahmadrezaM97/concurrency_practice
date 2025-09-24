#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    pid_t pid;
    int status;
    int x = 100;

    pid = fork();
    if (pid == 0) {
        printf("it's child process pid: %d parent_pid %d \n", getpid(),
               getppid());
        char *argv[] = {"ls", "-l", "/", NULL};
        execvp("ls", argv);
    } else if (pid > 0) {
        printf("it's parent process pid %d\n", getpid());
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
        }
        printf("child finished with %d\n", status);
    } else if (pid < 0) {
        printf("it's error\n");
    }

    return 0;
}
