/*
Escreva um programa que cria tarefas para realizar
N inserções em duas listas encadeadas, cada uma
associada a uma thread. Cada tarefa deve escolher
aleatoriamente em qual lista inserir um número.
Garanta a integridade das listas evitando condição
de corrida e, sempre que possível, use regiões
críticas nomeadas para que a inserção em uma lista
não bloqueie a outra. Em seguida, generalize o
programa para um número de listas definido pelo
usuário. Explique por que, nesse caso, regiões
críticas nomeadas não são suficientes e por que
o uso de locks explícitos se torna necessário.
*/


#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>

typedef struct Node {
    int valor;
    struct Node* prox;
} Node;

Node* criar_no(int valor) {
    Node* novo = (Node*)malloc(sizeof(Node));
    novo->valor = valor;
    novo->prox = NULL;
    return novo;
}

void adicionar_no(Node** head, int valor) {
    Node* novo = criar_no(valor);
    if (*head == NULL) {
        *head = novo;
    } else {
        Node* atual = *head;
        while (atual->prox != NULL)
            atual = atual->prox;
        atual->prox = novo;
    }
}

void imprimir_lista(Node* head) {
    while (head) {
        printf("%d -> ", head->valor);
        head = head->prox;
    }
    printf("NULL\n");
}

void liberar_lista(Node* head) {
    Node* temp;
    while (head) {
        temp = head;
        head = head->prox;
        free(temp);
    }
}

double get_time(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

int main() {
    Node* lista1 = NULL;
    Node* lista2 = NULL;
    struct timeval inicio, fim;
    int N = 200000;

    unsigned int seed = time(NULL);

    gettimeofday(&inicio, NULL);

    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < N; i++) {
                #pragma omp task
                {
                    int valor = rand_r(&seed) % 100;
                    int escolha = rand_r(&seed) % 2;

                    if (escolha == 0) {
                        #pragma omp critical(lista1)
                        adicionar_no(&lista1, valor);
                    } else {
                        #pragma omp critical(lista2)
                        adicionar_no(&lista2, valor);
                    }
                }
            }
        }
    }

    gettimeofday(&fim, NULL);
    double time_lapsed = get_time(inicio, fim);
    printf("Tempo gasto: %f segundos\n", time_lapsed);
    //printf("Lista 1:\n");
    //imprimir_lista(lista1);
    //printf("Lista 2:\n");
    //imprimir_lista(lista2);

    liberar_lista(lista1);
    liberar_lista(lista2);
    return 0;
}

