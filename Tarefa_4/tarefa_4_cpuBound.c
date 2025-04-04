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
gcc -fopenmp -o tarefa_4b tarefa_4b.c -lm

###### EXECUÇÃO ######
OMP_NUM_THREADS=1 ./cpu_bound
OMP_NUM_THREADS=2 ./cpu_bound
...
*/

#include <stdio.h>
#include <math.h>
#include <omp.h>

#define SIZE 100000000

int main() {
    double result = 0.0;
    int i;
    double start, end;

    start = omp_get_wtime();
    #pragma omp parallel for reduction(+:result)
    for (i = 0; i < SIZE; i++) {
        result += sin(i) * cos(sqrt(i)) / (i + 1.0);
    }
    end = omp_get_wtime();

    printf("Tempo de execução (CPU-bound): %f segundos\n", end - start);
    printf("Resultado final: %f\n", result);

    return 0;
}

