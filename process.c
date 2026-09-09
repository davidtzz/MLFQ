#include "process.h"
#include <string.h>

void initialize_process(
    Process *process,
    const char *pid,
    int arrival_time,
    int burst_time
) {
    strcpy(process->pid, pid);

    process->arrival_time = arrival_time;
    process->burst_time = burst_time;
    process->remaining_time = burst_time;

    process->start_time = -1;
    process->finish_time = -1;
    process->first_response_time = -1;

    process->current_queue = 0;
}

int get_response_time(const Process *process) {
    return process->first_response_time - process->arrival_time;
}

int get_turnaround_time(const Process *process) {
    return process->finish_time - process->arrival_time;
}

int get_waiting_time(const Process *process) {
    return get_turnaround_time(process) - process->burst_time;
}