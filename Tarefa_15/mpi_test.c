/*
Simulação da difusão de calor em uma barra 1D com MPI
Versão MPI_Test otimizada - removida computação extra desnecessária
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

// Parâmetros otimizados para observar diferenças
#define N 5000
#define MAX_ITERS 500
#define ALPHA 0.1
#define HALO_SIZE 2
#define NUM_RUNS 3      // Múltiplas execuções para média

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            printf("Este programa requer pelo menos 2 processos MPI\n");
        }
        MPI_Finalize();
        return 1;
    }

    int local_N = N / size;
    int start = rank * local_N;
    
    // Aloca espaço extra para halo zones
    double *u = (double*)malloc((local_N + 2 * HALO_SIZE) * sizeof(double));
    double *u_new = (double*)malloc((local_N + 2 * HALO_SIZE) * sizeof(double));
    
    if (!u || !u_new) {
        printf("Erro de alocação de memória no processo %d\n", rank);
        MPI_Finalize();
        return 1;
    }

    // Arrays para armazenar requests de comunicação
    MPI_Request requests[8];  // 4 sends + 4 recvs máximo
    MPI_Status statuses[8];
    
    double total_time = 0.0;
    
    // Executa múltiplas vezes para obter média mais precisa
    for (int run = 0; run < NUM_RUNS; run++) {
        // Inicialização
        for (int i = HALO_SIZE; i < local_N + HALO_SIZE; i++) {
            int global_index = start + i - HALO_SIZE;
            if (global_index == N / 2) {
                u[i] = 100.0;
            } else {
                u[i] = 0.0;
            }
        }
        
        // Inicializa halo zones
        for (int i = 0; i < HALO_SIZE; i++) {
            u[i] = 0.0;
            u[local_N + HALO_SIZE + i] = 0.0;
        }

        MPI_Barrier(MPI_COMM_WORLD); // Sincroniza antes de medir
        double t_start = MPI_Wtime();

        for (int iter = 0; iter < MAX_ITERS; iter++) {
            int req_count = 0;
            
            // Inicia todas as comunicações não-bloqueantes
            // Envio para vizinho à direita
            if (rank != size - 1) {
                MPI_Isend(&u[local_N], HALO_SIZE, MPI_DOUBLE, rank + 1, 0, 
                         MPI_COMM_WORLD, &requests[req_count++]);
            }
            
            // Envio para vizinho à esquerda
            if (rank != 0) {
                MPI_Isend(&u[HALO_SIZE], HALO_SIZE, MPI_DOUBLE, rank - 1, 1, 
                         MPI_COMM_WORLD, &requests[req_count++]);
            }
            
            // Recebimento do vizinho à direita
            if (rank != size - 1) {
                MPI_Irecv(&u[local_N + HALO_SIZE], HALO_SIZE, MPI_DOUBLE, rank + 1, 1, 
                         MPI_COMM_WORLD, &requests[req_count++]);
            }
            
            // Recebimento do vizinho à esquerda  
            if (rank != 0) {
                MPI_Irecv(&u[0], HALO_SIZE, MPI_DOUBLE, rank - 1, 0, 
                         MPI_COMM_WORLD, &requests[req_count++]);
            }

            // Aplica condições de contorno para processos nas extremidades
            if (rank == 0) {
                for (int i = 0; i < HALO_SIZE; i++) {
                    u[i] = 0.0;
                }
            }
            if (rank == size - 1) {
                for (int i = 0; i < HALO_SIZE; i++) {
                    u[local_N + HALO_SIZE + i] = 0.0;
                }
            }

            // Computação no interior (não depende das halo zones)
            // Pode ser feita enquanto a comunicação está em andamento
            for (int i = HALO_SIZE + 1; i < local_N + HALO_SIZE - 1; i++) {
                // Operação principal de difusão
                u_new[i] = u[i] + ALPHA * (u[i - 1] - 2 * u[i] + u[i + 1]);
                
                // Computação artificial para balancear comunicação/computação
                for (int k = 0; k < 5; k++) {
                    u_new[i] += 0.0001 * sin(u_new[i] * 0.1) * cos(u_new[i] * 0.1);
                }
                
                // Operação adicional para simular problema mais complexo
                double gradient = (u[i + 1] - u[i - 1]) / 2.0;
                u_new[i] += 0.001 * gradient * gradient;
            }

            // Usa MPI_Test para verificar comunicações
            int all_complete = 0;
            int test_count = 0;
            
            while (!all_complete && req_count > 0) {
                MPI_Testall(req_count, requests, &all_complete, statuses);
                test_count++;
                
                // Evita polling excessivo - limita número de testes
                if (test_count > 50) {
                    MPI_Waitall(req_count, requests, statuses);
                    all_complete = 1;
                }
            }

            // Agora processa as bordas que dependem das halo zones
            // Borda esquerda
            if (local_N > 0) {
                int i = HALO_SIZE;
                u_new[i] = u[i] + ALPHA * (u[i - 1] - 2 * u[i] + u[i + 1]);
                
                for (int k = 0; k < 5; k++) {
                    u_new[i] += 0.0001 * sin(u_new[i] * 0.1) * cos(u_new[i] * 0.1);
                }
                
                if (i > HALO_SIZE && i < local_N + HALO_SIZE - 1) {
                    double gradient = (u[i + 1] - u[i - 1]) / 2.0;
                    u_new[i] += 0.001 * gradient * gradient;
                }
            }
            
            // Borda direita
            if (local_N > 1) {
                int i = local_N + HALO_SIZE - 1;
                u_new[i] = u[i] + ALPHA * (u[i - 1] - 2 * u[i] + u[i + 1]);
                
                for (int k = 0; k < 5; k++) {
                    u_new[i] += 0.0001 * sin(u_new[i] * 0.1) * cos(u_new[i] * 0.1);
                }
                
                if (i > HALO_SIZE && i < local_N + HALO_SIZE - 1) {
                    double gradient = (u[i + 1] - u[i - 1]) / 2.0;
                    u_new[i] += 0.001 * gradient * gradient;
                }
            }

            // Atualiza todos os valores
            for (int i = HALO_SIZE; i < local_N + HALO_SIZE; i++) {
                u[i] = u_new[i];
            }
        }

        MPI_Barrier(MPI_COMM_WORLD); // Sincroniza antes de finalizar medição
        double t_end = MPI_Wtime();
        total_time += (t_end - t_start);
    }

    double avg_time = total_time / NUM_RUNS;
    double max_avg_time;
    MPI_Reduce(&avg_time, &max_avg_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("=== MPI_TEST VERSION (OPTIMIZED) ===\n");
        printf("Parametros: N=%d, ITERS=%d, PROCESSES=%d, HALO_SIZE=%d\n", 
               N, MAX_ITERS, size, HALO_SIZE);
        printf("Tempo medio (%d execucoes): %.6f segundos\n", NUM_RUNS, max_avg_time);
        printf("Throughput: %.2f Mflops/s\n", 
               (double)(N * MAX_ITERS * 10) / (max_avg_time * 1e6)); // ~10 ops por ponto
        
        // Verifica conservação de energia (soma total deve ser aproximadamente constante)
        double local_sum = 0.0;
        for (int i = HALO_SIZE; i < local_N + HALO_SIZE; i++) {
            local_sum += u[i];
        }
        double global_sum;
        MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        printf("Energia total final: %.6f\n", global_sum);
        printf("====================================\n\n");
    } else {
        double local_sum = 0.0;
        for (int i = HALO_SIZE; i < local_N + HALO_SIZE; i++) {
            local_sum += u[i];
        }
        double global_sum;
        MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    // Opcional: salva alguns valores para verificação
    if (rank == 0) {
        printf("Primeiros valores: ");
        for (int i = HALO_SIZE; i < HALO_SIZE + 5; i++) {
            printf("%.3f ", u[i]);
        }
        printf("\n");
    }

    free(u);
    free(u_new);
    MPI_Finalize();
    return 0;
}