// rpn_alt.c — Calculadora RPN alternativa
// Compilar: gcc rpn_alt.c -o rpn_alt -lm
// Ejecutar: ./rpn_alt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_STACK 1024
#define MAX_LINE  2048

typedef struct {
    double values[MAX_STACK];
    int count;
} Pila;

/* ===== Manejo de pila ===== */

void pila_iniciar(Pila *p) {
    p->count = 0;
}

int pila_agregar(Pila *p, double n) {
    if (p->count >= MAX_STACK) return 0;
    p->values[p->count++] = n;
    return 1;
}

int pila_quitar(Pila *p, double *n) {
    if (p->count == 0) return 0;
    *n = p->values[--p->count];
    return 1;
}

int pila_ver_tope(Pila *p, double *n) {
    if (p->count == 0) return 0;
    *n = p->values[p->count - 1];
    return 1;
}

void pila_vaciar(Pila *p) {
    p->count = 0;
}

void pila_mostrar(Pila *p) {
    int limite = 8;

    printf("Pila:\n");

    for (int i = limite; i >= 1; i--) {
        double v = 0.0;

        if (i <= p->count) {
            v = p->values[p->count - i];
        }

        printf("%d. %.6f\n", i, v);
    }
}

/* ===== Utilidades ===== */

int es_numero(const char *txt, double *out) {
    char *fin;
    double val = strtod(txt, &fin);

    if (fin == txt || *fin != '\0') return 0;

    *out = val;
    return 1;
}

/* ===== Operaciones ===== */

int operar_binario(Pila *p, char operador) {
    double x, y;

    if (!pila_quitar(p, &y) || !pila_quitar(p, &x)) {
        printf("Error: pila insuficiente para '%c'\n", operador);
        return 0;
    }

    double resultado;

    switch (operador) {
        case '+': resultado = x + y; break;
        case '-': resultado = x - y; break;
        case '*': resultado = x * y; break;
        case '/':
            if (y == 0) {
                printf("Error: división por cero\n");
                pila_agregar(p, x);
                pila_agregar(p, y);
                return 0;
            }
            resultado = x / y;
            break;
        default:
            return 0;
    }

    pila_agregar(p, resultado);
    printf("= %g\n", resultado);
    return 1;
}

int operar_unario(Pila *p, const char *op) {
    double a;

    if (!pila_quitar(p, &a)) {
        printf("Error: pila insuficiente para '%s'\n", op);
        return 0;
    }

    double r;

    if (!strcmp(op, "sqrt")) {
        if (a < 0) {
            printf("Error: raíz negativa\n");
            pila_agregar(p, a);
            return 0;
        }
        r = sqrt(a);
    }
    else if (!strcmp(op, "sin")) {
        r = sin(a * M_PI / 180.0);
    }
    else if (!strcmp(op, "cos")) {
        r = cos(a * M_PI / 180.0);
    }
    else if (!strcmp(op, "tan")) {
        r = tan(a * M_PI / 180.0);
    }
    else {
        printf("Operador desconocido: %s\n", op);
        pila_agregar(p, a);
        return 0;
    }

    pila_agregar(p, r);
    printf("= %g\n", r);
    return 1;
}

int operar_potencia(Pila *p) {
    double exp, base;

    if (!pila_quitar(p, &exp) || !pila_quitar(p, &base)) {
        printf("Error: pila insuficiente para 'pow'\n");
        return 0;
    }

    double r = pow(base, exp);
    pila_agregar(p, r);
    printf("= %g\n", r);
    return 1;
}

/* ===== Ayuda ===== */

void ayuda() {
    printf("Calculadora RPN\n");
    printf("Ejemplo: 3 4 +\n");
    printf("Operadores: + - * /\n");
    printf("Funciones: sqrt sin cos tan pow\n");
    printf("Trigonometría en grados\n");
    printf("Comandos:\n");
    printf(" p -> ver tope\n");
    printf(" s -> mostrar pila\n");
    printf(" c -> limpiar pila\n");
    printf(" h -> ayuda\n");
    printf(" q -> salir\n");
}

/* ===== Programa principal ===== */

int main() {

    Pila pila;
    pila_iniciar(&pila);

    ayuda();

    char linea[MAX_LINE];

    while (1) {

        printf("rpn> ");
        fflush(stdout);

        if (!fgets(linea, sizeof(linea), stdin))
            break;

        linea[strcspn(linea, "\n")] = 0;

        char *tok = strtok(linea, " \t");

        while (tok != NULL) {

            if (!strcmp(tok, "q"))
                return 0;

            else if (!strcmp(tok, "h"))
                ayuda();

            else if (!strcmp(tok, "c")) {
                pila_vaciar(&pila);
                printf("[pila limpia]\n");
            }

            else if (!strcmp(tok, "p")) {
                double t;
                if (pila_ver_tope(&pila, &t))
                    printf("tope: %g\n", t);
                else
                    printf("[pila vacía]\n");
            }

            else if (!strcmp(tok, "s"))
                pila_mostrar(&pila);

            else if (!strcmp(tok, "pow"))
                operar_potencia(&pila);

            else if (!strcmp(tok, "sqrt") ||
                     !strcmp(tok, "sin")  ||
                     !strcmp(tok, "cos")  ||
                     !strcmp(tok, "tan"))
                operar_unario(&pila, tok);

            else if (strlen(tok) == 1 && strchr("+-*/", tok[0]))
                operar_binario(&pila, tok[0]);

            else {
                double n;
                if (es_numero(tok, &n))
                    pila_agregar(&pila, n);
                else
                    printf("Token inválido: '%s'\n", tok);
            }

            tok = strtok(NULL, " \t");
        }
    }

    return 0;
}
