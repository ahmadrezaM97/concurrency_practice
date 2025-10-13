// File control options:
// Provides functions and macros for controlling file descriptors.
// open(), fcntl()
// enabling non-blocking I/O, locking files, or duplicating file descriptors.
#include <errno.h>
#include <fcntl.h>

#include <stdbool.h>

// Standard Input/Output:
// Provides standard I/O operations like reading/writing to files or console.
// Common functions: printf(), scanf(), fopen(), fclose(), fprintf(), fgets(),
#include <stdio.h>

// POSIX API for system calls:  Provides access to the operating system’s API
// for low-level I/O and process control. Common functions: read(), write(),
// close(), fork(), exec(), sleep(), getpid() Use cases: working directly with
// file descriptors, creating processes, or handling low-level input/output
// operations.
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

typedef struct {
    char *data;
    size_t length;
} Content;

Content *simple_read(const char *addrs) {
    printf("Reading file: %s\n", addrs);
    int fd = open(addrs, O_RDONLY);
    if (fd == -1) {
        if (errno == EACCES) {
            fprintf(stderr, "Permission denied to open file: %s\n", addrs);
        } else if (errno == ENOENT) {
            fprintf(stderr, "File not found: %s\n", addrs);
        } else {
            fprintf(stderr, "Error opening file %s: %s\n", addrs,
                    strerror(errno));
        }
        perror("Failed to open file");
        return NULL;
    }

    /*
        int fstat(int fildes, struct stat *buf);

        The stat() function obtains information about the file pointed to by
            path. Read, write or execute permission of the named file is not
            required, but all directories listed in the path name leading to the
            file must be searchable.
    */

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Failed to get file size");
        close(fd);
        return NULL;
    }

    printf("File size: %lld bytes\n", (long long)st.st_size);

    size_t total_bytes = 0;
    char *data = (char *)malloc(st.st_size); // Allocate 1MB for file content
    if (data == NULL) {
        perror("Failed to allocate memory");
        close(fd);
        return NULL;
    }

    int bytes_read;
    while ((bytes_read =
                read(fd, data + total_bytes, st.st_size - total_bytes)) > 0) {
        total_bytes += bytes_read;
    }
    if (bytes_read == -1) {
        perror("Error reading from stdin");
        return NULL;
    }

    Content *result = (Content *)malloc(sizeof(Content));
    if (result == NULL) {
        perror("malloc result failed");
        return NULL;
    }
    *result = (Content){.data = data, .length = total_bytes};

    close(fd);
    return result;
}

int simple_write(char const *addr, char const *content, size_t content_size) {

    int fd = open(addr, O_WRONLY | O_CREAT);
    if (fd == -1) {
        perror("open failed");
    }

    size_t total_bytes_sent = 0;
    while (total_bytes_sent < content_size) {
        int n = write(fd, content + total_bytes_sent,
                      content_size - total_bytes_sent);
        printf("-- write %d bytes\n", n);
        if (n > 0) {
            total_bytes_sent += n;
            continue;
        }

        if (n <= 0) {
            perror("write failed");
            return -1;
        }
    }

    close(fd);
    return 0;
}
int main(void) {
    char *file_path = "./low_level_io/test.txt";
    Content *res = simple_read(file_path);
    if (res == NULL) {
        printf("content is null\n");

        return -1;
    }

    int err = simple_write("./low_level_io/copied.txt", res->data, res->length);
    if (err != 0) {
        printf("simple write failed\n");

        return -1;
    }

    return 0;
}
