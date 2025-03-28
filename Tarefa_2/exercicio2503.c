#include <stdio.h>
#define N = 1000000000;

//Usar parametros o0, o1 e o3 para compilar

int A[N];

void inicialization(){
    for(int i=0; i<N; i++){
        A[i] = i+2;
    }
}

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

int version_2(){

    int soma = 0, soma1 = 0, soma2 = 0;


    //Start timer
    double start = get_time();

    for(int i=0; i<n; i+=2){
        soma1 += A[i];
        soma2 += A[i+1];
    }

    soma = soma1 + soma2;

    //Finishing timer
    double end = get_time();


}





int main() {

    
    int soma = 0;
    

    //
    

    
    

    

    soma = soma1 + soma2;

    return 0;
}
