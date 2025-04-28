#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <sys/time.h>

#define NUM_DOTS 100000000

double get_time(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

int main() {

    int hit = 0;
    int hits [omp_get_max_threads()]; // Vetor para armazenar contagens locais
    unsigned int seeds[omp_get_max_threads()]; // Sementes individuais para cada thread
    struct timeval inicio, fim;
    double time_lapsed;


    for (int i = 0; i < omp_get_max_threads(); i++) {
        seeds[i] = time(NULL) + i; // Sementes individuais para cada thread
        hits[i] = 0; // Inicializa o vetor de contagens locais
    }

    gettimeofday(&inicio, NULL);

    // Região paralela
    #pragma omp parallel default(none) shared(hits, seeds)
    {
        int tid = omp_get_thread_num();               // ID da thread
        unsigned int seed = seeds[tid];               // Semente local da thread
        int hit_priv = 0;                             // Contador local da thread

        // Loop de Monte Carlo dividido entre as threads
        #pragma omp for
        for(int i = 0; i < NUM_DOTS; i++){
            double x = 2.0 * rand_r(&seed) / RAND_MAX - 1.0; // x entre -1 e 1
            double y = 2.0 * rand_r(&seed) / RAND_MAX - 1.0; // y entre -1 e 1

            if (x*x + y*y <= 1.0){
                hits[tid]++;
            }
        }

    }

    for(int i = 0; i < omp_get_max_threads(); i++){
        hit += hits[i]; // Soma as contagens locais
    }

    gettimeofday(&fim, NULL);
    time_lapsed = get_time(inicio, fim);
    
    // Estimativa final de pi
    double pi = 4.0 * (double)hit / NUM_DOTS;
    printf("Tempo de execução: %.4f segundos\n", time_lapsed);
    printf("Estimativa de Pi com rand_r vetorizado: %f\n", pi);

    return 0;
}
