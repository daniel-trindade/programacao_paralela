/*
####### Tarefa 5 - Programação Paralela #######
Implemente um programa em C que conte quantos
números primos existem entre 2 e um valor
máximo n. Depois, paralelize o laço principal
usando a diretiva #pragma omp parallel for sem
alterar a lógica original. Compare o tempo de
execução e os resultados das versões sequencial
e paralela. Observe possíveis diferenças no
resultado e no desempenho, e reflita sobre os
desafios iniciais da programação paralela, como
correção e distribuição de carga.

####COMPILAÇÃO#####
gcc tarefa_5.c -o sequential

####EXECUÇÃO#####
./sequential
*/


#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>

#define N 100000000

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
