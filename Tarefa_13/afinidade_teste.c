/*
Avalie como a escalabilidade do seu código
de Navier-Strokes muda ao utilizar os diversos
tipos de afinidades de threads suportados
pelo sistema operacional e pelo OpenMP no
mesmo nó de computação do NPAD que utilizou
para a tarefa 12

Informações documentais:
https://www.openmp.org/spec-html/5.0/openmpsu36.html#x56-900002.6.2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h> 

#define NX 20                   
#define NY 20                   
#define NZ 20                   
#define DX 0.01                 
#define DY 0.01                 
#define DZ 0.01                 
#define DT 0.00001              
#define NU 0.01                 
#define NSTEPS 10000            
#define SAVE_INTERVAL 100       
#define MAX_THREADS 4
#define NUM_EXECUCOES 3         // Para média dos tempos

// Variável global para controlar a política de proc_bind
static int politica_atual = 0;

// Função para inicializar o campo de velocidade
void initialize(double u[NX][NY][NZ]) {
    switch(politica_atual) {
        case 0: // sem proc_bind
            #pragma omp parallel for collapse(3)
            for (int i = 0; i < NX; i++) {
                for (int j = 0; j < NY; j++) {
                    for (int k = 0; k < NZ; k++) {
                        u[i][j][k] = 0.0;
                    }
                }
            }
            break;
        case 1: // proc_bind(master)
            #pragma omp parallel for proc_bind(master) collapse(3)
            for (int i = 0; i < NX; i++) {
                for (int j = 0; j < NY; j++) {
                    for (int k = 0; k < NZ; k++) {
                        u[i][j][k] = 0.0;
                    }
                }
            }
            break;
        case 2: // proc_bind(close)
            #pragma omp parallel for proc_bind(close) collapse(3)
            for (int i = 0; i < NX; i++) {
                for (int j = 0; j < NY; j++) {
                    for (int k = 0; k < NZ; k++) {
                        u[i][j][k] = 0.0;
                    }
                }
            }
            break;
        case 3: // proc_bind(spread)
            #pragma omp parallel for proc_bind(spread) collapse(3)
            for (int i = 0; i < NX; i++) {
                for (int j = 0; j < NY; j++) {
                    for (int k = 0; k < NZ; k++) {
                        u[i][j][k] = 0.0;
                    }
                }
            }
            break;
    }
    
    int cx = NX / 2;
    int cy = NY / 2;
    int cz = NZ / 2;
    u[cx][cy][cz] = 1.0;
}

// Condições de contorno
void apply_boundary_conditions(double u[NX][NY][NZ]) {
    switch(politica_atual) {
        case 0: // sem proc_bind
            #pragma omp parallel for collapse(2) schedule(static)
            for (int i = 0; i < NX; i++) {
                for (int j = 0; j < NY; j++) {
                    u[i][j][0] = 0.0;
                    u[i][j][NZ-1] = 0.0;
                }
            }
            
            #pragma omp parallel for collapse(2) schedule(static)
            for (int i = 0; i < NX; i++) {
                for (int k = 0; k < NZ; k++) {
                    u[i][0][k] = 0.0;
                    u[i][NY-1][k] = 0.0;
                }
            }
            
            #pragma omp parallel for collapse(2) schedule(static)
            for (int j = 0; j < NY; j++) {
                for (int k = 0; k < NZ; k++) {
                    u[0][j][k] = 0.0;
                    u[NX-1][j][k] = 0.0;
                }
            }
            break;
            
        case 1: // proc_bind(master)
            #pragma omp parallel for proc_bind(master) collapse(2) schedule(static)
            for (int i = 0; i < NX; i++) {
                for (int j = 0; j < NY; j++) {
                    u[i][j][0] = 0.0;
                    u[i][j][NZ-1] = 0.0;
                }
            }
            
            #pragma omp parallel for proc_bind(master) collapse(2) schedule(static)
            for (int i = 0; i < NX; i++) {
                for (int k = 0; k < NZ; k++) {
                    u[i][0][k] = 0.0;
                    u[i][NY-1][k] = 0.0;
                }
            }
            
            #pragma omp parallel for proc_bind(master) collapse(2) schedule(static)
            for (int j = 0; j < NY; j++) {
                for (int k = 0; k < NZ; k++) {
                    u[0][j][k] = 0.0;
                    u[NX-1][j][k] = 0.0;
                }
            }
            break;
            
        case 2: // proc_bind(close)
            #pragma omp parallel for proc_bind(close) collapse(2) schedule(static)
            for (int i = 0; i < NX; i++) {
                for (int j = 0; j < NY; j++) {
                    u[i][j][0] = 0.0;
                    u[i][j][NZ-1] = 0.0;
                }
            }
            
            #pragma omp parallel for proc_bind(close) collapse(2) schedule(static)
            for (int i = 0; i < NX; i++) {
                for (int k = 0; k < NZ; k++) {
                    u[i][0][k] = 0.0;
                    u[i][NY-1][k] = 0.0;
                }
            }
            
            #pragma omp parallel for proc_bind(close) collapse(2) schedule(static)
            for (int j = 0; j < NY; j++) {
                for (int k = 0; k < NZ; k++) {
                    u[0][j][k] = 0.0;
                    u[NX-1][j][k] = 0.0;
                }
            }
            break;
            
        case 3: // proc_bind(spread)
            #pragma omp parallel for proc_bind(spread) collapse(2) schedule(static)
            for (int i = 0; i < NX; i++) {
                for (int j = 0; j < NY; j++) {
                    u[i][j][0] = 0.0;
                    u[i][j][NZ-1] = 0.0;
                }
            }
            
            #pragma omp parallel for proc_bind(spread) collapse(2) schedule(static)
            for (int i = 0; i < NX; i++) {
                for (int k = 0; k < NZ; k++) {
                    u[i][0][k] = 0.0;
                    u[i][NY-1][k] = 0.0;
                }
            }
            
            #pragma omp parallel for proc_bind(spread) collapse(2) schedule(static)
            for (int j = 0; j < NY; j++) {
                for (int k = 0; k < NZ; k++) {
                    u[0][j][k] = 0.0;
                    u[NX-1][j][k] = 0.0;
                }
            }
            break;
    }
}

// Atualização do campo
void update(double u[NX][NY][NZ], double u_new[NX][NY][NZ]) {
    switch(politica_atual) {
        case 0: // sem proc_bind
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
            
        case 1: // proc_bind(master)
            #pragma omp parallel for proc_bind(master) collapse(3) schedule(static)
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
            
        case 2: // proc_bind(close)
            #pragma omp parallel for proc_bind(close) collapse(3) schedule(static)
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
            
        case 3: // proc_bind(spread)
            #pragma omp parallel for proc_bind(spread) collapse(3) schedule(static)
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
}

// Função para executar uma simulação completa
double executar_simulacao(int n_threads) {
    double u[NX][NY][NZ];
    double u_new[NX][NY][NZ];
    
    omp_set_num_threads(n_threads);
    initialize(u);
    
    double inicio = omp_get_wtime();
    
    for (int n = 0; n < NSTEPS; n++) {
        apply_boundary_conditions(u);
        update(u, u_new);
        memcpy(u, u_new, sizeof(u));
    }
    
    double fim = omp_get_wtime();
    return fim - inicio;
}

// Função para obter o nome da política
const char* get_politica_nome(int politica) {
    switch(politica) {
        case 0: return "SEM PROC_BIND";
        case 1: return "MASTER";
        case 2: return "CLOSE";
        case 3: return "SPREAD";
        default: return "DESCONHECIDA";
    }
}

int main() {
    printf("========================================\n");
    printf("ESTUDO DE PROC_BIND - NAVIER-STOKES 3D\n");
    printf("========================================\n");
    printf("Dimensões: %d x %d x %d\n", NX, NY, NZ);
    printf("Passos de tempo: %d\n", NSTEPS);
    printf("Execuções por configuração: %d\n", NUM_EXECUCOES);
    printf("----------------------------------------\n");

    // Verificar se existe configuração de ambiente que pode interferir
    char* env_proc_bind = getenv("OMP_PROC_BIND");
    if (env_proc_bind) {
        printf("AVISO: OMP_PROC_BIND = %s (pode interferir nos testes)\n", env_proc_bind);
    }
    
    printf("\nIniciando testes...\n\n");

    // Loop pelas diferentes políticas de proc_bind
    for (politica_atual = 0; politica_atual < 4; politica_atual++) {
        printf("=== POLÍTICA: %s ===\n", get_politica_nome(politica_atual));
        
        // Loop pelos números de threads
        for (int n_threads = 1; n_threads <= MAX_THREADS; n_threads *= 2) {
            double tempos[NUM_EXECUCOES];
            double soma_tempos = 0.0;
            
            // Múltiplas execuções para calcular média
            for (int exec = 0; exec < NUM_EXECUCOES; exec++) {
                tempos[exec] = executar_simulacao(n_threads);
                soma_tempos += tempos[exec];
                
                // Pequena pausa entre execuções
                #pragma omp parallel
                {
                    volatile int dummy = 0;
                    for (int i = 0; i < 1000000; i++) dummy++;
                }
            }
            
            double tempo_medio = soma_tempos / NUM_EXECUCOES;
            
            // Calcular desvio padrão
            double soma_quadrados = 0.0;
            for (int exec = 0; exec < NUM_EXECUCOES; exec++) {
                double diff = tempos[exec] - tempo_medio;
                soma_quadrados += diff * diff;
            }
            double desvio_padrao = sqrt(soma_quadrados / NUM_EXECUCOES);
            
            printf("Threads: %d | Tempo médio: %.4f s | Desvio: %.4f s\n", 
                   n_threads, tempo_medio, desvio_padrao);
        }
        printf("\n");
    }

    printf("========================================\n");
    printf("ESTUDO CONCLUÍDO\n");
    printf("========================================\n");
    
    return 0;
}