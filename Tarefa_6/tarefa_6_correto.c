#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define NUM_DOTS 1000000000

int main() {

    int hit = 0;
    unsigned int seeds[omp_get_max_threads()]; // Sementes individuais para cada thread

    for (int i = 0; i < omp_get_max_threads(); i++) {
        seeds[i] = time(NULL) + i;
    }

    // Região paralela
    #pragma omp parallel default(none) shared(hit) private(seeds)
    {
        int tid = omp_get_thread_num();               // ID da thread
        unsigned int seed = seeds[tid];               // Semente local da thread
        int hit_priv = 0;                       // Contador local da thread

        // Loop de Monte Carlo dividido entre as threads
        #pragma omp for
        for(int i = 0; i < NUM_DOTS; i++){
            double x = 2.0 * rand_r(&seed) / RAND_MAX - 1.0; // x entre -1 e 1
            double y = 2.0 * rand_r(&seed) / RAND_MAX - 1.0; // y entre -1 e 1

            if (x*x + y*y <= 1.0){
                hit_priv++; // Contagem local (sem condição de corrida)
            }
        }

        // Agora, cada thread soma seu resultado à variável global
        #pragma omp critical
        {
            hit += hit_priv;
        }
    }

    // Estimativa final de pi
    double pi = 4.0 * (double)hit / NUM_DOTS;
    printf("Estimativa de Pi (com variável local + critical): %f\n", pi);

    return 0;
}
