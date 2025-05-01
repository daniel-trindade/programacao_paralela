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
    int K, N;
    struct timeval inicio, fim;

    printf("Digite o número de listas: ");
    scanf("%d", &K);
    printf("Digite o número de inserções (tarefas): ");
    scanf("%d", &N);

    Node** listas = (Node**)malloc(K * sizeof(Node*));
    omp_lock_t* locks = (omp_lock_t*)malloc(K * sizeof(omp_lock_t));
    for (int i = 0; i < K; i++) {
        listas[i] = NULL;
        omp_init_lock(&locks[i]);
    }

    int seed = time(NULL);

    gettimeofday(&inicio, NULL);

    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < N; i++) {
                #pragma omp task
                {
                    int valor = rand_r(&seed) % 100;
                    int indice = rand_r(&seed) % K;

                    omp_set_lock(&locks[indice]);
                    adicionar_no(&listas[indice], valor);
                    omp_unset_lock(&locks[indice]);
                }
            }
        }
    }

    gettimeofday(&fim, NULL);
    double tempo = get_time(inicio, fim);
    

    printf("Tempo total: %.6f segundos\n", tempo);

    for (int i = 0; i < K; i++) {
        printf("Lista %d: ", i);
        //imprimir_lista(listas[i]);
        liberar_lista(listas[i]);
        omp_destroy_lock(&locks[i]);
    }

    free(listas);
    free(locks);
    return 0;
}
