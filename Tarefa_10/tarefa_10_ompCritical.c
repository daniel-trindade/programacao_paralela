/*

Pesquisar:
 - test_and_set
 - swap_and_compare
 - diferênça entre atomic e 
 - Reduction

Implemente novamente o estimador da tarefa 8
que usa contadores privados e o rand_r
substituindo o #pragma omp critical pelo
#pragma omp atomic. Compare as duas
implementações com uma 3ª versão que utiliza
apenas a cláusula reduction ao invés das
diretivas de sincronização. Reflita sobre a
aplicabilidade de desses mecanismos em termos
de desempenho e produtividade e proponha um
roteiro para quando utilizar qual mecanismo
de sincronização.

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

#define NUM_DOTS 1000000000

double get_time(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

int main() {

    int hit = 0;
    unsigned int seeds[omp_get_max_threads()];
    struct timeval inicio, fim;
    double time_lapsed;

    for (int i = 0; i < omp_get_max_threads(); i++) {
        seeds[i] = time(NULL) + i;
    }

    gettimeofday(&inicio, NULL);

    #pragma omp parallel default(none) shared(hit) private(seeds)
    {
        int tid = omp_get_thread_num();
        unsigned int seed = seeds[tid];
        int hit_priv = 0;

        #pragma omp for
        for(int i = 0; i < NUM_DOTS; i++){
            double x = 2.0 * rand_r(&seed) / RAND_MAX - 1.0; // x entre -1 e 1
            double y = 2.0 * rand_r(&seed) / RAND_MAX - 1.0; // y entre -1 e 1

            if (x*x + y*y <= 1.0){
                hit_priv++;
            }
        }

        #pragma omp critical
        {
            hit += hit_priv;
        }
    }

    gettimeofday(&fim, NULL); 

    time_lapsed = get_time(inicio, fim);

    double pi = 4.0 * (double)hit / NUM_DOTS;

    printf("Tempo gasto: %f segundos\n", time_lapsed);
    printf("Estimativa de Pi (com variável local + critical): %f\n", pi);

    return 0;
}
