/*
 * Calculadora RPN (Notación Polaca Inversa)
 * ----------------------------------------
 * Características:
 * - Pila de doubles (máx 1024 elementos)
 * - Entrada por líneas completas usando fgets
 * - Tokens separados por espacios o tabulaciones
 * - Soporta números decimales, negativos y notación científica
 * - Operadores binarios: + - * /
 * - Funciones: sqrt, sin, cos, tan (en GRADOS), pow
 * - Comandos:
 *      h -> ayuda
 *      c -> limpiar pila
 *      p -> mostrar tope
 *      s -> mostrar últimos 8 elementos
 *      q -> salir
 * - Manejo básico de errores
 * - Muestra resultado tras cada operación exitosa
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define STACK_MAX 1024
#define LINE_MAX  1024
#define SHOW_MAX  8

/* ============================
   Estructura de la pila
   ============================ */

typedef struct {
    double data[STACK_MAX];
    int top;
} Stack;

/* Inicializa la pila */
void stack_init(Stack *s) {
    s->top = 0;
}

/* Cantidad de elementos */
int stack_size(Stack *s) {
    return s->top;
}

/* Push */
int stack_push(Stack *s, double val) {
    if (s->top >= STACK_MAX) {
        printf("Error: pila llena\n");
        return 0;
    }
    s->data[s->top++] = val;
    return 1;
}

/* Pop */
int stack_pop(Stack *s, double *out) {
    if (s->top <= 0) {
        return 0;
    }
    *out = s->data[--s->top];
    return 1;
}

/* Peek (ver tope sin eliminar) */
int stack_peek(Stack *s, double *out) {
    if (s->top <= 0) return 0;
    *out = s->data[s->top - 1];
    return 1;
}

/* Limpiar pila */
void stack_clear(Stack *s) {
    s->top = 0;
}

/* ============================
   Utilidades
   ============================ */

/* Convierte grados a radianes */
double deg_to_rad(double deg) {
    return deg * (M_PI / 180.0);
}

/* Mostrar ayuda */
void mostrar_ayuda(void) {
    printf("\n--- Calculadora RPN ---\n");
    printf("Ingrese expresiones en notación polaca inversa.\n\n");

    printf("Operadores binarios:\n");
    printf("  +  -  *  /\n\n");

    printf("Funciones:\n");
    printf("  sqrt      -> raiz cuadrada\n");
    printf("  sin cos tan (grados)\n");
    printf("  pow       -> base expo -> pow\n\n");

    printf("Comandos:\n");
    printf("  h -> ayuda\n");
    printf("  c -> limpiar pila\n");
    printf("  p -> mostrar tope\n");
    printf("  s -> mostrar últimos 8 elementos\n");
    printf("  q -> salir\n\n");
}

/* Mostrar pila (máx 8 elementos desde el tope) */
void mostrar_pila(Stack *s) {
    printf("Pila:\n");

    int count = s->top < SHOW_MAX ? s->top : SHOW_MAX;

    for (int i = 0; i < count; i++) {
        int idx = s->top - 1 - i;
        printf("%d. %.6f\n", i + 1, s->data[idx]);
    }

    if (s->top == 0) {
        printf("(vacía)\n");
    }
}

/* Verifica si token es número */
int es_numero(const char *token, double *out) {
    char *endptr;
    double val = strtod(token, &endptr);

    if (endptr == token || *endptr != '\0') {
        return 0;
    }

    *out = val;
    return 1;
}

/* ============================
   Operaciones binarias
   ============================ */

int op_binaria(Stack *s, char op) {
    if (stack_size(s) < 2) {
        printf("Error: pila insuficiente\n");
        return 0;
    }

    double b, a;

    /* Obtener operandos */
    stack_pop(s, &b);
    stack_pop(s, &a);

    double res;

    switch (op) {
        case '+': res = a + b; break;
        case '-': res = a - b; break;
        case '*': res = a * b; break;

        case '/':
            if (b == 0.0) {
                printf("Error: división por cero\n");
                /* Restaurar operandos */
                stack_push(s, a);
                stack_push(s, b);
                return 0;
            }
            res = a / b;
            break;

        default:
            return 0;
    }

    stack_push(s, res);
    printf("= %g\n", res);
    return 1;
}

/* ============================
   Funciones unarias
   ============================ */

int op_sqrt(Stack *s) {
    if (stack_size(s) < 1) {
        printf("Error: pila insuficiente\n");
        return 0;
    }

    double a;
    stack_pop(s, &a);

    if (a < 0.0) {
        printf("Error: raíz de número negativo\n");
        stack_push(s, a);
        return 0;
    }

    double res = sqrt(a);
    stack_push(s, res);
    printf("= %g\n", res);
    return 1;
}

int op_trig(Stack *s, const char *func) {
    if (stack_size(s) < 1) {
        printf("Error: pila insuficiente\n");
        return 0;
    }

    double a;
    stack_pop(s, &a);

    double rad = deg_to_rad(a);
    double res = 0.0;

    if (strcmp(func, "sin") == 0) res = sin(rad);
    else if (strcmp(func, "cos") == 0) res = cos(rad);
    else if (strcmp(func, "tan") == 0) res = tan(rad);
    else return 0;

    stack_push(s, res);
    printf("= %g\n", res);
    return 1;
}

int op_pow(Stack *s) {
    if (stack_size(s) < 2) {
        printf("Error: pila insuficiente\n");
        return 0;
    }

    double expo, base;
    stack_pop(s, &expo);
    stack_pop(s, &base);

    double res = pow(base, expo);

    stack_push(s, res);
    printf("= %g\n", res);
    return 1;
}

/* ============================
   Procesamiento de tokens
   ============================ */

int procesar_token(Stack *s, const char *tok) {
    double num;

    /* Número */
    if (es_numero(tok, &num)) {
        stack_push(s, num);
        return 1;
    }

    /* Operadores */
    if (strlen(tok) == 1) {
        char c = tok[0];

        if (c == '+' || c == '-' || c == '*' || c == '/') {
            return op_binaria(s, c);
        }

        /* Comandos */
        if (c == 'h') {
            mostrar_ayuda();
            return 1;
        }

        if (c == 'c') {
            stack_clear(s);
            printf("Pila limpiada\n");
            return 1;
        }

        if (c == 'p') {
            double top;
            if (stack_peek(s, &top))
                printf("Tope: %g\n", top);
            else
                printf("Pila vacía\n");
            return 1;
        }

        if (c == 's') {
            mostrar_pila(s);
            return 1;
        }

        if (c == 'q') {
            return -1;
        }
    }

    /* Funciones */
    if (strcmp(tok, "sqrt") == 0) return op_sqrt(s);
    if (strcmp(tok, "sin")  == 0) return op_trig(s, "sin");
    if (strcmp(tok, "cos")  == 0) return op_trig(s, "cos");
    if (strcmp(tok, "tan")  == 0) return op_trig(s, "tan");
    if (strcmp(tok, "pow")  == 0) return op_pow(s);

    printf("Token desconocido: %s\n", tok);
    return 0;
}

/* ============================
   MAIN
   ============================ */

int main(void) {
    Stack pila;
    stack_init(&pila);

    char linea[LINE_MAX];

    mostrar_ayuda();

    while (1) {
        printf("rpn> ");

        if (!fgets(linea, sizeof(linea), stdin)) {
            break;
        }

        char *token = strtok(linea, " \t\n");

        while (token) {
            int r = procesar_token(&pila, token);

            if (r == -1) {
                printf("Saliendo...\n");
                return 0;
            }

            token = strtok(NULL, " \t\n");
        }
    }

    return 0;
}
