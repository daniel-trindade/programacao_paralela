/*
Implemente três laços em C para investigar os
efeitos do paralelismo ao nível de instrução
(ILP): 1) inicialize um vetor com um cálculo
simples; 2) some seus elementos de forma
acumulativa, criando dependência entre as
iterações; e 3) quebre essa dependência
utilizando múltiplas variáveis. Compare o
tempo de execução das versões compiladas com
diferentes níveis de otimização (O0, O2, O3) e
analise como o estilo do código e as dependências
influenciam o desempenho.

###Compilação###

gcc -O0 tarefa_2.c -o teste_O0

gcc -O2 tarefa_2.c -o teste_O2

gcc -O3 tarefa_2.c -o teste_O3
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 1000000000

//Função que retorna o tempo decorrido em segundos
double get_time(clock_t start, clock_t end){
    return (double)(end - start) / CLOCKS_PER_SEC;
}


// Função 1: Inicializa o vetor com um cálculo simples
void init_array(double *array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = i * 0.3;
    }
}

// Função 2: Soma acumulativa
double sum_cumulative(double *array, int size) {
    double sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

// Função 3: Soma acumulativa com múltiplas variáveis
double sum_parallel(double *array, int size) {
    double sum1 = 0;
    double sum2 = 0;
    for (int i = 0; i < size; i += 2) {
        sum1 += array[i];
        if (i + 1 < size) {
            sum2 += array[i + 1];
        }
    }
    return sum1 + sum2;
}

int main() {

    double *array = malloc(N * sizeof(double));
    if (!array) {
        printf("Erro ao alocar memória.\n");
        return 1;
    }

    clock_t start, end;
    double time_spent;
    double result_sum;

    // Loop 1: Inicialização do vetor
    start = clock();
    init_array(array, N);
    end = clock();
    time_spent = get_time(start, end);
    printf("Loop 1 (Inicialização): %f segundos\n", time_spent);
    
    // Loop 2: Soma acumulativa
    start = clock();
    result_sum = sum_cumulative(array, N);
    end = clock();
    time_spent = get_time(start, end);
    printf("Loop 2 (Soma Acumulativa): %f segundos, soma = %f\n", time_spent, result_sum);
    
    // Loop 3: Soma com múltiplas variáveis (quebra de dependência)
    start = clock();
    result_sum = sum_parallel(array, N);
    end = clock();
    time_spent = get_time(start, end);
    printf("Loop 3 (Soma com Múltiplas Variáveis): %f segundos, soma = %f\n", time_spent, result_sum);
    
    free(array);

    return 0;
}




/*
 int version_1(){
    int sum = 0;

    //Start timer
    double start = get_time();

    for(int i=0; i<N; i++){
        sum+= A[i];
    }

    //Finishing timer
    double end = get_time();

    printf("Versão 2 - Soma Acumulativa: %.6f segundos")
} 
*/
