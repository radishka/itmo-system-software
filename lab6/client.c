#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#define PORT 8085

static void usage(char *name) {
    fprintf(stderr, "Usage: %s <host> <dir ...>\n", name);
    _exit(1);
}


int main(int argc, char *argv[]) {
    int sd = -1;
    struct sockaddr_in addr;
    struct hostent *host_entity;
    char ip[20];
    int i;
    char answer[1025];

    if (argc < 3) usage(argv[0]);

    if (NULL == (host_entity = gethostbyname(argv[1]))) {      /*netdb.h*/
        perror("gethostbyname");
        _exit(2);
    }

    if (NULL == (struct in_addr **) host_entity->h_addr_list) {
        fputs("Cannot find host", stderr);
        _exit(3);
    }
    strcpy(ip, inet_ntoa(*((struct in_addr **) host_entity->h_addr_list)[0])); /*inet.h - адрес двоичного представления в десятичный a.b.c.d*/

    if (-1 == (sd = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("socket");
        _exit(2);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    inet_aton(ip, &addr.sin_addr);

    if (-1 == connect(sd, (struct sockaddr *) &addr, sizeof(addr))) {
        perror("connect");
    }
    for (i = 2; i < argc; ++i) {
        write(sd, argv[i], strlen(argv[i]));
        write(sd, "\n", sizeof("\n") - 1);
        {
            ssize_t read_len;
            while (0 < (read_len = read(sd, answer, 1))) {
                write(STDIN_FILENO, answer, (size_t) read_len);
                if ('\0' == answer[read_len-1])
                    break;
            }
        }
    }
    write(sd, "//close\n", sizeof("//close\n"));
    close(sd);
}
