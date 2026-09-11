#include "queue.h"
#include <stdlib.h>

void initialize_queue(Queue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
}

int is_queue_empty(const Queue *queue) {
    return queue->front == NULL;
}

void enqueue(Queue *queue, Process *process) {
    QueueNode *new_node = malloc(sizeof(QueueNode));

    if (new_node == NULL) {
        return;
    }

    new_node->process = process;
    new_node->next = NULL;

    if (queue->rear == NULL) {
        queue->front = new_node;
        queue->rear = new_node;
    } else {
        queue->rear->next = new_node;
        queue->rear = new_node;
    }
}

Process *dequeue(Queue *queue) {
    if (is_queue_empty(queue)) {
        return NULL;
    }

    QueueNode *node = queue->front;
    Process *process = node->process;

    queue->front = node->next;

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    free(node);

    return process;
}