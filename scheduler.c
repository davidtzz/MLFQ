#include "scheduler.h"
#include <stdio.h>

static int process_is_queued(
    Scheduler *scheduler,
    Process *process
) {
    for (int i = 0; i < NUM_QUEUES; i++) {
        QueueNode *current = scheduler->queues[i].front;

        while (current != NULL) {
            if (current->process == process) {
                return 1;
            }

            current = current->next;
        }
    }

    return 0;
}
static void add_arrived_processes(
    Scheduler *scheduler,
    Process processes[],
    int process_count
) {
    for (int i = 0; i < process_count; i++) {
        if (processes[i].arrival_time == scheduler->current_time &&
            processes[i].remaining_time > 0 &&
            !process_is_queued(scheduler, &processes[i])) {

            processes[i].current_queue = 0;
            enqueue(&scheduler->queues[0], &processes[i]);
        }
    }
}

static int get_next_queue(Scheduler *scheduler) {
    for (int i = 0; i < NUM_QUEUES; i++) {
        if (!is_queue_empty(&scheduler->queues[i])) {
            return i;
        }
    }

    return -1;
}

static void priority_boost(
    Scheduler *scheduler,
    Process processes[],
    int process_count
) {
    for (int i = 0; i < NUM_QUEUES; i++) {
        while (!is_queue_empty(&scheduler->queues[i])) {
            Process *process = dequeue(&scheduler->queues[i]);

            if (process->remaining_time > 0) {
                process->current_queue = 0;
                enqueue(&scheduler->queues[0], process);
            }
        }
    }

    for (int i = 0; i < process_count; i++) {
        if (processes[i].remaining_time > 0 &&
            processes[i].arrival_time <= scheduler->current_time) {
            processes[i].current_queue = 0;
        }
    }
}

void initialize_scheduler(Scheduler *scheduler) {
    scheduler->quantums[0] = 2;
    scheduler->quantums[1] = 4;
    scheduler->quantums[2] = 8;

    scheduler->current_time = 0;

    for (int i = 0; i < NUM_QUEUES; i++) {
        initialize_queue(&scheduler->queues[i]);
    }
}

void run_scheduler(
    Scheduler *scheduler,
    Process processes[],
    int process_count
) {
    int finished_processes = 0;

    while (finished_processes < process_count) {
        
        add_arrived_processes(
            scheduler,
            processes,
            process_count
        );

        if (scheduler->current_time > 0 &&
            scheduler->current_time % BOOST_INTERVAL == 0) {

            priority_boost(
                scheduler,
                processes,
                process_count
            );
        }

        int queue_index = get_next_queue(scheduler);

        if (queue_index == -1) {
            scheduler->current_time++;
            continue;
        }

        Process *process = dequeue(
            &scheduler->queues[queue_index]
        );

        if (process->start_time == -1) {
            process->start_time = scheduler->current_time;
            process->first_response_time = scheduler->current_time;
        }

        int quantum = scheduler->quantums[queue_index];
        int cycles = 0;

        while (cycles < quantum &&
               process->remaining_time > 0) {

            process->remaining_time--;
            scheduler->current_time++;
            cycles++;

            add_arrived_processes(
                scheduler,
                processes,
                process_count
            );
        }

        if (process->remaining_time == 0) {
            process->finish_time = scheduler->current_time;
            finished_processes++;
        } else {
            if (cycles == quantum &&
                queue_index < NUM_QUEUES - 1) {

                process->current_queue = queue_index + 1;
            }

            enqueue(
                &scheduler->queues[process->current_queue],
                process
            );
        }
    }

    FILE *file = fopen("results.csv", "w");

    if (file == NULL) {
    printf("No se pudo crear results.csv\n");
    return;
    }

    fprintf(
    file,
    "PID,Arrival,Burst,Start,Finish,Response,Turnaround,Waiting\n"
    );

    for (int i = 0; i < process_count; i++) {
        fprintf(
        file,
        "%s,%d,%d,%d,%d,%d,%d,%d\n",
        processes[i].pid,
        processes[i].arrival_time,
        processes[i].burst_time,
        processes[i].start_time,
        processes[i].finish_time,
        get_response_time(&processes[i]),
        get_turnaround_time(&processes[i]),
        get_waiting_time(&processes[i])
    );
    }

    fclose(file);

    printf("Resultados guardados en results.csv\n");
    }