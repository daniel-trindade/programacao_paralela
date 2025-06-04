# Afinidade de Threads

## Resumo dos Conceitos
A afinidade de threads refere-se ao mecanismo utilizado para "prender" ou direcionar a execução de threads específicas para determinados processadores ou sockets. Trata-se de um conceito importante pois influencia na:
- **Localidade dos dados** - se uma thread acessa repetidamente os mesmos dados será interessante mantê-la no mesmo núcleo, pois assim ela poderá tirar mais proveito do acesso a cache.
- **Arquiteturas NUMA** - Em sistemas com multiplos processadores, cada um deles tem sua própria memoria local e acessar a própria memória é melhor do que acessar de outro processador.
- **Evitar _Overhead_** - Migrar threads de núcleo tem um custo e fixar threads pode ajudar a diminuir esse _overhead_.

### Cláusula proc_bind()
A cláusula ```proc_bind()``` é utilizada para definir a política de afinidade que deverá ser utilizada. Quando essa cláusula não é utilizada em uma área paralela o OpenMP recore a variável interna ICV (Internal Control Variable) para saber qual política aplicar.

As políticas de afinidade de threads avaliadas foram:

- **```proc_bind(spread)```** - Espalha as threads o máximo possível, o objetivo é dar as threads o máximo de recurso dedicado possível. No entanto aumenta consideravelmente a latencia de comuniação se eventualmente as threads precisarem trocar dados frequentemente.

- **```proc_bind(master)```** - Todas as threads são alocadas no mesmo núcleo em que a thread principal está executando. Ela maximiza a comunicação, mas causa alta disputa por recursos o que geralmente anula o paralelismo.

- **```proc_bind(close)```** - Agrupa as threads próximas geralmente no mesmo socket ou em núcleos adjacentes ao da thread principal. O objetivo é reduzir a latência de comunicação entre as threads sem gerar disputa de recursos como na política ```master```.

- **```OMP_PROC_BIND=true```** - Ativa a afinidade de thread não permitindo que o sistema operacional mova livremente as threads

- **```OMP_PROC_BIND=false```** - Desabilita as políticas de threads explicitas do código e permite que o sistema operacional mova livremente as threads caso necessário.