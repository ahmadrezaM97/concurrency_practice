#include <errno.h>
#include <stdio.h>
#include <unistd.h>

void run_1() {

    char name[4];   // Declare string to hold returned value
    size_t len = 3; // purposly too small so error is revleaded

    int return_value;

    if (gethostname(name, len) == -1) {
        switch (errno) {
        case EINVAL:
            printf("EINVAL: The len argument is zero or exceeds the "
                   "implementation-defined limit.\n");
            break;
        case ENAMETOOLONG:
            printf("ENAMETOOLONG: The hostname is longer than len bytes.\n");
            break;
        case EPERM:
            printf("EPERM: The process does not have appropriate privileges to "
                   "perform this operation.\n");
            break;
        default:
            printf("An unknown error occurred: %d\n", errno);
            break;
        }
    } else {
        printf("Hostname: %s\n", name);
    }
}

void run_2() {

    char name[4];   // Declare string to hold returned value
    size_t len = 3; // purposly too small so error is revleaded

    int return_value;
    if (gethostname(name, len) == -1) {
        switch (errno) { perror("gethostname failed"); }
    } else {
        printf("Hostname: %s\n", name);
    }
}
int main(void) {
    run_1();
    return 0;
}
