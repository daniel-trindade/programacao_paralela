/*
Tarefa 4:
Implemente dois programas paralelos em C com OpenMP:
um limitado por memória, com somas simples em vetores,
e outro limitado por CPU, com cálculos matemáticos
intensivos. Paralelize com #pragma omp parallel for e
meça o tempo de execução variando o número de threads.
Analise quando o desempenho melhora, estabiliza ou piora,
e reflita sobre como o multithreading de hardware pode 
ajudar em programas memory-bound, mas atrapalhar em 
programas compute-bound pela competição por recursos.

###### COMPILAÇÃO ######

gcc -fopenmp -o tarefa_4a tarefa_4a.c

###### EXECUÇÃO ######
Executar alterando o numero de threads para analizar o comportamento
OMP_NUM_THREADS=1 ./memory_bound
OMP_NUM_THREADS=2 ./memory_bound
...
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 100000000

int main() {

    double *a = (double *)malloc(SIZE * sizeof(double));
    double start, end;

    start = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 0; i < SIZE; i++) {
        a[i] = i+2;
    }
    end = omp_get_wtime();

    printf("Tempo de execução (Memory-bound): %f segundos\n", end - start);

    free(a);

    return 0;
}

