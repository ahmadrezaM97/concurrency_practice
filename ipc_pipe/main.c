#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {

    pid_t pid;

    int pipe_file_descriptors[2]; // 0 -> read end 1-> write end
    if (pipe(pipe_file_descriptors) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (pid > 0) {
        // parent process
        printf("this is partent process: %d\n", getpid());

        close(pipe_file_descriptors[0]); // closed unused read end

        const char *s = "Helloo, it's meee...";
        size_t length = strlen(s);
        sleep(2);
        int n = write(pipe_file_descriptors[1], s, length);
        if (n < 0) {
            perror("write");
            close(pipe_file_descriptors[1]);
            return EXIT_FAILURE;
        }

        close(pipe_file_descriptors[0]); // close write file descriptor and send
                                         // EOF
        // wait for the child to finish
        wait(NULL);

    } else {
        // child process
        printf("this is the child process %d \n", getpid());

        // close unused write-end file descriptor
        close(pipe_file_descriptors[1]);

        char buff[1024];
        int n = read(pipe_file_descriptors[0], buff, sizeof(buff) - 1);
        if (n == -1) {
            perror("read");
            return EXIT_FAILURE;
        }
        if (n == 0) {
            printf("no data EOF \n");
            return EXIT_FAILURE;
        }

        buff[n] = '\0';
        printf("data recevied from pipe %s %d \n", buff, n);

        close(pipe_file_descriptors[0]); // close read-end
    }

    return EXIT_SUCCESS;
}
