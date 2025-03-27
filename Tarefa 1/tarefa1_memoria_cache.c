#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Função para medir o tempo
double get_time(clock_t start, clock_t end) {
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Multiplicacao MxV com acesso por linhas
void mat_vec_row(double** mat, double* vec, double* res, int n) {
    for (int i = 0; i < n; i++) {
        res[i] = 0.0;
        for (int j = 0; j < n; j++) {
            res[i] += mat[i][j] * vec[j];
        }
    }
}

// Multiplicacao MxV com acesso por colunas
void mat_vec_col(double** mat, double* vec, double* res, int n) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            res[i] += mat[i][j] * vec[j];
        }
    }
}

int main() {
    int sizes[] = {100, 1000, 10000, 50000, 80000};
    int num_tests = 5;

    for (int t = 0; t < num_tests; t++) {
        int n = sizes[t];
        printf("Tamanho da matriz: %d\n", n);

        // Alocação de matriz e vetores
        double** mat = (double**) malloc(n * sizeof(double*));
        double* vec = (double*) malloc(n * sizeof(double));
        double* res_row = (double*) calloc(n, sizeof(double));
        double* res_col = (double*) calloc(n, sizeof(double));

        for (int i = 0; i < n; i++) {
            mat[i] = (double*) malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) {
                mat[i][j] = (double)rand() / RAND_MAX;
            }
            vec[i] = (double)rand() / RAND_MAX;
        }

        // Tempo para acesso por linhas
        clock_t start = clock();
        mat_vec_row(mat, vec, res_row, n);
        clock_t end = clock();
        printf("Tempo (acesso por linhas): %.4f s\n", get_time(start, end));

        // Tempo para acesso por colunas
        start = clock();
        mat_vec_col(mat, vec, res_col, n);
        end = clock();
        printf("Tempo (acesso por colunas): %.4f s\n\n", get_time(start, end));

        // Liberação de memória
        for (int i = 0; i < n; i++) {
            free(mat[i]);
        }
        free(mat);
        free(vec);
        free(res_row);
        free(res_col);
    }

    return 0;
}
