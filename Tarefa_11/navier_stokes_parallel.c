/*
A clausula schedule os segunte atributos:
 - static
 - dynamic
 - guided
 - Chunksize

 Escreva um código em C que simule o movimento de um fluido
 ao longo do tempo usando a equação de Navier-Stokes,
 considerando apenas os efeitos da viscosidade.
 Desconsidere a pressão e quaisquer forças externas.
 Utilize diferenças finitas para discretizar o espaço
 e simule a evolução da velocidade do fluido no tempo.
 Inicialize o fluido parado ou com velocidade constante
 e verifique se o campo permanece estável. Em seguida,
 crie uma pequena perturbação e observe se ela se difunde
 suavemente. Após validar o código, paralelize-o com
 OpenMP e explore o impacto das cláusulas schedule e
 collapse no desempenho da execução paralela.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h> 
#include <sys/time.h>

#define NX 20                   // Número de pontos em x
#define NY 20                   // Número de pontos em y
#define NZ 20                   // Número de pontos em z
#define DX 0.01                 // Espaçamento em x
#define DY 0.01                 // Espaçamento em y
#define DZ 0.01                 // Espaçamento em z
#define DT 0.00001              // Passo de tempo
#define NU 0.01                 // Viscosidade
#define NSTEPS 5000             // Número de passos de tempo
#define SAVE_INTERVAL 100       // Salvar a cada 100 passos


double get_time(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

// Função para inicializar o campo de velocidade
// Inicializa todos os valores do campo como zero
// e coloca uma perturbação no centro do domínio
void initialize(double u[NX][NY][NZ]) {
    #pragma omp parallel for collapse(3)
    for (int i = 0; i < NX; i++) {
        for (int j = 0; j < NY; j++) {
            for (int k = 0; k < NZ; k++) {
                u[i][j][k] = 0.0;
            }
        }
    }
    int cx = NX / 2;
    int cy = NY / 2;
    int cz = NZ / 2;
    u[cx][cy][cz] = 1.0;
}

// Condições de contorno
// Fecha as bordas do domínio com velocidade zero
void apply_boundary_conditions(double u[NX][NY][NZ]) {
    // Paralelização das faces XY (planos Z=0 e Z=NZ-1)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < NX; i++) {
        for (int j = 0; j < NY; j++) {
            u[i][j][0] = 0.0;
            u[i][j][NZ-1] = 0.0;
        }
    }
    
    // Paralelização das faces XZ (planos Y=0 e Y=NY-1)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < NX; i++) {
        for (int k = 0; k < NZ; k++) {
            u[i][0][k] = 0.0;
            u[i][NY-1][k] = 0.0;
        }
    }
    
    // Paralelização das faces YZ (planos X=0 e X=NX-1)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int j = 0; j < NY; j++) {
        for (int k = 0; k < NZ; k++) {
            u[0][j][k] = 0.0;
            u[NX-1][j][k] = 0.0;
        }
    }
}

// Atualização do campo - versão paralelizada
void update(double u[NX][NY][NZ], double u_new[NX][NY][NZ]) {
    #pragma omp parallel for collapse(3) schedule(dynamic, 4)
    for (int i = 1; i < NX-1; i++) {
        for (int j = 1; j < NY-1; j++) {
            for (int k = 1; k < NZ-1; k++) {
                double dudx2 = (u[i+1][j][k] - 2*u[i][j][k] + u[i-1][j][k]) / (DX*DX);
                double dudy2 = (u[i][j+1][k] - 2*u[i][j][k] + u[i][j-1][k]) / (DY*DY);
                double dudz2 = (u[i][j][k+1] - 2*u[i][j][k] + u[i][j][k-1]) / (DZ*DZ);
                u_new[i][j][k] = u[i][j][k] + NU * DT * (dudx2 + dudy2 + dudz2);
            }
        }
    }
}

// Versão alternativa usando guided schedule
void update_guided(double u[NX][NY][NZ], double u_new[NX][NY][NZ]) {
    #pragma omp parallel for collapse(3) schedule(guided, 2)
    for (int i = 1; i < NX-1; i++) {
        for (int j = 1; j < NY-1; j++) {
            for (int k = 1; k < NZ-1; k++) {
                double dudx2 = (u[i+1][j][k] - 2*u[i][j][k] + u[i-1][j][k]) / (DX*DX);
                double dudy2 = (u[i][j+1][k] - 2*u[i][j][k] + u[i][j-1][k]) / (DY*DY);
                double dudz2 = (u[i][j][k+1] - 2*u[i][j][k] + u[i][j][k-1]) / (DZ*DZ);
                u_new[i][j][k] = u[i][j][k] + NU * DT * (dudx2 + dudy2 + dudz2);
            }
        }
    }
}

// Salva o campo de velocidade em arquivo
void save_field(double u[NX][NY][NZ], int step) {
    char filename[64];
    sprintf(filename, "data/saida3d_%04d.dat", step);
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Erro ao abrir arquivo para escrita");
        exit(1);
    }

    for (int i = 0; i < NX; i++) {
        for (int j = 0; j < NY; j++) {
            for (int k = 0; k < NZ; k++) {
                fprintf(fp, "%f ", u[i][j][k]);
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

// Função para medir o tempo de execução com diferentes configurações de schedule
void benchmark_schedules(double u[NX][NY][NZ], double u_new[NX][NY][NZ], int num_threads) {
    struct timeval start_time, end_time;
    double times[4]; // Para armazenar os tempos de diferentes schedules
    char* schedule_names[5] = {"static", "dynamic", "guided", "auto", "default"};
    
    // Configurar o número de threads
    omp_set_num_threads(num_threads);
    
    // Testes com diferentes schedules
    for (int test = 0; test < 5; test++) {
        // Reinicializar arrays
        initialize(u);
        
        gettimeofday(&start_time, NULL);
        
        for (int n = 0; n < 100000; n++) { // Teste com 100 passos de tempo
            apply_boundary_conditions(u);
            
            // Diferentes schedules para teste
            switch(test) {
                case 0: // static
                    #pragma omp parallel for collapse(3) schedule(static)
                    for (int i = 1; i < NX-1; i++) {
                        for (int j = 1; j < NY-1; j++) {
                            for (int k = 1; k < NZ-1; k++) {
                                double dudx2 = (u[i+1][j][k] - 2*u[i][j][k] + u[i-1][j][k]) / (DX*DX);
                                double dudy2 = (u[i][j+1][k] - 2*u[i][j][k] + u[i][j-1][k]) / (DY*DY);
                                double dudz2 = (u[i][j][k+1] - 2*u[i][j][k] + u[i][j][k-1]) / (DZ*DZ);
                                u_new[i][j][k] = u[i][j][k] + NU * DT * (dudx2 + dudy2 + dudz2);
                            }
                        }
                    }
                    break;
                case 1: // dynamic
                    #pragma omp parallel for collapse(3) schedule(dynamic, 4)
                    for (int i = 1; i < NX-1; i++) {
                        for (int j = 1; j < NY-1; j++) {
                            for (int k = 1; k < NZ-1; k++) {
                                double dudx2 = (u[i+1][j][k] - 2*u[i][j][k] + u[i-1][j][k]) / (DX*DX);
                                double dudy2 = (u[i][j+1][k] - 2*u[i][j][k] + u[i][j-1][k]) / (DY*DY);
                                double dudz2 = (u[i][j][k+1] - 2*u[i][j][k] + u[i][j][k-1]) / (DZ*DZ);
                                u_new[i][j][k] = u[i][j][k] + NU * DT * (dudx2 + dudy2 + dudz2);
                            }
                        }
                    }
                    break;
                case 2: // guided
                    #pragma omp parallel for collapse(3) schedule(guided, 4)
                    for (int i = 1; i < NX-1; i++) {
                        for (int j = 1; j < NY-1; j++) {
                            for (int k = 1; k < NZ-1; k++) {
                                double dudx2 = (u[i+1][j][k] - 2*u[i][j][k] + u[i-1][j][k]) / (DX*DX);
                                double dudy2 = (u[i][j+1][k] - 2*u[i][j][k] + u[i][j-1][k]) / (DY*DY);
                                double dudz2 = (u[i][j][k+1] - 2*u[i][j][k] + u[i][j][k-1]) / (DZ*DZ);
                                u_new[i][j][k] = u[i][j][k] + NU * DT * (dudx2 + dudy2 + dudz2);
                            }
                        }
                    }
                    break;
                case 3: // auto
                    #pragma omp parallel for collapse(3) schedule(auto)
                    for (int i = 1; i < NX-1; i++) {
                        for (int j = 1; j < NY-1; j++) {
                            for (int k = 1; k < NZ-1; k++) {
                                double dudx2 = (u[i+1][j][k] - 2*u[i][j][k] + u[i-1][j][k]) / (DX*DX);
                                double dudy2 = (u[i][j+1][k] - 2*u[i][j][k] + u[i][j-1][k]) / (DY*DY);
                                double dudz2 = (u[i][j][k+1] - 2*u[i][j][k] + u[i][j][k-1]) / (DZ*DZ);
                                u_new[i][j][k] = u[i][j][k] + NU * DT * (dudx2 + dudy2 + dudz2);
                            }
                        }
                    }
                    break;
                case 4: // static
                    #pragma omp parallel for collapse(3)
                    for (int i = 1; i < NX-1; i++) {
                        for (int j = 1; j < NY-1; j++) {
                            for (int k = 1; k < NZ-1; k++) {
                                double dudx2 = (u[i+1][j][k] - 2*u[i][j][k] + u[i-1][j][k]) / (DX*DX);
                                double dudy2 = (u[i][j+1][k] - 2*u[i][j][k] + u[i][j-1][k]) / (DY*DY);
                                double dudz2 = (u[i][j][k+1] - 2*u[i][j][k] + u[i][j][k-1]) / (DZ*DZ);
                                u_new[i][j][k] = u[i][j][k] + NU * DT * (dudx2 + dudy2 + dudz2);
                            }
                        }
                    }
                    break;
            }
            
            memcpy(u, u_new, sizeof(double) * NX * NY * NZ);
        }
        
        gettimeofday(&end_time, NULL);

        times[test] = get_time(start_time, end_time);
        
        printf("Schedule %-8s com %2d threads: %.6f segundos\n", 
               schedule_names[test], num_threads, times[test]);
    }
}

int main() {
    double u[NX][NY][NZ];
    double u_new[NX][NY][NZ];
    struct timeval inicio, fim;
    double time_lapsed;
    
    printf("Simulação de fluido com Navier-Stokes paralelizado com OpenMP\n");
    printf("Dimensões do domínio: %d x %d x %d\n", NX, NY, NZ);
    
    // Executar benchmark com diferentes números de threads
    printf("\n=== Comparação de diferentes configurações de schedule ===\n");
    
    // Testar com diferentes números de threads (1, 2, 4, 8)
    int thread_counts[] = {1, 2, 4, 8};
    for (int t = 0; t < 4; t++) {
        int num_threads = thread_counts[t];
        printf("\nBenchmark com %d threads:\n", num_threads);
        benchmark_schedules(u, u_new, num_threads);
    }
    
    // Executar a simulação completa com a melhor configuração (usar guided como exemplo)
    printf("\n=== Executando simulação completa ===\n");
    omp_set_num_threads(4); // Usar 4 threads para a simulação
    
    gettimeofday(&inicio, NULL);
    
    initialize(u);

    for (int n = 0; n < NSTEPS; n++) {
        apply_boundary_conditions(u);
        update_guided(u, u_new); // Usando guided schedule
        memcpy(u, u_new, sizeof(u));

        if (n % 50 == 0) {
            printf("Passo %d: Velocidade no centro = %f\n", n, u[NX/2][NY/2][NZ/2]);
        }

        if (n % SAVE_INTERVAL == 0) {
            save_field(u, n);
        }
    }
    
    gettimeofday(&fim, NULL);
    time_lapsed = get_time(inicio, fim);

    printf("Tempo total de execução: %f segundos\n", time_lapsed);
    printf("Arquivos gerados para animação.\n");

    return 0;
}