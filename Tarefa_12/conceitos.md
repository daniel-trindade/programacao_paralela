# Resumo dos Conceitos

## Definições

- **Escalabilidade** - É a capacidade que um programa paralelo tem de lidar com o aumento na carga de trabalho ou no tamanho do problema mantendo ou melhorando o desempenho.
- **Escalabilidade Forte** - Se refere a como o desempenho de um programa se comporta quando fixamos o tamanho de um problema e variamos o número de processadores. **Em um cenário ideal**, quando dobramos o número de processadores o tempo de execução é reduzido pela metade. Suas principais limitações é a porção sequencial do código e _overheads_ de sicronização.
- **Escalabilidade Fraca** - Refere-se a como o tempo de solução varia quando o aumentamos o tamanho do problema proporcionalmente com o número de processadores. **Em um cenário ideal** o tempo de execução deveria permanecer igual.

## Metricas

- **SpeedUp (_S_)** - Tempo despendido pela execução com 1 thread dividido pelo tempo de execução com _n_ threads.
> $S = T(1)/T(n)$

- **Eficiência** - SpeedUp dividido pelo número de threads usadas
>$E=S/n(threads)$


