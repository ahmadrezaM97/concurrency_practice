#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("number of arguments: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argument %d: %s\n", i, argv[i]);
    }

    char *addr = argv[1];

    printf("addr %s\n", addr);

    FILE *f = fopen(addr, "r");
    if (f == NULL) {
        perror("Failed to open file");
        return 1;
    }

    char buff[20];
    size_t n = 0;
    while ((n = fread(buff, 1, sizeof(buff) - 1, f)) > 0) {
        buff[n] = '\0';
        printf("read %d : %s\n", n, buff);
    }

    if (ferror(f)) {
        perror("Error reading file");
        fclose(f);
        return 1;
    }

    fclose(f);

    return 0;
}
