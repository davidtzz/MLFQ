# Implementación de scheduler con políticas MLFQ

## Resumen del proyecto

Este es un simulador de planificación de procesos con el algoritmo MLFQ (Multi-Level Feedback Queue), con 3 colas de prioridad, quantums crecientes, aging por priority boost, y salida en results.csv / schedule.txt. Está organizado en 4 módulos: main.c (entrada/orquestación), process.h/c (entidad Proceso + métricas), queue.h/c (cola FIFO enlazada) y scheduler.h/c (algoritmo MLFQ + generación de reportes).