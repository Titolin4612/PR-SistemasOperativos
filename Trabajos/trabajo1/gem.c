#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

// Configuración general
#define NUM_FILOSOFOS 5
#define PENSANDO 0
#define HAMBRIENTO 1
#define COMIENDO 2

// Identificadores de vecinos
#define IZQUIERDA (num_filosofo + NUM_FILOSOFOS - 1) % NUM_FILOSOFOS
#define DERECHA (num_filosofo + 1) % NUM_FILOSOFOS

// Variables globales
int estado[NUM_FILOSOFOS];
int id_filosofos[NUM_FILOSOFOS];

sem_t mutex;               // Semáforo para exclusión mutua al cambiar estados
sem_t s[NUM_FILOSOFOS];    // Un semáforo por filósofo para bloquearse si no puede comer

// Función para verificar si un filósofo puede empezar a comer
void verificar(int num_filosofo) {
    if (estado[num_filosofo] == HAMBRIENTO && 
        estado[IZQUIERDA] != COMIENDO && 
        estado[DERECHA] != COMIENDO) {
        
        estado[num_filosofo] = COMIENDO;
        sleep(2); // Tiempo que tarda en comer
        
        printf("Filosofo [%d] usa cubiertos (%d, %d)\n", num_filosofo + 1, IZQUIERDA + 1, num_filosofo + 1);
        printf("Filosofo [%d] come...\n", num_filosofo + 1);
        
        // sem_post incrementa el valor, indicando que el filósofo tiene los tenedores
        sem_post(&s[num_filosofo]);
    }
}

// Acción de tomar los cubiertos
void tomar_cubiertos(int num_filosofo) {
    sem_wait(&mutex); // Entra a sección crítica
    
    estado[num_filosofo] = HAMBRIENTO;
    printf("Filosofo [%d] hambriento...\n", num_filosofo + 1);
    
    verificar(num_filosofo); // Intenta comer
    
    sem_post(&mutex); // Sale de sección crítica
    
    // Si no pudo comer, se queda esperando en su semáforo personal
    sem_wait(&s[num_filosofo]);
    sleep(1);
}

// Acción de dejar los cubiertos
void dejar_cubiertos(int num_filosofo) {
    sem_wait(&mutex); // Entra a sección crítica
    
    estado[num_filosofo] = PENSANDO;
    printf("Filosofo [%d] regresa los cubiertos (%d, %d)\n", num_filosofo + 1, IZQUIERDA + 1, num_filosofo + 1);
    printf("Filosofo [%d] piensa...\n", num_filosofo + 1);
    
    // Al soltar cubiertos, avisa a sus vecinos por si ellos estaban esperando
    verificar(IZQUIERDA);
    verificar(DERECHA);
    
    sem_post(&mutex); // Sale de sección crítica
}

// Rutina que ejecuta cada hilo (filósofo)
void* rutina_filosofo(void* num) {
    int i = *(int*)num;
    while (1) {
        // Ciclo de vida del filósofo
        tomar_cubiertos(i);
        dejar_cubiertos(i);
    }
}

int main() {
    pthread_t hilos[NUM_FILOSOFOS];

    // Inicializar semáforos
    sem_init(&mutex, 0, 1); // Mutex binario para proteger el arreglo de estados
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        sem_init(&s[i], 0, 0); // Semáforos personales inician en 0 (bloqueados)
        estado[i] = PENSANDO;
        id_filosofos[i] = i;
    }

    // Crear los hilos (Filósofos)
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_create(&hilos[i], NULL, rutina_filosofo, &id_filosofos[i]);
        printf("Filosofo [%d] piensa...\n", i + 1);
    }

    // Esperar a que los hilos terminen (en este caso es infinito)
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Destruir semáforos al finalizar
    sem_destroy(&mutex);
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        sem_destroy(&s[i]);
    }

    return 0;
}