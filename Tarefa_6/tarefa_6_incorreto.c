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

####COMPILAÇÃO#####
gcc -fopenmp tarefa_6_incorreto.c -o incorreto

####EXECUÇÃO#####
./incorreto

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_DOTS 1000000000

int main() {
    int hit = 0;

    srand(time(NULL));

    #pragma omp parallel for
    for (int i = 0; i < NUM_DOTS; i++) {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;

        if (x*x + y*y <= 1.0) {
            hit++;
        }
    }

    double pi = 4.0 * hit / NUM_DOTS;
    printf("Pi estimado (incorreto): %f\n", pi);

    return 0;
}
