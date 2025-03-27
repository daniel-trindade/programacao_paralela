#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//Grafico com a quantidade de interações por digitos corretos

double get_time(clock_t start, clock_t end) {
    return (double)(end - start) / CLOCKS_PER_SEC;
}

double piByLeibniz(int n) {
    double res_pi = 0.0;

    for (int i = 0; i < n; i++) {
        res_pi += ((i % 2 == 0) ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }

    return 4.0 * res_pi;
}

int main(){

    long long int n_it = 10;
    int num_test = 10;

    for(int i=0; i<num_test; i++){

        double pi;

        clock_t start = clock();
        pi = piByLeibniz(n_it);
        clock_t end = clock();
        printf("Valor obtido para pi: %.30f Tempo de execução com %lld iterações: %.4f s\n",pi, n_it, get_time(start, end));
        n_it *=10;

    }

    return 0;
}

//pi_value 30 casas decimais: 3.141592653589793238462643383279
//pi_value 30 casas decimais: 3.141592652878837377272702724440
