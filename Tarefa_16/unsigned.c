/*
Implemente um programa MPI que calcule o produto y=A⋅x,
onde A é uma matriz M×N e x é um vetor de tamanho N.
Divida a matriz A por linhas entre os processos com
MPI_Scatter, e distribua o vetor x inteiro com
MPI_Bcast. Cada processo deve calcular os elementos de
y correspondentes às suas linhas e enviá-los de volta
ao processo 0 com MPI_Gather. Compare os tempos com
diferentes tamanhos de matriz e número de processos.
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define M 2048  // Número de linhas da matriz A
#define N 2048  // Número de colunas da matriz A

int main(int argc, char *argv[]) {
    int rank, size;
    uint8_t A[M][N];   // Matriz A
    uint8_t x[N];      // Vetor x
    uint8_t y[M];      // Resultado final
    uint8_t *local_A;  // Parte da matriz A que cada processo recebe
    uint8_t *local_y;  // Resultado parcial de y em cada processo
    int local_rows;    // Número de linhas que cada processo vai processar
    
    // Variáveis para medição de tempo
    double start_time, end_time, total_time;
    double max_time, min_time, avg_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Verifica se M é divisível pelo número de processos
    if (M % size != 0) {
        if (rank == 0) {
            printf("Erro: número de linhas (%d) não divisível pelo número de processos (%d).\n", M, size);
        }
        MPI_Finalize();
        return -1;
    }
    
    local_rows = M / size;
    
    // Aloca espaço para parte da matriz e do vetor resultado parcial
    local_A = (uint8_t *)malloc(local_rows * N * sizeof(uint8_t));
    local_y = (uint8_t *)malloc(local_rows * sizeof(uint8_t));
    
    if (local_A == NULL || local_y == NULL) {
        printf("Erro na alocação de memória no processo %d\n", rank);
        MPI_Finalize();
        return -1;
    }
    
    if (rank == 0) {
        printf("=== PRODUTO MATRIZ-VETOR MPI ===\n");
        printf("Matriz A: %dx%d\n", M, N);
        printf("Vetor x: %d elementos\n", N);
        printf("Número de processos: %d\n", size);
        printf("Linhas por processo: %d\n", local_rows);
        printf("================================\n\n");
        
        // Inicializa a matriz A e o vetor x com valores exemplo
        printf("Inicializando matriz A e vetor x...\n");
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                A[i][j] = (uint8_t)((i + j + 1) % 256);  // Para evitar overflow
            }
        }
        
        for (int i = 0; i < N; i++) {
            x[i] = 1;  // Exemplo: todos elementos de x iguais a 1
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    start_time = MPI_Wtime();
    
    MPI_Bcast(x, N, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    MPI_Scatter(A, local_rows * N, MPI_UNSIGNED_CHAR, local_A, local_rows * N, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    for (int i = 0; i < local_rows; i++) {
        uint16_t temp = 0;  // Para acumular e evitar overflow durante cálculo
        for (int j = 0; j < N; j++) {
            temp += local_A[i * N + j] * x[j];
        }
        local_y[i] = (uint8_t)(temp % 256);  // Resultado truncado para uint8_t
    }
    
    MPI_Gather(local_y, local_rows, MPI_UNSIGNED_CHAR, y, local_rows, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    end_time = MPI_Wtime();
    total_time = end_time - start_time;
    
    // Correção: usar MPI_DOUBLE nas reduções de tempo!
    MPI_Reduce(&total_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total_time, &avg_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        avg_time /= size;
        
        printf("\n=== ESTATÍSTICAS DE TEMPO ===\n");
        printf("Tempo total de execução: %.6f segundos\n", max_time);
        printf("Tempo mínimo por processo: %.6f segundos\n", min_time);
        printf("Tempo máximo por processo: %.6f segundos\n", max_time);
        printf("Tempo médio por processo: %.6f segundos\n", avg_time);
        printf("Diferença min-max: %.6f segundos\n", max_time - min_time);
        
        long long total_ops = (long long)M * N * 2;
        double throughput = total_ops / max_time;
        printf("Throughput: %.2f operações/segundo\n", throughput);
        printf("Throughput: %.2f MFLOPS\n", throughput / 1e6);
        printf("=============================\n");
    }
    
    printf("Processo %d: tempo = %.6f segundos\n", rank, total_time);
    
    free(local_A);
    free(local_y);
    MPI_Finalize();
    return 0;
}
