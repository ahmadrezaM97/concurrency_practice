#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFSIZE 1024
#define PORT 8082

int main(void) {
    printf("server with pid %d is running \n", getpid());
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd,    // socket file descriptor
                   SOL_SOCKET,   // level: socket-level option
                   SO_REUSEADDR, // option: allow reuse of local address
                   &opt,         // pointer to option value(non-zero = enable)
                   sizeof(opt))  // size of that option value
    ) {

        perror("setsockopt");
        return EXIT_FAILURE;
    }

    struct sockaddr_in srv_addr = {0};
    // htonl -> host to network long
    // htons -> host to network short
    srv_addr = (struct sockaddr_in){.sin_family = AF_INET,
                                    .sin_port = htons(PORT),
                                    .sin_addr = {.s_addr = htonl(INADDR_ANY)}};

    // bind
    if (bind(server_fd, (struct sockaddr *)(&srv_addr), sizeof(srv_addr))) {
        perror("bind");
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    struct sockaddr_in cli_addr = {0};
    socklen_t cli_len = sizeof(cli_addr);
    /*
      Use sockaddr_in / sockaddr_in6 / sockaddr_storage (or getaddrinfo) to
      construct the address because they contain the right fields and sizes.
        Cast to (struct sockaddr *) only when calling socket API functions —
      that’s the protocol-agnostic interface the API designers wanted.
    */

    while (true) {
        int client_fd =
            accept(server_fd, (struct sockaddr *)&cli_addr, &cli_len);
        if (client_fd < 0) {
            perror("accept");
            return EXIT_FAILURE;
        }

        printf("accept connection for df %d \n", client_fd);

        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            printf("this is the child process with pid %d for fd %d\n",
                   getpid(), client_fd);
            // close server file descriptor in the process child
            close(server_fd);

            int n;
            char buff[BUFFSIZE];
            while ((n = recv(client_fd, buff, sizeof(buff), 0)) > 0) {
                printf("here\n");
                ssize_t sent = 0;
                while (sent < n) {
                    ssize_t k = send(client_fd, buff + sent, n - sent, 0);
                    if (k < 0) {
                        perror("send");
                        return EXIT_FAILURE;
                    }
                    sent += k;
                }
            }
            if (n < 0) {
                perror("recv");
                return EXIT_FAILURE;
            }

            close(client_fd);
            printf("exit connection %d\n", client_fd);

            /*
             * _exit vs exit
             *  exit() -> runs C-library cleanup(flushes stdio, calls atexit
             * handlers, runs destructors) _exit() -> immediately terminates the
             * process without any of that cleanup
             *
             * I need to use _exit in a child fork() or anywhere you must
             * terminate immediately After fork() the child inherits copies of
             * stdio buffers, if the child call exit(), those buffers will be
             * flushed a second time, producing duplicate out put
             * */
            _exit(42);
        }

        if (pid > 0) {

            /*
              When a child process terminates, the kernel keeps a small
                process-table entry with the child’s exit status until the
                parent ca1ls wait()/waitpid() to collect that status

            the leftover entry is a zombie(appears as Z in ps).
            it use the PID table slot  but no other resoruce

            if the parent never reaps terminated children, zombies accumulate
            and eventually can exhaust the process table


              */
            printf("this is parent process and we closed the clined fd\n");
            // close(client_fd);
            // pid_t w = wait(NULL);
            // if(w==-1){
            //     perror("wait");
            //     return  EXIT_FAILURE;
            // }

            /*
             *
             * Loop + WNOHANG = non-blocking reaping of all already-dead
             * children. Reap terminated children without blocking. Repeatedly
             * call waitpid(-1, &status, WNOHANG) to collect all exited child
             * PIDs (returns >0 for a reaped child, 0 if none ready, -1 on
             * error). Preserve errno and avoid non-async-safe calls if used
             * inside a SIGCHLD handler.
             */
            int status;
            pid_t w;
            while ((w = waitpid(-1, &status, WNOHANG)) > 0) {
                /* child w reaped */
            }

            printf("parent process done waiting\n");
        }
    }
    return 0;
}
