/*
Implemente estimativa estocástica de π
usando rand() para gerar os pontos. Cada
thread deve usar uma variável privada
para contar os acertos e acumular o total
em uma variável global com #pragma omp
critical. Depois, implemente uma segunda
versão em que cada thread escreve seus
acertos em uma posição distinta de um
vetor compartilhado. A acumulação deve
ser feita em um laço serial após a região
paralela. Compare o tempo de execução das
duas versões. Em seguida, substitua rand()
por rand_r() em ambas e compare novamente.
Explique o comportamento dos quatro
programas com base na coerência de cache
e nos efeitos do falso compartilhamento.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

#define NUM_DOTS 100000000

double get_time(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

int main() {

    int hit = 0;
    int num_threads = omp_get_max_threads();
    int hits[num_threads];

    struct timeval inicio, fim;
    double time_lapsed;

    for (int i = 0; i < num_threads; i++) {
        hits[i] = 0;
    }

    srand(time(NULL));

    gettimeofday(&inicio, NULL);

    #pragma omp parallel default(none) shared(hits) 
    {
        int tid = omp_get_thread_num();

        #pragma omp for
        for(int i = 0; i < NUM_DOTS; i++){
            double x = 2.0 * rand() / RAND_MAX - 1.0;
            double y = 2.0 * rand() / RAND_MAX - 1.0;

            if (x*x + y*y <= 1.0){
                hits[tid]++;
            }
        }
    }

    for(int i = 0; i < num_threads; i++){
        hit += hits[i];
    }

    gettimeofday(&fim, NULL);

    time_lapsed = get_time(inicio, fim);


    double pi = 4.0 * (double)hit / NUM_DOTS;
    printf("Tempo de execução: %.4f segundos\n", time_lapsed);
    printf("Estimativa de Pi com rand vetorizado: %f\n", pi);

    return 0;
}

