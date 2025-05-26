/*
Implemente uma simulação da difusão de calor
em uma barra 1D, dividida entre dois ou mais
processos MPI. Cada processo deve simular um
trecho da barra com células extras para troca
de bordas com vizinhos. Implemente três
versões: uma com MPI_Send/ MPI_Recv, outra
com MPI_Isend/ MPI_Irecv e MPI_Wait, e uma
terceira usando MPI_Test para atualizar os
pontos internos enquanto aguarda a
comunicação. Compare os tempos de execução e
discuta os ganhos com sobreposição de
comunicação e computação.
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 100
#define MAX_ITERS 1000
#define ALPHA 0.1

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int local_N = N / size;
    int start = rank * local_N;

    double *u = (double*)malloc((local_N + 2) * sizeof(double));
    double *u_new = (double*)malloc((local_N + 2) * sizeof(double));

    for (int i = 1; i <= local_N; i++) {
        int global_index = start + i - 1;
        if (global_index == N / 2) {
            u[i] = 100.0;
        } else {
            u[i] = 0.0;
        }
    }

    // Medição do tempo de execução
    double t_start = MPI_Wtime();

    for (int iter = 0; iter < MAX_ITERS; iter++) {
        if (rank != 0) {
            MPI_Send(&u[1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
            MPI_Recv(&u[0], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else {
            u[0] = 0.0;
        }

        if (rank != size - 1) {
            MPI_Send(&u[local_N], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            MPI_Recv(&u[local_N + 1], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else {
            u[local_N + 1] = 0.0;
        }

        for (int i = 1; i <= local_N; i++) {
            u_new[i] = u[i] + ALPHA * (u[i-1] - 2*u[i] + u[i+1]);
        }

        for (int i = 1; i <= local_N; i++) {
            u[i] = u_new[i];
        }
    }

    double t_end = MPI_Wtime();
    double local_elapsed = t_end - t_start;

    // Obtém o tempo máximo entre todos os processos
    double max_elapsed;
    MPI_Reduce(&local_elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);


    // Impressão final do resultado (um processo de cada vez)
    for (int p = 0; p < size; p++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (p == rank) {
            printf("Processo %d: ", rank);
            for (int i = 1; i <= local_N; i++) {
                printf("%.2f ", u[i]);
            }
            printf("\n");
        }
    }

    if (rank == 0) {
        printf("Tempo total de execucao: %f segundos\n", max_elapsed);
    }

    free(u);
    free(u_new);
    MPI_Finalize();
    return 0;
}
