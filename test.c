#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>

#define NX 100
#define NY 100
#define NT 10000
#define DX 1.0
#define DY 1.0
#define DT 0.1
#define NU 0.1  // viscosidade
#define MAX_THREADS 32

void initialize(double u[NX][NY]) {
    for (int i = 0; i < NX; i++)
        for (int j = 0; j < NY; j++)
            u[i][j] = 0.0;

    // Adiciona uma perturbação no centro
    u[NX/2][NY/2] = 1.0;
}

void apply_boundary_conditions(double u[NX][NY]) {
    // Paralelização das bordas horizontais (superior e inferior)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int i = 0; i < NX; i++) {
        u[i][0] = 0.0;           // borda inferior
        u[i][NY-1] = 0.0;       // borda superior
    }

    // Paralelização das bordas verticais (esquerda e direita)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int j = 0; j < NY; j++) {
        u[0][j] = 0.0;          // borda esquerda
        u[NX-1][j] = 0.0;       // borda direita
    }
}


void update(double u[NX][NY], double un[NX][NY]) {
    for (int i = 1; i < NX - 1; i++) {
        for (int j = 1; j < NY - 1; j++) {
            u[i][j] = un[i][j] + NU * DT * (
                (un[i+1][j] - 2 * un[i][j] + un[i-1][j]) / (DX * DX) +
                (un[i][j+1] - 2 * un[i][j] + un[i][j-1]) / (DY * DY)
            );
        }
    }
}

void copy(double from[NX][NY], double to[NX][NY]) {
    for (int i = 0; i < NX; i++)
        for (int j = 0; j < NY; j++)
            to[i][j] = from[i][j];
}

int main() {
    static double u[NX][NY], un[NX][NY];

    for(int threads=1; threads<=MAX_THREADS; threads*=2){
        omp_set_num_threads(threads);
        int threads_utilizadas = 0;
        #pragma omp parallel
        {
            #pragma omp single
            threads_utilizadas = omp_get_num_threads();
        }
        initialize(u);

        double start = omp_get_wtime();

        for (int n = 0; n < NT; n++) {
          copy(u, un);
          update(u, un);
          apply_boundary_conditions(u);
        }

        double end = omp_get_wtime();

        double tempo_execucao = end - start;
        printf("Threads Utilizadas: %d Tempo de execucao: %.6f segundos\n", threads_utilizadas, tempo_execucao);
    }

    return 0;
}