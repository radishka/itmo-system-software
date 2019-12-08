#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>

void write_out(char* message, size_t size) {
    if (write(STDOUT_FILENO, message, size) < 0) {
        perror("Write to stdout");
        exit(EXIT_FAILURE);
    }
}

size_t string_length(char* string) {
    size_t length = 0;

    while (*(string++)) length++;

    return length;
}

void head(int file, long long max_lines_count) {
    int read_result;
    long long lines = 1;
    char buffer[4096];

    for (;;) {
        if ((read_result = read(file, buffer, sizeof(buffer))) == 0) {
            break;
        }

        if (errno == EISDIR) {
            errno = 0;
            break;
        }

        if (read_result < 0) {
            perror("read from file");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < read_result; ++i) {
            write_out(&buffer[i], 1);

            if (buffer[i] == '\n') {
                lines++;

                if (lines > max_lines_count) {
                    return;
                }
            }
        }

    };
}

int main(int argc, char** argv) {
    int file = STDIN_FILENO;
    long long n = 10;
    char opt;

    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch(opt) {
            case 'n':
                n = atoll(optarg);
                break;
            default:
                write_out("USAGE: head [-n line-count] [files]\n", 37);
                return EXIT_FAILURE;
        }
    }

    if (n < 1) {
        perror("invalid option");
        exit(EXIT_FAILURE);
    }

    int files_count = argc - optind;

    if (files_count > 0) {
        for (int i = optind; i < argc; ++i) {
            file = ((*argv[i] == '-') && (string_length(argv[i]) == 1))
                   ? STDIN_FILENO
                   : open(argv[i], O_RDONLY | O_LARGEFILE);
            if (file < 0) {
                perror("open file");
                continue;
            }

            if (files_count > 1) {
                write_out("==> ",4);
                write_out(argv[i], string_length(argv[i]));
                write_out(" <==\n",5);
            }

            head(file, n);

            if (files_count > 1 && i != argc - 1) {
                write_out("\n",1);
            }

            if (close(file) != 0) {
                perror("close file");
                exit(EXIT_FAILURE);
            }
        }
    } else {
        head(file, n);
    }

//    int lines;
//    scanf("%d", &lines);
//    char fileName[20];
//    scanf ("%char",fileName);
//    int file = open(fileName, O_RDONLY);
//    close(file);

    return EXIT_SUCCESS;
}
