#ifndef PROCESS_H
#define PROCESS_H

typedef struct {
    char pid[10];

    int arrival_time;
    int burst_time;
    int remaining_time;

    int start_time;
    int finish_time;
    int first_response_time;

    int current_queue;
} Process;

void initialize_process(
    Process *process,
    const char *pid,
    int arrival_time,
    int burst_time
);

int get_response_time(const Process *process);
int get_turnaround_time(const Process *process);
int get_waiting_time(const Process *process);

#endif