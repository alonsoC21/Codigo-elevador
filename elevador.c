#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_ELEVADORES 3
#define NUM_PISOS 10

typedef struct {
    int piso_actual;
    int objetivo;
    int id;
    int en_movimiento;
} Elevador;

Elevador elevadores[NUM_ELEVADORES];

// Semáforo para sincronizar el acceso a las solicitudes de los elevadores
sem_t semaforo_elevador;

// Función para mover el elevador al piso objetivo
void *mover_elevador(void *arg) {
    Elevador *elevador = (Elevador *)arg;
    
    while (1) {
        if (elevador->en_movimiento) {
            printf("Elevador %d moviéndose del piso %d al piso %d\n", elevador->id, elevador->piso_actual, elevador->objetivo);
            
            // Simular movimiento
            while (elevador->piso_actual != elevador->objetivo) {
                if (elevador->piso_actual < elevador->objetivo) {
                    elevador->piso_actual++;
                } else if (elevador->piso_actual > elevador->objetivo) {
                    elevador->piso_actual--;
                }
                
                printf("Elevador %d en piso %d\n", elevador->id, elevador->piso_actual);
                sleep(1);  // Simular el tiempo que tarda en subir o bajar
            }

            printf("Elevador %d llegó al piso %d\n", elevador->id, elevador->piso_actual);
            elevador->en_movimiento = 0;
        }

        sleep(1);  // Esperar un momento antes de verificar de nuevo
    }
    return NULL;
}

// Función para asignar un elevador a una solicitud
void solicitar_elevador(int piso_origen, int piso_destino) {
    int mejor_elevador = -1;
    int distancia_minima = NUM_PISOS + 1;

    sem_wait(&semaforo_elevador);  // Bloquear el acceso a los elevadores

    // Buscar el elevador más cercano que no esté en movimiento
    for (int i = 0; i < NUM_ELEVADORES; i++) {
        if (!elevadores[i].en_movimiento) {
            int distancia = abs(elevadores[i].piso_actual - piso_origen);
            if (distancia < distancia_minima) {
                mejor_elevador = i;
                distancia_minima = distancia;
            }
        }
    }

    if (mejor_elevador != -1) {
        printf("Elevador %d asignado a solicitud del piso %d al piso %d\n", mejor_elevador, piso_origen, piso_destino);
        elevadores[mejor_elevador].objetivo = piso_origen;
        elevadores[mejor_elevador].en_movimiento = 1;
        sleep(1);  // Simular el tiempo que tarda en llegar al origen
        
        // Una vez que llega al origen, se dirige al destino
        elevadores[mejor_elevador].objetivo = piso_destino;
        elevadores[mejor_elevador].en_movimiento = 1;
    } else {
        printf("No hay elevadores disponibles en este momento.\n");
    }

    sem_post(&semaforo_elevador);  // Liberar el acceso a los elevadores
}

// Función para verificar si hay algún elevador en movimiento
int elevadores_en_movimiento() {
    for (int i = 0; i < NUM_ELEVADORES; i++) {
        if (elevadores[i].en_movimiento) {
            return 1;  // Si algún elevador está en movimiento, retornar 1
        }
    }
    return 0;  // Ningún elevador en movimiento
}

int main() {
    pthread_t hilos_elevadores[NUM_ELEVADORES];
    int piso_origen, piso_destino;

    // Inicializar los elevadores
    for (int i = 0; i < NUM_ELEVADORES; i++) {
        elevadores[i].piso_actual = 0;
        elevadores[i].objetivo = 0;
        elevadores[i].id = i;
        elevadores[i].en_movimiento = 0;
    }

    // Inicializar el semáforo
    sem_init(&semaforo_elevador, 0, 1);

    // Crear los hilos de los elevadores
    for (int i = 0; i < NUM_ELEVADORES; i++) {
        pthread_create(&hilos_elevadores[i], NULL, mover_elevador, (void *)&elevadores[i]);
    }

    // Simular solicitudes de usuarios
    while (1) {
        // Esperar hasta que no haya elevadores en movimiento
        while (elevadores_en_movimiento()) {
            sleep(1);  // Esperar un segundo antes de verificar de nuevo
        }

        // Solicitar elevador cuando no haya elevadores en movimiento
        printf("Solicitar elevador: \n");
        printf("Introduce el piso origen (0-%d): ", NUM_PISOS - 1);
        scanf("%d", &piso_origen);
        printf("Introduce el piso destino (0-%d): ", NUM_PISOS - 1);
        scanf("%d", &piso_destino);

        if (piso_origen >= 0 && piso_origen < NUM_PISOS && piso_destino >= 0 && piso_destino < NUM_PISOS) {
            solicitar_elevador(piso_origen, piso_destino);
        } else {
            printf("Pisos inválidos. Intenta de nuevo.\n");
        }
    }

    // Unir los hilos (esto no se ejecutará debido al bucle infinito)
    for (int i = 0; i < NUM_ELEVADORES; i++) {
        pthread_join(hilos_elevadores[i], NULL);
    }

    // Destruir el semáforo
    sem_destroy(&semaforo_elevador);

    return 0;
}
