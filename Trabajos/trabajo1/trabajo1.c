#define _DEFAULT_SOURCE    // expone usleep/useconds_t con -std=c11 -pedantic
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifndef N_PHILOS
#define N_PHILOS 5
#endif

#ifndef ROUNDS
#define ROUNDS 1
#endif

// Tiempos fijos (microsegundos) para TODOS los hilos
#ifndef THINK_US
#define THINK_US 700000
#endif

#ifndef EAT_US
#define EAT_US 700000
#endif

typedef enum { THINKING, HUNGRY, EATING } state_t;

static state_t state[N_PHILOS];
static sem_t mutex;          // protege state[]
static sem_t s[N_PHILOS];    // semáforo privado por filósofo

static pthread_t th[N_PHILOS];
static int ids[N_PHILOS];

// Para medir tiempo relativo (ms desde inicio)
static struct timespec t0;

static inline int LEFT(int i)  { return (i + N_PHILOS - 1) % N_PHILOS; }
static inline int RIGHT(int i) { return (i + 1) % N_PHILOS; }

static long now_ms_since_start(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    long sec  = (long)(t.tv_sec - t0.tv_sec);
    long nsec = (long)(t.tv_nsec - t0.tv_nsec);
    return sec * 1000L + nsec / 1000000L;
}

// Imprime en formato uniforme para que se vea "pro"
static void log_event(int i, int round, const char *msg) {
    // Filósofo se imprime 1..N
    printf("[%6ld ms] F[%d] R[%d/%d] %s\n",
           now_ms_since_start(), i + 1, round, ROUNDS, msg);
    fflush(stdout);
}

// Si i está hambriento y sus vecinos no están comiendo, habilítalo a comer
static void test(int i) {
    if (state[i] == HUNGRY &&
        state[LEFT(i)] != EATING &&
        state[RIGHT(i)] != EATING) {

        state[i] = EATING;
        // Despierta SOLO a este filósofo
        sem_post(&s[i]);
    }
}

static void take_forks(int i, int round) {
    sem_wait(&mutex);

    state[i] = HUNGRY;
    log_event(i, round, "hambriento...");

    test(i);

    sem_post(&mutex);

    // Si no quedó habilitado, se bloquea hasta que alguien lo despierte
    sem_wait(&s[i]);

    // Ya quedó en EATING
    char buf[128];
    snprintf(buf, sizeof(buf), "usa cubiertos (%d, %d) y come...",
             i + 1, RIGHT(i) + 1);
    log_event(i, round, buf);
}

static void put_forks(int i, int round) {
    sem_wait(&mutex);

    state[i] = THINKING;

    char buf[128];
    snprintf(buf, sizeof(buf), "regresa los cubiertos (%d, %d)",
             i + 1, RIGHT(i) + 1);
    log_event(i, round, buf);

    // Al soltar, intenta habilitar a los vecinos
    test(LEFT(i));
    test(RIGHT(i));

    sem_post(&mutex);
}

static void *philosopher(void *arg) {
    int i = *(int *)arg;

    for (int r = 1; r <= ROUNDS; r++) {
        log_event(i, r, "piensa...");
        usleep((useconds_t)THINK_US);

        take_forks(i, r);
        usleep((useconds_t)EAT_US);
        put_forks(i, r);
    }

    // Mensaje final por hilo
    printf("[%6ld ms] F[%d] FIN: completo %d rondas.\n",
           now_ms_since_start(), i + 1, ROUNDS);
    fflush(stdout);

    return NULL;
}

int main(void) {
    // Inicio del reloj
    clock_gettime(CLOCK_MONOTONIC, &t0);

    // Mensaje inicial bonito
    printf("=== Filosofos comensales | N=%d | RONDAS=%d | THINK_US=%d | EAT_US=%d ===\n\n",
           N_PHILOS, ROUNDS, THINK_US, EAT_US);
    fflush(stdout);

    // Inicializa mutex y semáforos privados
    if (sem_init(&mutex, 0, 1) != 0) { perror("sem_init(mutex)"); return 1; }

    for (int i = 0; i < N_PHILOS; i++) {
        state[i] = THINKING;
        if (sem_init(&s[i], 0, 0) != 0) { perror("sem_init(s[i])"); return 1; }
    }

    // Crea hilos con IDs estables
    for (int i = 0; i < N_PHILOS; i++) {
        ids[i] = i;
        if (pthread_create(&th[i], NULL, philosopher, &ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < N_PHILOS; i++) pthread_join(th[i], NULL);

    for (int i = 0; i < N_PHILOS; i++) sem_destroy(&s[i]);
    sem_destroy(&mutex);

    printf("\n=== Listo: todos los filosofos completaron %d rondas. ===\n", ROUNDS);
    return 0;
}
