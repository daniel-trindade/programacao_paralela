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

#define NX 20      // Número de pontos em x
#define NY 20      // Número de pontos em y
#define NZ 20      // Número de pontos em z
#define DX 0.01    // Espaçamento em x
#define DY 0.01    // Espaçamento em y
#define DZ 0.01    // Espaçamento em z
#define DT 0.00001 // Passo de tempo
#define NU 0.01    // Viscosidade
#define NSTEPS 5000 // Número de passos de tempo
#define SAVE_INTERVAL 100 // Salvar a cada 100 passos

// Função para inicializar o campo de velocidade
void initialize(double u[NX][NY][NZ]) {
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
void apply_boundary_conditions(double u[NX][NY][NZ]) {
    for (int i = 0; i < NX; i++) {
        for (int j = 0; j < NY; j++) {
            u[i][j][0] = 0.0;
            u[i][j][NZ-1] = 0.0;
        }
        for (int k = 0; k < NZ; k++) {
            u[i][0][k] = 0.0;
            u[i][NY-1][k] = 0.0;
        }
    }
    for (int j = 0; j < NY; j++) {
        for (int k = 0; k < NZ; k++) {
            u[0][j][k] = 0.0;
            u[NX-1][j][k] = 0.0;
        }
    }
}

// Atualização do campo
void update(double u[NX][NY][NZ], double u_new[NX][NY][NZ]) {
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

    initialize(u);

    for (int n = 0; n < NSTEPS; n++) {
        apply_boundary_conditions(u);
        update(u, u_new);
        memcpy(u, u_new, sizeof(u));

        if (n % 50 == 0) {
            printf("Passo %d: Velocidade no centro = %f\n", n, u[NX/2][NY/2][NZ/2]);
        }

        if (n % SAVE_INTERVAL == 0) {
            save_field(u, n);
        }
    }

    printf("Simulação finalizada. Arquivos gerados para animação.\n");
    return 0;
}
