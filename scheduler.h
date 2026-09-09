#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "queue.h"

#define NUM_QUEUES 3

typedef struct {
    Queue queues[NUM_QUEUES];
    int quantums[NUM_QUEUES];
    int current_time;
    int boost_interval;
} Scheduler;

void initialize_scheduler(Scheduler *scheduler, int boost_interval);

void run_scheduler(
    Scheduler *scheduler,
    Process processes[],
    int process_count
);

#endif