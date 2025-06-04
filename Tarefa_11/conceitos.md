# Resumo dos conceitos

Essa tarefa foi desenvolvida com o intuito de ajudar a compreender como funciona as cláusulas schedule e colapse

## Colapse

O colapse(_n_) atua como um achatador de laços de repetição aninhados. Quando temos:

```
#pragma omp parallel for
for(int i=0; i<n; i++){
   for(int j=0; j<n; j++){
       // ... trabalho aqui ...
   }
}
```
A paralelização so acontece para o loop mais externo, ou seja, apenas as interações do loop mais externo serão distribuidas entre as threads

```
#pragma omp parallel for colapse(2)
for(int i=0; i<n; i++){
   for(int j=0; j<n; j++){
       // ... trabalho aqui ...
   }
}
```

A clausula colapse irá transformar esse loop aninhado em um só e todas interações serão distribuidas entre as threads existentes na área paralela.

Usamos colapse(n) onde _n_ é o número de loops aninhados que serão achatados. No trabalho realizado apesar de termos 3 loops aninhados, o melhor desempenho aconteceu quando usamos o colapse(2). Isso pode ser devido a como as informações do loop estavam organizados na cache, ou o _Overhead_ do colapse pode ter incluenciado.

## Schedule

A clausula schedule é utilizada para organizar como as interações são divididos entre as threads e por isso temos diferêntes políticas de trabalho:

### schedule(static, chunk_size)
Essa politica distribui as interações **antes da execução do loop** e de forma ciclica entre as threads, ou seja:

- Thread 0 <- interação 1, 2, 3 
- Thread 1 <- interação 4, 5, 6 
- Thread 2 <- interação 7, 8, 9 
...


Esse é seu comportamento padrão, a menos que alteremos o valor do chunk_size:

se usarmos ```schedule(2)``` teremos 

- Thread 0 <- interação 1 e 2 | 7 e 8
- Thread 1 <- interação 3 e 4 | 9 e 10
- Thread 2 <- interação 5 e 6 | 11 e 12

Essa politica funciona muito bem para laços homogênios bem balanciados, onde todas as iterações despende do mesmo tempo.

### schedule(dynamic, chunk_size)
Essa politica distribui as interações **durante a execução do loop**  conforme a demanda das threads, por exemplo: Se estivermos ttrabalhando com 2 threads será dado um bloco de iterações para cada thread do tamanho do chunk e conforme elas acabem de processar as iterações que receberam, será dado para elas outro chunk de mesmo tamanho até que não haja mais iterações a serem processadas.

O trabalho nessa política dica dimencionado sob demanda das threads, assim se o primeiro chunk de iterações despender de muito tempo as outras threads vão tralhando em chunks que exijam menos tempo.

Essa política trás consigo um grande _overhead_ graças ao gerenciamento da distribuição de carga de trabalho, mas esse _overhead_ acaba sendo aceitável quando trabalhamos com iterações desbalanciadas e distribuidas de forma heterogênia.

### schedule (guided, chunk_size)
O guided distribui as interações entregando primeiro blocos grandes e a medida que o trabalho vai sendo executado o tamanho desses blocos diminue exponencialmente, exemplo:

com 3 threads e 1000 iterações teríamos:
- **t0 pede trabalho** - guided divide 1000(it)/3(thr) e entrega 333 iterações para serem executadas;
- **t1 pede trabalho** - guided divide 667(it)/3(thr) e entrega 222 iterações para serem executadas;
- **t2 pede trabalho** - guided divide 445(it)/3(thr) e entrega 148 iterações para serem executadas;
- **aprimeira thread que terminar pedirá trabalho** - guided divide 297(it)/(3thr) e entregará 99 iterações para serem executadas.
- A divisão continua da mesma forma até que não haja mais iterações a serem executadas.

Quando se estabelece um ```chunk_size```, guided garante que o menor bloco a ser entregue terá o tamanho do chunk definido.

### schedule(auto)
Quando usado, você está delegando para o compilador e/ou o OpenMP decidir qual das políticas é mais apropriada para o loop em questão. No caso desse projeto, os tempos obtidos com o ```schedule(auto)``` foram muito semelhantes ao do ```schedule(static)```. E de fato o ```schedule(static)``` teve o melhor desempenho dentre as políticas testadas. Isso valida a acertividade do ```schedule(auto)```.