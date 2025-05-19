/*
Avalie a escalabilidade do seu código de
Navier-Strokes utilizando algum nó de
computação do NPAD. Procure identificar
gargalos de escalabilidade e reporte o
seu progresso em versões sucessivas da
evolução do código otimizado. Comente
sobre a escalabilidade, a escalabilidade
fraca e a escalabilidade fortes das
versões
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h> 

#define NX 20                   // Número de pontos em x
#define NY 20                   // Número de pontos em y
#define NZ 20                   // Número de pontos em z
#define DX 0.01                 // Espaçamento em x
#define DY 0.01                 // Espaçamento em y
#define DZ 0.01                 // Espaçamento em z
#define DT 0.00001              // Passo de tempo
#define NU 0.01                 // Viscosidade
#define NSTEPS 100000             // Número de passos de tempo
#define SAVE_INTERVAL 100       // Salvar a cada 100 passos
#define MAX_THREADS 8

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

int main() {
    double u[NX][NY][NZ];
    double u_new[NX][NY][NZ];

    printf("Simulação de fluido com Navier-Stokes paralelizado com OpenMP\n");
    printf("Dimensões do domínio: %d x %d x %d\n", NX, NY, NZ);

    for (int n_threads = 1; n_threads <= MAX_THREADS; n_threads *= 2) {
        omp_set_num_threads(n_threads);

        // Força criação da região paralela para medir o número real
        int threads_utilizadas = 0;
        #pragma omp parallel
        {
            #pragma omp single
            threads_utilizadas = omp_get_num_threads();
        }

        initialize(u);
        double inicio = omp_get_wtime();

        for (int n = 0; n < NSTEPS; n++) {
            apply_boundary_conditions(u);
            update_guided(u, u_new);
            memcpy(u, u_new, sizeof(u));
        }

        double fim = omp_get_wtime();
        double tempo = fim - inicio;

        printf("Threads: %d | Tempo de execução: %f segundos\n", threads_utilizadas, tempo);
    }

    printf("Execução finalizada para todas as configurações.\n");
    return 0;
}
