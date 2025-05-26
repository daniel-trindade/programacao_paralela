#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 1000000
#define MAX_ITERS 10000
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

    double t_start = MPI_Wtime();

    for (int iter = 0; iter < MAX_ITERS; iter++) {
        MPI_Request requests[4];
        int req_count = 0;

        // Comunicação não bloqueante com vizinho à esquerda
        if (rank != 0) {
            MPI_Irecv(&u[0], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
            MPI_Isend(&u[1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
        } else {
            u[0] = 0.0;
        }

        // Comunicação não bloqueante com vizinho à direita
        if (rank != size - 1) {
            MPI_Irecv(&u[local_N + 1], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
            MPI_Isend(&u[local_N], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
        } else {
            u[local_N + 1] = 0.0;
        }

        // Espera que todas as comunicações terminem
        if (req_count > 0) {
            MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
        }

        // Cálculo da nova temperatura
        for (int i = 1; i <= local_N; i++) {
            u_new[i] = u[i] + ALPHA * (u[i - 1] - 2 * u[i] + u[i + 1]);
        }

        // Atualiza
        for (int i = 1; i <= local_N; i++) {
            u[i] = u_new[i];
        }
    }

    double t_end = MPI_Wtime();
    double local_elapsed = t_end - t_start;

    double max_elapsed;
    MPI_Reduce(&local_elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Tempo total de execucao: %f segundos\n", max_elapsed);
    }

    free(u);
    free(u_new);
    MPI_Finalize();
    return 0;
}
