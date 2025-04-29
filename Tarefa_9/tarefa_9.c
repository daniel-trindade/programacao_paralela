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

