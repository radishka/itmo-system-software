#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

#include "queue.h"

#define N 3
#define K 5
#define PORT 8085

static int sd = -1;
static size_t pool_size;        /*всего*/
static size_t free_threads;     /*из них свободных*/

static pthread_mutex_t queue_mutex;
static pthread_mutex_t pool_size_mutex;
static pthread_mutex_t free_threads_mutex;
static pthread_cond_t client_came;

static struct queue * clients;

static void stop_server() {
    int client;

    puts("[Close all connections]");
    while (-1 != (client = queue_dequeue(clients))) {
        close(client);
    }
    queue_destroy(clients);
    puts("[Close socket descriptor]");
    close(sd);
    puts("[Close mutexes]");
    pthread_cond_destroy(&client_came);
    pthread_mutex_destroy(&queue_mutex);
    puts("[Stop server]");
    _exit(0);
}


static void sighandler(int s) {
    signal(s, sighandler);
    puts("[Signal caught]");
    stop_server();
}


static void process_dir(int client_sd) {
    char *buf = NULL;

    DIR* dir;
    struct dirent* dirent;
    size_t d_name_len, i;

    while (1) {
        puts("[work cycle start]");

        char *new_buf = NULL;
        size_t buf_size = 1024;
        size_t len = 0;
        char c = '\0';

        if (NULL == (buf = (char *) malloc(buf_size))) {
            goto clean;
        }

        /*read till \n*/
        while ('\n' != c) {
            if (-1 == read(client_sd, &c, 1)) {
                goto clean;
            }
            buf[len++] = c;
            if ((buf_size - len) <= 2) {
                if (NULL == (new_buf = (char *) realloc(buf, buf_size * 2))) {
                    goto clean;
                }
                buf = new_buf;
                buf_size *= 2;
            }
        }

        if (!strcmp(buf, "//close\n")) {
            puts("[Close-command received]");
            free(buf);
            return;
        }

        for (i = 0; i <= len; ++i) {
            if (buf[i] == '\n' || buf[i] == '\r') {
                buf[i] = '\0';
                len = i;
                break;
            }
        }

        write(STDOUT_FILENO, buf, sizeof(buf));
        printf(" - ");

        write(client_sd, buf, len);
        write(client_sd, ":\n", 2);
        
        errno = 0;
        dir = opendir(buf);
        if (NULL != dir) {
            printf("dir opened:\n");
            /*watch all files in dir*/
            while (1) {
                dirent = readdir(dir);
                /* error and end of dir handling */
                if (dirent == NULL) {
                    puts(" !dirent is NULL!");
                    if (errno == 0) {
                        /* end of dir found */
                        break;
                    } else {
                        /* error handling */
                        errno = 0;
                        /* continue reading */
                        continue;
                    }
                }
                /* output dirent name */
                /* remove \n symbols */
                printf("%s, ", dirent->d_name);
                d_name_len = strlen(dirent->d_name);
                for (i = 0; i < d_name_len; i++) {
                    if (dirent->d_name[i] == '\n') {
                        dirent->d_name[i] = '?';
                    }
                }

                write(client_sd, dirent->d_name, d_name_len);
                write(client_sd, "\r\n", 2);
            }
            write(client_sd, "\0", 1);
            puts("[dir closed]");
            closedir(dir);
        } else {
            printf("dir unreachable\n");
            write(client_sd, "\t<error>\n", sizeof("\t<error>\n"));
        }
        puts("[Search finish]");
        /*{
            write(client_sd, buf, sizeof(buf));
            dir = opendir(buf);
            if (NULL != dir) {
                printf("dir opened");
                while (NULL != (dirent = readdir(dir))) {
                    write(client_sd, "\t", 1);
                    write(client_sd, dirent->d_name, strlen(dirent->d_name));
                    write(client_sd, "\n", 1);
                }
                write(client_sd, "\0", 1);
                closedir(dir);
            } else {
                printf("dir unreachable");
                write(client_sd, "\t<error>\n", sizeof("\t<error>\n"));
            }
            printf("\n");
        }*/

        free(buf);
    }

    clean:
    puts("[Clean started]");
    write(client_sd, "server error", sizeof("server error"));
    free(buf);
}


static void* prepare_process(void *argv) {
    int client;
    if (argv) { }
    while (1) {
        /*increase counter*/
        pthread_mutex_lock(&free_threads_mutex);      /*x - free_threads*/
        ++free_threads;
        pthread_mutex_unlock(&free_threads_mutex);    /*o - free_threads*/

        /*when client come - retrieve from queue*/
        printf("\rFree = %i ", free_threads);
        puts("[wait client]");
        pthread_cond_wait(&client_came, &queue_mutex);      /*x - queue - атомарная проверка и блокировка*/
        client = queue_dequeue(clients);
        pthread_mutex_unlock(&queue_mutex);                 /*o - queue*/

        if (-1 == client) {
            puts("oops..");
            pthread_mutex_lock(&pool_size_mutex);           /*x - pool_size*/
            if (pool_size > N) {                            /*уничтожаем - если больше N*/
                --pool_size;
                pthread_mutex_unlock(&pool_size_mutex);     /*o - pool_size*/
                break;
            }
            pthread_mutex_unlock(&pool_size_mutex);         /*o - pool_size*/
        }

        /*decrease counter*/
        pthread_mutex_lock(&free_threads_mutex);      /*x - free_threads*/
        --free_threads;
        pthread_mutex_unlock(&free_threads_mutex);    /*o - free_threads*/

        puts("[start processing client]");
        process_dir(client);
        puts("[finish processing client]");
        close(client);
    }
    pthread_mutex_lock(&free_threads_mutex);          /*x - free_threads*/
    --free_threads;
    pthread_mutex_unlock(&free_threads_mutex);        /*o - free_threads*/
    pthread_exit(NULL);
}


static void start_processing_client() {
    pthread_mutex_lock(&free_threads_mutex);          /*x - free_count*/
    if (0 == free_threads) {
        pthread_t th;

        pthread_mutex_unlock(&free_threads_mutex);    /*o - free_count*/

        pthread_mutex_lock(&pool_size_mutex);               /*x - pool_size*/
        if (pool_size < K) {
            ++pool_size;
            pthread_mutex_unlock(&pool_size_mutex);         /*o - pool_size*/
            if (pthread_create(&th, NULL, prepare_process, NULL)) {
                perror("pthread_create");
                stop_server();
            }

            if (pthread_detach(th)) {
                perror("pthread_detach");
                stop_server();
            }
        } else {
            pthread_mutex_unlock(&pool_size_mutex);         /*o - pool_size*/
        }
    } else {
        pthread_mutex_unlock(&free_threads_mutex);    /*o - free_count*/
    }
    pthread_cond_signal(&client_came);
}


int main(int argc, char *argv[]) {
    struct sockaddr_in addr;                /*in.h*/
    int opt = 1;                            /*для setsockopt*/

    /* setup SIGINT handler*/
    signal(SIGINT, sighandler);

    puts("[queue init]");
    if (NULL == (clients = queue_init())) {
        perror("malloc");
        stop_server();
    }

    if ((0 != pthread_mutex_init(&queue_mutex, NULL))             /*pthread.h - before using mutex*/
        || (0 != pthread_cond_init(&client_came, NULL))           /*pthread.h - no race*/
        || (0 != pthread_mutex_init(&pool_size_mutex, NULL))) {
        perror("pthread_mutex_init");
        stop_server();
    }

    {
        size_t i;
        for (i = 0; i < N; ++i) {
            pthread_t th;                                   /*pthread.h*/
            if (pthread_create(&th, NULL, prepare_process, NULL)     /*pthread.h - start routine "prepare_process"*/
                || pthread_detach(th)) {
                perror("pthread");
                stop_server();
            }
        }
        pool_size = N;
    }

if ((-1 == (sd = socket(AF_INET, SOCK_STREAM, 0)))                                  /*socket.h - create tcp socket-descriptor(sd)*/
        || (-1 == setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)))) {  /*socket.h - change options of socket to universal and reusable(with bind) */
        perror("socket");
        stop_server();
    }

    /* prepare to bind */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if ((-1 == bind(sd, (struct sockaddr *) &addr, sizeof(addr)))
            || (-1 == listen(sd, 50))) {
        perror("bind");
        stop_server();
    }

    while (1) {
        int client_sd;
        struct sockaddr_in client_addr;
        size_t client_addr_len = sizeof(client_addr);
        if (-1 == (client_sd = accept(sd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len))) {
            perror("accept");
            stop_server();
        }
        pthread_mutex_lock(&queue_mutex);
        queue_enqueue(clients, client_sd);
        pthread_mutex_unlock(&queue_mutex);

        start_processing_client();
    }
}
