#ifndef QUEUE_H
#define QUEUE_H

#include "process.h"

typedef struct QueueNode {
    Process *process;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
} Queue;

void initialize_queue(Queue *queue);
int is_queue_empty(const Queue *queue);

void enqueue(Queue *queue, Process *process);
Process *dequeue(Queue *queue);

#endif