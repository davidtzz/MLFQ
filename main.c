#include <stdio.h>
#include "process.h"
#include "scheduler.h"

int main() {

    Process processes[4];

    initialize_process(&processes[0], "P1", 0, 8);
    initialize_process(&processes[1], "P2", 1, 4);
    initialize_process(&processes[2], "P3", 2, 9);
    initialize_process(&processes[3], "P4", 3, 5);

    Scheduler scheduler;

    initialize_scheduler(&scheduler);

    run_scheduler(&scheduler, processes, 4);

    return 0;
}