#include <stdlib.h>
#include <stdio.h>

#include "queue.h"
/*FIFO*/
struct queue {
    struct queue* head;
    struct queue* tail;
    int elem;
};


struct queue* queue_init() {
    struct queue* q;
    if (NULL == (q = (struct queue*) malloc(sizeof(struct queue)))) {
        return NULL;
    }
    q->head = NULL;
    q->tail = NULL;
    return q;
}

/*добавляет элемент в конец очереди*/
void queue_enqueue(struct queue *q, int e) {
    if (NULL == q) return;
    if (NULL == q->tail) {
        q->tail = q->head = queue_init();
        if (NULL == q->tail) return;
        q->tail->elem = e;
        return;
    }
    q->tail->tail = queue_init();
    q->tail->tail->elem = e;
    q->tail = q->tail->tail;
}

/*извлекает и возвращает первый элемент очереди*/
int queue_dequeue(struct queue *q) {
    int e;
    struct queue *new_head;
    if (NULL == q || NULL == q->head) {
        return -1;
    }
    e = q->head->elem;
    new_head = q->head->head;
    free(q->head);
    q->head = new_head;
    if (NULL == q->head)
        q->tail = NULL;
    return e;
}


void queue_destroy(struct queue *q) {
    if (q) {
        struct queue *next = q->head;
        while (next) {
            q->head = q->head->head;
            free(next);
            next = q->head;
        }
    }
    free(q);
}
