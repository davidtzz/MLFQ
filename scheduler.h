#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "queue.h"

#define NUM_QUEUES 3
#define BOOST_INTERVAL 20

typedef struct {
    Queue queues[NUM_QUEUES];
    int quantums[NUM_QUEUES];
    int current_time;
} Scheduler;

void initialize_scheduler(Scheduler *scheduler);

void run_scheduler(
    Scheduler *scheduler,
    Process processes[],
    int process_count
);

#endif