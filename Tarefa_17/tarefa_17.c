/*
Reimplemente a tarefa 16, agora distribuindo as colunasentre
os processos. Utilize MPI_Type_vector e MPI_Type_create_resized
para definir um tipo derivado que represente colunas da matriz.
Use MPI_Scatter com esse tipo para distribuir blocos de colunas,
e MPI_Scatter ou cópia manual para enviar os segmentos
correspondentes de x. Cada processo deve calcular uma
contribuição parcial para todos os elementos de y e usar
MPI_Reduce com MPI_SUM para somar os vetores parciais no
processo 0. Discuta as diferenças de acesso à memória e
desempenho em relação à distribuição por linhas.
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define M 16384  // Número de linhas da matriz A
#define N 16384  // Número de colunas da matriz A

int main(int argc, char *argv[]) {
    int rank, size;
    uint8_t A[M][N];   // Matriz A
    uint8_t x[N];      // Vetor x
    uint8_t y[M];      // Resultado final
    uint8_t *local_A;  // Parte da matriz A que cada processo recebe
    uint8_t *local_x;  // Parte do vetor x que cada processo recebe
    uint8_t *local_y;  // Contribuição parcial de y em cada processo
    int local_cols;    // Número de colunas que cada processo vai processar
    
    // Variáveis para medição de tempo
    double start_time, end_time, total_time;
    double max_time, min_time, avg_time;
    
    // Tipos derivados MPI
    MPI_Datatype col_type, resized_col_type;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Verifica se N é divisível pelo número de processos
    if (N % size != 0) {
        if (rank == 0) {
            printf("Erro: número de colunas (%d) não divisível pelo número de processos (%d).\n", N, size);
        }
        MPI_Finalize();
        return -1;
    }
    
    local_cols = N / size;
    
    // Aloca espaço para parte da matriz, vetor x local e vetor y parcial
    local_A = (uint8_t *)malloc(M * local_cols * sizeof(uint8_t));
    local_x = (uint8_t *)malloc(local_cols * sizeof(uint8_t));
    local_y = (uint8_t *)malloc(M * sizeof(uint8_t));
    
    if (local_A == NULL || local_x == NULL || local_y == NULL) {
        printf("Erro na alocação de memória no processo %d\n", rank);
        MPI_Finalize();
        return -1;
    }
    
    // Cria tipo derivado para representar colunas da matriz
    MPI_Type_vector(M, local_cols, N, MPI_UNSIGNED_CHAR, &col_type);
    MPI_Type_create_resized(col_type, 0, local_cols * sizeof(uint8_t), &resized_col_type);
    MPI_Type_commit(&resized_col_type);
    
    if (rank == 0) {
        printf("=== PRODUTO MATRIZ-VETOR MPI (POR COLUNAS) ===\n");
        printf("Matriz A: %dx%d\n", M, N);
        printf("Vetor x: %d elementos\n", N);
        printf("Número de processos: %d\n", size);
        printf("Colunas por processo: %d\n", local_cols);
        printf("==============================================\n\n");
        
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
        
        // Inicializa vetor resultado
        for (int i = 0; i < M; i++) {
            y[i] = 0;
        }
    }
    
    // Inicializa vetor parcial
    for (int i = 0; i < M; i++) {
        local_y[i] = 0;
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    start_time = MPI_Wtime();
    
    // Distribui as colunas da matriz A usando tipo derivado
    MPI_Scatter(A, 1, resized_col_type, local_A, M * local_cols, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    // Distribui os segmentos correspondentes do vetor x
    MPI_Scatter(x, local_cols, MPI_UNSIGNED_CHAR, local_x, local_cols, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    // Cada processo calcula sua contribuição parcial para todos os elementos de y
    for (int i = 0; i < M; i++) {
        uint16_t temp = 0;  // Para acumular e evitar overflow durante cálculo
        for (int j = 0; j < local_cols; j++) {
            temp += local_A[i * local_cols + j] * local_x[j];
        }
        local_y[i] = (uint8_t)(temp % 256);  // Resultado truncado para uint8_t
    }
    
    // Soma as contribuições parciais de todos os processos
    MPI_Reduce(local_y, y, M, MPI_UNSIGNED_CHAR, MPI_SUM, 0, MPI_COMM_WORLD);
    
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
    
    // Libera o tipo derivado
    MPI_Type_free(&col_type);
    MPI_Type_free(&resized_col_type);
    
    free(local_A);
    free(local_x);
    free(local_y);
    MPI_Finalize();
    return 0;
}