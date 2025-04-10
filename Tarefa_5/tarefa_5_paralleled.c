/*
####COMPILAÇÃO#####
gcc -fopenmp tarefa_5.c -o sequential

####EXECUÇÃO#####
OMP_NUM_THREADS=N ./sequential

N é o número de threads que você quer usar.
*/

#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <omp.h>  // Inclui suporte ao OpenMP

#define N 10000000

double get_time(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

bool ehPrimo(int num) {
    if (num < 2) return false;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return false;
    }
    return true;
}

int main() {
    int contador = 0;
    struct timeval inicio, fim;
    double time_lapsed;

    gettimeofday(&inicio, NULL);

    // Loop paralelizado com redução da variável contador
    #pragma omp parallel for reduction(+:contador)
    for (int i = 2; i <= N; i++) {
        if (ehPrimo(i)) {
            contador++;
        }
    }

    gettimeofday(&fim, NULL); 

    time_lapsed = get_time(inicio, fim);

    printf("Quantidade de números primos eh: %d\n", contador);
    printf("Tempo gasto: %f segundos\n", time_lapsed);

    return 0;
}
