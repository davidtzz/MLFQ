# Implementación de scheduler con políticas MLFQ

## Resumen del proyecto

Este es un simulador de planificación de procesos con el algoritmo MLFQ (Multi-Level Feedback Queue), con 3 colas de prioridad, quantums crecientes, aging por priority boost, y salida en `` `results.csv` ``(donde se puede encontrar las métricas calculadas en el ejercicio)  `` ` schedule.txt` `` -donde se construye el diagrama de Gantt, para comprender mejor, el desplazamiento de los procesos-. Está organizado en 4 módulos: main.c (entrada/orquestación), process.h/c (entidad Proceso + métricas), queue.h/c (cola FIFO enlazada) y scheduler.h/c (algoritmo MLFQ + generación de reportes).

## Decisiones de diseño y justificación

### Separación de módulos por responsabilidades
Este código divide la implemetación del scheduler por responsibilidades utilizando 3 conceptos claros: Process, Queue y Scheduler

 - Process: En el archivo .h se define la entidad proceso que utiliza el scheduler con todos campos requeridos en el enunciado del laboratorio.
 - Queue: En estos archivos tenemos el .h en que se define la entidad Cola y que nos permite crear las colas de los procesos y el .c que utiliza la estructura y crea las colas durante la simulación.
 - Scheduler: 