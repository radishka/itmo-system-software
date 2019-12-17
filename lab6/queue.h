#ifndef _QUEUE_H_
#define _QUEUE_H_

struct queue;

struct queue * queue_init(void);
void queue_enqueue(struct queue *q, int e);
int queue_dequeue(struct queue *q);
void queue_destroy(struct queue *q);

#endif /* _QUEUE_H_ */
