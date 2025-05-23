/*
Implemente um programa MPI com exatamente dois processos.
O processo 0 deve enviar uma mensagem ao processo 1, que
imediatamente responde com a mesma mensagem. Meça o tempo
total de execução de múltiplas trocas consecutivas dessa
mensagem,utilizando MPI_Wtime. Registre os tempos para
diferentes tamanhos, desde mensagens pequenas (como 8
bytes) até mensagens maiores (como 1 MB ou mais). Analise
o graficamente o tempo em função do tamanho da mensagem e
identifique os regimes onde a latência domina e onde a
largura de banda se torna o fator principal.
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define MIN_SIZE 8          // 8 bytes
#define MAX_SIZE (16*1024*1024)  // 16 MB
#define NUM_ITERATIONS 1000 // Número de trocas para cada tamanho
#define WARMUP_ITERATIONS 100 // Aquecimento antes da medição

void print_header() {
    printf("# MPI Ping-Pong Benchmark\n");
    printf("# Tamanho(bytes)\tLatencia(us)\tLargura_Banda(MB/s)\tTempo_Total(us)\n");
    fflush(stdout);
}

void print_results(int size, double latency_us, double bandwidth_mbps, double total_time_us) {
    printf("%d\t\t%.6f\t\t%.2f\t\t\t%.6f\n", 
           size, latency_us, bandwidth_mbps, total_time_us);
    fflush(stdout);
}

double calculate_bandwidth(int size, double time_seconds) {
    // Largura de banda = (2 * tamanho_mensagem) / tempo
    // Multiplicamos por 2 porque a mensagem vai e volta
    // Convertemos para MB/s (1 MB = 1024*1024 bytes)
    return (2.0 * size) / (time_seconds * 1024.0 * 1024.0);
}

int main(int argc, char** argv) {
    int rank, size, provided;
    char *buffer;
    double start_time, end_time, total_time;
    double min_time, max_time, avg_time;
    MPI_Status status;
    
    // Inicializa MPI com suporte a threads (opcional para supercomputadores)
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SINGLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Verifica se temos exatamente 2 processos
    if (size != 2) {
        if (rank == 0) {
            fprintf(stderr, "Erro: Este programa requer exatamente 2 processos MPI\n");
            fprintf(stderr, "Execute com: mpirun -np 2 %s\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    // Processo 0 imprime o cabeçalho
    if (rank == 0) {
        print_header();
        printf("# Processo 0: Hostname = %s\n", getenv("HOSTNAME") ? getenv("HOSTNAME") : "unknown");
        fflush(stdout);
    }
    
    // Sincroniza os processos antes de começar
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Loop através de diferentes tamanhos de mensagem
    for (int msg_size = MIN_SIZE; msg_size <= MAX_SIZE; msg_size *= 2) {
        
        // Aloca buffer para a mensagem
        buffer = (char*) malloc(msg_size);
        if (buffer == NULL) {
            fprintf(stderr, "Erro: Falha ao alocar memória para tamanho %d\n", msg_size);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        // Inicializa o buffer com dados
        memset(buffer, rank, msg_size);
        
        // Sincroniza antes das medições
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Aquecimento - importante para supercomputadores
        for (int i = 0; i < WARMUP_ITERATIONS; i++) {
            if (rank == 0) {
                MPI_Send(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
            } else {
                MPI_Recv(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
                MPI_Send(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }
        
        // Sincroniza após aquecimento
        MPI_Barrier(MPI_COMM_WORLD);
        
        // Medições reais
        min_time = 1e9;
        max_time = 0.0;
        total_time = 0.0;
        
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            
            if (rank == 0) {
                // Processo 0: envia e recebe
                start_time = MPI_Wtime();
                
                MPI_Send(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(buffer, msg_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);
                
                end_time = MPI_Wtime();
                
                double iteration_time = end_time - start_time;
                total_time += iteration_time;
                
                if (iteration_time < min_time) min_time = iteration_time;
                if (iteration_time > max_time) max_time = iteration_time;
                
            } else {
                // Processo 1: recebe e envia de volta
                MPI_Recv(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);
                MPI_Send(buffer, msg_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }
        
        // Apenas o processo 0 calcula e imprime os resultados
        if (rank == 0) {
            avg_time = total_time / NUM_ITERATIONS;
            
            // Latência é metade do tempo de ida e volta
            double latency_us = (avg_time * 1e6) / 2.0;
            double total_time_us = avg_time * 1e6;
            
            // Largura de banda baseada no tempo médio
            double bandwidth_mbps = calculate_bandwidth(msg_size, avg_time);
            
            print_results(msg_size, latency_us, bandwidth_mbps, total_time_us);
            
            // Para debug: imprime estatísticas detalhadas para alguns tamanhos
            if (msg_size <= 1024 || msg_size >= 1024*1024) {
                fprintf(stderr, "# Debug - Tamanho %d: min=%.6fus, avg=%.6fus, max=%.6fus\n",
                       msg_size, min_time*1e6, avg_time*1e6, max_time*1e6);
            }
        }
        
        free(buffer);
        
        // Pequena pausa entre medições para estabilidade
        MPI_Barrier(MPI_COMM_WORLD);
        usleep(1000); // 1ms
    }
    
    if (rank == 0) {
        printf("# Benchmark concluído.\n");
        printf("# Estatísticas: %d iterações por tamanho, %d iterações de aquecimento\n", 
               NUM_ITERATIONS, WARMUP_ITERATIONS);
    }
    
    MPI_Finalize();
    return 0;
}