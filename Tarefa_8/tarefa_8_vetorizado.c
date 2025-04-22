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
#include <omp.h>

#define NUM_DOTS 10000000

int main() {

    int hit = 0;
    int num_threads = omp_get_max_threads();
    int hits[num_threads];

    printf("Número de threads: %d\n", num_threads);

    // Inicializa o vetor de acertos
    for (int i = 0; i < num_threads; i++) {
        hits[i] = 0;
    }

    // Região paralela
    srand(time(NULL)); // Isso não é suficiente para paralelismo real, mas está conforme a tarefa
    #pragma omp parallel default(none) shared(hits) 
    {
        int tid = omp_get_thread_num();

        // Loop de Monte Carlo dividido entre as threads
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

    double pi = 4.0 * (double)hit / NUM_DOTS;
    printf("Estimativa de Pi (com rand() e vetor de acertos): %f\n", pi);

    return 0;
}

