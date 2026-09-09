#include <stdio.h>
#include "process.h"
#include "scheduler.h"

static int read_positive_value(const char *prompt) {
    int value;

    do {
        printf("%s", prompt);
        while (scanf("%d", &value) != 1) {
            printf("Ingrese un numero entero: ");
            while (getchar() != '\n') {
            }
        }

        if (value <= 0) {
            printf("El valor debe ser mayor que cero.\n");
        }
    } while (value <= 0);

    return value;
}

static int read_non_negative_value(const char *prompt) {
    int value;

    do {
        printf("%s", prompt);
        while (scanf("%d", &value) != 1) {
            printf("Ingrese un numero entero: ");
            while (getchar() != '\n') {
            }
        }

        if (value < 0) {
            printf("El valor no puede ser negativo.\n");
        }
    } while (value < 0);

    return value;
}

int main(void) {
    int process_count = read_positive_value(
        "Cantidad de procesos: "
    );
    int boost_interval = read_positive_value(
        "Intervalo de aging (ciclos): "
    );
    Process processes[process_count];

    for (int i = 0; i < process_count; i++) {
        char pid[12];
        int arrival_time;
        int burst_time;

        snprintf(pid, sizeof(pid), "P%d", i + 1);
        printf("\nProceso %s\n", pid);
        arrival_time = read_non_negative_value("  Arrival time: ");
        burst_time = read_positive_value("  Burst time: ");
        initialize_process(
            &processes[i],
            pid,
            arrival_time,
            burst_time
        );
    }

    Scheduler scheduler;

    initialize_scheduler(&scheduler, boost_interval);

    run_scheduler(&scheduler, processes, process_count);

    return 0;
}