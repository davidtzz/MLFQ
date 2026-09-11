#include "scheduler.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int cycle;
    int queue_index;
    Process *process;
} ScheduleEntry;

static void write_table_border(FILE *file, int column_count) {
    fprintf(file, "+----");

    for (int i = 0; i < column_count; i++) {
        fprintf(file, "+----");
    }

    fprintf(file, "+\n");
}

static void add_schedule_entry(
    ScheduleEntry **schedule,
    int *schedule_count,
    int *schedule_capacity,
    int cycle,
    int queue_index,
    Process *process
) {
    if (*schedule_count == *schedule_capacity) {
        int new_capacity;

        if (*schedule_capacity == 0) {
            new_capacity = 16;
        } else {
            if (*schedule_capacity > INT_MAX / 2) {
                free(*schedule);
                *schedule = NULL;
                *schedule_count = -1;
                return;
            }

            new_capacity = *schedule_capacity * 2;
        }

        if (new_capacity > INT_MAX / (int)sizeof(ScheduleEntry)) {
            free(*schedule);
            *schedule = NULL;
            *schedule_count = -1;
            return;
        }

        ScheduleEntry *new_schedule = realloc(
            *schedule,
            new_capacity * sizeof(ScheduleEntry)
        );

        if (new_schedule == NULL) {
            free(*schedule);
            *schedule = NULL;
            *schedule_count = -1;
            return;
        }

        *schedule = new_schedule;
        *schedule_capacity = new_capacity;
    }

    (*schedule)[*schedule_count].cycle = cycle;
    (*schedule)[*schedule_count].queue_index = queue_index;
    (*schedule)[*schedule_count].process = process;
    (*schedule_count)++;
}

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
    Scheduler *scheduler
) {
    /*
     * El boost solo mueve procesos que esperan en Q1/Q2. El proceso activo
     * se agrega despues, para conservar el orden FIFO de los que esperaban.
     * Asi el boost no transforma una interrupcion en una perdida de quantum.
     */
    for (int i = 1; i < NUM_QUEUES; i++) {
        while (!is_queue_empty(&scheduler->queues[i])) {
            Process *process = dequeue(&scheduler->queues[i]);

            if (process->remaining_time > 0) {
                process->current_queue = 0;
                enqueue(&scheduler->queues[0], process);
            }
        }
    }

}

void initialize_scheduler(Scheduler *scheduler, int boost_interval) {
    /*
     * Un quantum pequeno en Q0 mejora la respuesta de procesos nuevos, pero
     * aumenta los cambios de contexto y hace que los procesos largos bajen
     * antes. Por eso los quantums crecen en las colas inferiores.
     *
     * Si el boost es muy frecuente, los procesos casi no permanecen en Q1/Q2
     * y el comportamiento se acerca a una ronda de alta prioridad. Si no hay
     * boost (intervalo 0), un flujo continuo de procesos nuevos puede causar
     * starvation en las colas inferiores; los boosts periodicos son la
     * proteccion contra ese caso.
     */
    scheduler->quantums[0] = 2;
    scheduler->quantums[1] = 4;
    scheduler->quantums[2] = 8;

    scheduler->current_time = 0;
    scheduler->boost_interval = boost_interval;

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
    ScheduleEntry *schedule = NULL;
    int schedule_count = 0;
    int schedule_capacity = 0;

    while (finished_processes < process_count) {
        
        add_arrived_processes(
            scheduler,
            processes,
            process_count
        );

        if (scheduler->boost_interval > 0 &&
            scheduler->current_time > 0 &&
            scheduler->current_time % scheduler->boost_interval == 0) {

            priority_boost(
                scheduler
            );
        }

        int queue_index = get_next_queue(scheduler);

        if (queue_index == -1) {
            add_schedule_entry(
                &schedule,
                &schedule_count,
                &schedule_capacity,
                scheduler->current_time,
                -1,
                NULL
            );
            if (schedule_count == -1) {
                printf("No se pudo guardar la tabla de ejecucion.\n");
                return;
            }
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
        int boosted = 0;

        while (cycles < quantum &&
               process->remaining_time > 0) {

            add_schedule_entry(
                &schedule,
                &schedule_count,
                &schedule_capacity,
                scheduler->current_time,
                queue_index,
                process
            );
            if (schedule_count == -1) {
                printf("No se pudo guardar la tabla de ejecucion.\n");
                return;
            }
            process->remaining_time--;
            scheduler->current_time++;
            cycles++;

            add_arrived_processes(
                scheduler,
                processes,
                process_count
            );

            if (scheduler->boost_interval > 0 &&
                scheduler->current_time > 0 &&
                scheduler->current_time % scheduler->boost_interval == 0) {
                priority_boost(
                    scheduler
                );

                if (process->remaining_time > 0) {
                    process->current_queue = 0;
                    enqueue(&scheduler->queues[0], process);
                }

                boosted = 1;
                break;
            }
        }

        if (process->remaining_time == 0) {
            process->finish_time = scheduler->current_time;
            finished_processes++;
        } else if (!boosted) {
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

    FILE *table_file = fopen("schedule.txt", "w");

    if (table_file == NULL) {
        printf("Resultados guardados en results.csv, pero no se pudo crear schedule.txt\n");
        free(schedule);
        return;
    }

    fprintf(table_file, "Tabla de distribucion de procesos\n\n");
    write_table_border(table_file, schedule_count);
    fprintf(table_file, "| %-2s", "Q/T");

    for (int cycle = 0; cycle < schedule_count; cycle++) {
        fprintf(table_file, "|%-4d", cycle);
    }

    fprintf(table_file, "|\n");
    write_table_border(table_file, schedule_count);

    for (int queue_index = 0; queue_index < NUM_QUEUES; queue_index++) {
        fprintf(table_file, "| Q%d ", queue_index);

        for (int cycle = 0; cycle < schedule_count; cycle++) {
            const char *process_id = "-";

            for (int i = 0; i < schedule_count; i++) {
                if (schedule[i].cycle == cycle &&
                    schedule[i].queue_index == queue_index) {
                    process_id = schedule[i].process->pid;
                    break;
                }
            }

            fprintf(table_file, "|%-4s", process_id);
        }

        fprintf(table_file, "|\n");
        write_table_border(table_file, schedule_count);
    }

    fclose(table_file);
    free(schedule);

    printf("Resultados guardados en results.csv y schedule.txt\n");
    }