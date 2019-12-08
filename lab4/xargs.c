#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

int read_line(char **line, const int fd) {
    char c;

    char *tmp = malloc(sizeof(char));
    char *buf = malloc(sizeof(char));
    tmp[0] = 0;
    buf[0] = 0;

    int result = EOF;

    ssize_t read_bytes;

    while ((read_bytes = read(fd, &c, 1)) && c != '\n') {
        tmp = malloc((strlen(buf) + 1) * sizeof(char));
        strcpy(tmp, buf);
        tmp[strlen(buf)] = 0;
        if (strlen(buf) > 1) {
            free(buf);
        }

        buf = malloc((strlen(buf) + 1) * sizeof(char));
        strcpy(buf, tmp);
        buf[strlen(tmp)] = c;
        buf[strlen(tmp) + 1] = 0;

        free(tmp);
    }

    if (c == '\n') {
        result = !EOF;
    }

    if (read_bytes < 0) {
        return -1;
    }

    *line = buf;
    return result;
}

static char *append_line_break(const char *const str) {
    size_t len = strlen(str);
    char *tmp = malloc((len + 2) * sizeof(char));
    strcpy(tmp, str);

    tmp[len] = '\n';
    tmp[len + 1] = 0;

    return tmp;
}

static void write_line(const char *const line, const int fd) {
    char *tmp = append_line_break(line);
    if (write(fd, tmp, strlen(tmp)) == -1) {
        if (fd != STDERR_FILENO) {
            write_line(strerror(errno), STDERR_FILENO);
            exit(errno);
        }
    }
    free(tmp);
}

static void handle_errno() {
    write_line(strerror(errno), STDERR_FILENO);
    exit(errno);
}

int main(int argc, char *argv[]) {
    char *command = NULL;
    char *in_buf = NULL;

    if (argc > 1) {
        command = argv[1];
    }

    int read_line_code = !EOF;

    while (1) {
        read_line_code = read_line(&in_buf, STDIN_FILENO);

        if (read_line_code == EOF) {
            break;
        } else if (read_line_code < 0) {
            handle_errno();
        }

        if (command != NULL) {
            char *concat_buf = malloc((strlen(command) + strlen(in_buf) + 1) * sizeof(char));

            sprintf(concat_buf,"%s %s", command, in_buf);
            if (system(concat_buf) < 0) {
                handle_errno();
            }
        } else {
            write_line(in_buf, STDOUT_FILENO);
        }
    }

    return EXIT_SUCCESS;
}
