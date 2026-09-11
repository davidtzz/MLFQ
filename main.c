#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include "process.h"
#include "scheduler.h"

/*
 * Pusimos límites para que el ejemplo sea un poco más manejable, 50 procesos
 * caben en el arreglo fijo sin depender de una reserva enorme en la pila,
 * y también limites de tiempo para que el programa no se quede mucho tiempo allí corriendo
 * y que no generen tablas de salida excesivamente grandes. La carga total se limita
 */
#define MAX_PROCESSES 50
#define MAX_TIME 10000
#define MAX_TOTAL_BURST 10000

static int read_value(
    const char *prompt,
    int minimum,
    int maximum,
    const char *range_message
) {
    char input[128];

    while (1) {
        /*
         * Se lee una linea completa para poder detectar overflow, texto
         * mezclado con numeros y valores fuera de rango antes de convertir.
         */
        printf("%s", prompt);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("No se pudo leer la entrada.\n");
            continue;
        }

        char *start = input;
        while (isspace((unsigned char)*start)) {
            start++;
        }

        errno = 0;
        char *end;
        long long parsed_value = strtoll(start, &end, 10);

        while (isspace((unsigned char)*end)) {
            end++;
        }

        if (start == end || *end != '\0' ||
            errno == ERANGE ||
            parsed_value < minimum ||
            parsed_value > maximum) {
            printf("%s\n", range_message);
            continue;
        }

        return (int)parsed_value;
    }
}

static int read_process_count(void) {
    return read_value(
        "Cantidad de procesos: ",
        1,
        MAX_PROCESSES,
        "Ingrese un numero entero entre 1 y 50."
    );
}

static int read_boost_interval(void) {
    return read_value(
        "Intervalo de aging (ciclos, 0 = sin boost): ",
        0,
        MAX_TIME,
        "Ingrese un numero entero entre 0 y 10000."
    );
}

static int read_arrival_time(void) {
    return read_value(
        "  Arrival time: ",
        0,
        MAX_TIME,
        "El arrival debe estar entre 0 y 10000."
    );
}

static int read_burst_time(int remaining_burst_limit) {
    if (remaining_burst_limit <= 0) {
        printf("Se alcanzo la carga total maxima de 10000 ciclos.\n");
        return 0;
    }

    int maximum = remaining_burst_limit < MAX_TIME
        ? remaining_burst_limit
        : MAX_TIME;

    return read_value(
        "  Burst time: ",
        1,
        maximum,
        "El burst debe ser positivo y la carga total no puede superar 10000."
    );
}

int main(void) {
    int process_count = read_process_count();
    int boost_interval = read_boost_interval();
    Process processes[MAX_PROCESSES];
    int total_burst = 0;

    for (int i = 0; i < process_count; i++) {
        char pid[12];
        int arrival_time;
        int burst_time;

        snprintf(pid, sizeof(pid), "P%d", i + 1);
        printf("\nProceso %s\n", pid);
        arrival_time = read_arrival_time();
        burst_time = read_burst_time(MAX_TOTAL_BURST - total_burst);
        if (burst_time == 0) {
            return 1;
        }
        total_burst += burst_time;
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