#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void rutina_hijo(int num_hijo) {

    for (int i = 0; i <= 4; i++) {
        printf("hijo[%d] --> pid = %d y ppid %d i = %d\n",
               num_hijo, getpid(), getppid(), i);
        sleep(1);
    }
}

void rutina_padre() {

    for (int i = 0; i <= 4; i++) {
        printf("rutina del proceso padre valor de i:%d\n", i);
        sleep(1);
    }
}

int main() {

    pid_t pid;

    // ===== HIJO 1 =====
    pid = fork();
    if (pid == 0) {
        rutina_hijo(1);
        exit(0);
    }
    wait(NULL);

    // ===== HIJO 2 =====
    pid = fork();
    if (pid == 0) {
        rutina_hijo(2);
        exit(0);
    }
    wait(NULL);

    // ===== HIJO 3 =====
    pid = fork();
    if (pid == 0) {
        rutina_hijo(3);
        exit(0);
    }
    wait(NULL);

    // ===== PADRE =====
    rutina_padre();

    return 0;
}
