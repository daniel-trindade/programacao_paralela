#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_REPETICOES 1000  // Número de trocas para cada tamanho

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) {
            printf("Este programa requer exatamente 2 processos.\n");
        }
        MPI_Finalize();
        return 0;
    }

    // Tamanhos de mensagens a serem testadas (em bytes)
    int tamanhos[] = {8, 1024, 10*1024, 100*1024, 1024*1024};
    int num_tamanhos = sizeof(tamanhos) / sizeof(tamanhos[0]);

    for (int t = 0; t < num_tamanhos; t++) {
        int tamanho_msg = tamanhos[t];
        char* buffer = (char*)malloc(tamanho_msg);
        memset(buffer, 'A', tamanho_msg);

        MPI_Barrier(MPI_COMM_WORLD);  // Sincroniza antes de medir o tempo
        double start_time = MPI_Wtime();

        for (int i = 0; i < NUM_REPETICOES; i++) {
            if (rank == 0) {
                MPI_Send(buffer, tamanho_msg, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer, tamanho_msg, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else if (rank == 1) {
                MPI_Recv(buffer, tamanho_msg, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(buffer, tamanho_msg, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }

        double end_time = MPI_Wtime();

        if (rank == 0) {
            double total_time = end_time - start_time;
            printf("Tamanho da mensagem: %d bytes, Tempo total para %d trocas: %f segundos\n",
                   tamanho_msg, NUM_REPETICOES, total_time);
        }

        free(buffer);
    }

    MPI_Finalize();
    return 0;
}
