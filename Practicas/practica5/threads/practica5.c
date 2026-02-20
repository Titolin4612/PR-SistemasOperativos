// practica5.c
// Compilar: gcc -O2 -Wall -Wextra -pthread practica5.c -o practica5
// Ejecutar:  ./practica5 15 200000

#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct { int thread_no, n; useconds_t delay_us; } thread_args_t;

static void die(const char *msg) { perror(msg); exit(EXIT_FAILURE); }

static void sleep_us(useconds_t us) {
    struct timespec ts = { us / 1000000u, (long)(us % 1000000u) * 1000L };
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

static void print_header(const char *tag, int no) {
    printf("[%s] hilo=%d pid=%ld ppid=%ld pthread=%lu\n",
           tag, no, (long)getpid(), (long)getppid(), (unsigned long)pthread_self());
    fflush(stdout);
}

static void run_acumulada(int n, useconds_t d, int t) {
    long long acc = 0;
    for (int i = 0; i < n; i++, sleep_us(d))
        printf("[ACUM] hilo=%d pid=%ld i=%d acumulada=%lld\n", t, (long)getpid(), i, acc += i), fflush(stdout);
}

static void run_productoria(int n, useconds_t d, int t) {
    unsigned long long p = 1;
    for (int i = 1; i <= n; i++, sleep_us(d))
        printf("[PROD] hilo=%d pid=%ld i=%d productoria=%llu\n", t, (long)getpid(), i, p *= i), fflush(stdout);
}

static void run_potencias2(int n, useconds_t d, int t) {
    unsigned long long v = 1;
    for (int k = 0; k < n; k++, sleep_us(d))
        printf("[POW2] hilo=%d pid=%ld k=%d 2^k=%llu\n", t, (long)getpid(), k, k ? (v <<= 1) : v), fflush(stdout);
}

static void run_fibonacci(int n, useconds_t d, int t) {
    unsigned long long a = 0, b = 1;
    for (int i = 0; i < n; i++, sleep_us(d)) {
        printf("[FIB ] hilo=%d pid=%ld i=%d fib=%llu\n", t, (long)getpid(), i, a), fflush(stdout);
        b += a; a = b - a;
    }
}

static void* thread_main(void *arg) {
    thread_args_t *c = arg;
    print_header("PARENT-TH", c->thread_no);

    pid_t child = fork();
    if (child < 0) { fprintf(stderr, "[ERROR] hilo=%d fork() falló: %s\n", c->thread_no, strerror(errno)); return NULL; }

    if (child == 0) {
        print_header("CHILD-PROC", c->thread_no);
        void (*fns[])(int,useconds_t,int) = { run_acumulada, run_productoria, run_potencias2, run_fibonacci };
        fns[c->thread_no - 1](c->n, c->delay_us, c->thread_no);
        _exit(0);
    }

    int status;
    if (waitpid(child, &status, 0) < 0)
        fprintf(stderr, "[WARN] hilo=%d waitpid() falló: %s\n", c->thread_no, strerror(errno));
    else if (WIFEXITED(status))
        printf("[JOIN] hilo=%d hijo_pid=%ld exit=%d\n", c->thread_no, (long)child, WEXITSTATUS(status));
    else
        printf("[JOIN] hilo=%d hijo_pid=%ld terminó raro\n", c->thread_no, (long)child);
    fflush(stdout);
    return NULL;
}

int main(int argc, char **argv) {
    int n = argc >= 2 ? atoi(argv[1]) : 15;
    useconds_t delay_us = argc >= 3 ? (useconds_t)strtoul(argv[2], NULL, 10) : 200000;
    if (n <= 0) { fprintf(stderr, "Uso: %s [iteraciones>0] [delay_us]\n", argv[0]); return EXIT_FAILURE; }

    pthread_t th[4];
    thread_args_t cfg[4];
    for (int i = 0; i < 4; i++) {
        cfg[i] = (thread_args_t){ i + 1, n, delay_us };
        if (pthread_create(&th[i], NULL, thread_main, &cfg[i]) != 0) die("pthread_create");
    }
    for (int i = 0; i < 4; i++) pthread_join(th[i], NULL);
    printf("[MAIN] pid=%ld terminado.\n", (long)getpid());
    return 0;
}