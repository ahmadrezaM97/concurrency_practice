#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handle_sigint(int sig) {
    printf("\nCaught signal %d (SIGINT). Exiting gracefully...\n", sig);
    exit(0);
}

int main() {
    // Register handler for SIGINT (Ctrl+C)
    signal(SIGINT, handle_sigint);

    printf("Press Ctrl+C to trigger SIGINT...\n");

    while (1) {
        printf("Running...\n");
        sleep(1);
    }

    return 0;
}
