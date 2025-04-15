/*
TAREFA_6: Implemente em C a estimativa estocástica
de π. Paralelize com #pragma omp parallel for e
explique o resultado incorreto. Corrija
reestruturando com #pragma omp parallel seguido de
#pragma omp for e aplicando as cláusulas private,
firstprivate, lastprivate e shared. Teste diferentes
combinações e explique como cada cláusula afeta o
comportamento do programa. Comente também como a
cláusula default(none) pode ajudar a tornar o escopo
mais claro em programas complexos.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_PONTOS 10000000

int main() {
    int dentro = 0;

    srand(time(NULL)); // Semente para números aleatórios

    // Versão INCORRETA
    #pragma omp parallel for
    for (int i = 0; i < NUM_PONTOS; i++) {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;

        if (x*x + y*y <= 1.0) {
            dentro++;
        }
    }

    double pi = 4.0 * dentro / NUM_PONTOS;
    printf("Pi estimado (incorreto): %f\n", pi);

    return 0;
}
