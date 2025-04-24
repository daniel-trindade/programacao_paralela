/*
Implemente um programa em C que cria uma
lista encadeada com nós, cada um, contendo
o nome de um arquivo fictício. Dentro de
uma região paralela, percorra a lista e
crie uma tarefa com #pragma omp task para
processar cada nó. Cada tarefa deve imprimir
o nome do arquivo e o identificador da
thread que a executou. Após executar o
programa, reflita: todos os nós foram
processados? Algum foi processado mais de
uma vez ou ignorado? O comportamento muda
entre execuções? Como garantir que cada
nó seja processado uma única vez e porapenas uma tarefa?
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct Node {
    char nome_arquivo[100];
    struct Node* prox;
} Node;

Node* criar_no(const char* nome) {
    Node* novo = (Node*)malloc(sizeof(Node));
    strcpy(novo->nome_arquivo, nome);
    novo->prox = NULL;
    return novo;
}

void adicionar_no(Node** head, const char* nome) {
    Node* novo = criar_no(nome);
    if (*head == NULL) {
        *head = novo;
    } else {
        Node* atual = *head;
        while (atual->prox != NULL)
            atual = atual->prox;
        atual->prox = novo;
    }
}

void liberar_lista(Node* head) {
    Node* temp;
    while (head) {
        temp = head;
        head = head->prox;
        free(temp);
    }
}

int main() {
    Node* lista = NULL;

    adicionar_no(&lista, "arquivo1.txt");
    adicionar_no(&lista, "arquivo2.txt");
    adicionar_no(&lista, "arquivo3.txt");
    adicionar_no(&lista, "arquivo4.txt");
    adicionar_no(&lista, "arquivo5.txt");

    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Thread %d está criando as tasks\n", omp_get_thread_num());

            Node* atual = lista;
            while (atual != NULL) {
                Node* no = atual;
                int criadora = omp_get_thread_num(); // Captura a thread que está criando

                #pragma omp task firstprivate(no, criadora)
                {
                    printf("Arquivo: %s | Task criada pela thread: %d | Executada pela thread: %d\n",
                           no->nome_arquivo, criadora, omp_get_thread_num());
                }

                atual = atual->prox;
            }
        }
    }

    liberar_lista(lista);
    return 0;
}
