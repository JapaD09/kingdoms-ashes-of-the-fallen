# Relatório técnico — Kingdoms: Ashes of the Fallen

Mateus e Eduarda — Programação e Sistemas Computacionais II, Centro Universitário Campo Real.

## O projeto

Jogo de estratégia por turnos em C++ com raylib. O jogador joga cartas pra construir prédios, junta
recursos, invoca heróis e faz expedições por mapas ramificados até o chefe de cada um dos 6 reinos
inimigos. A regra que seguimos foi dar a cada estrutura de dados uma função real dentro do jogo.
A lógica não inclui a raylib, então tudo pode ser testado sem abrir janela.

## Por que cada estrutura

| Estrutura | Onde | Por que ela |
|---|---|---|
| Lista encadeada | prédios do reino | a quantidade muda a partida toda (construir, demolir, desfazer). Com o ponteiro do último nó, inserir no fim é O(1), e remover não desloca ninguém. |
| Pilha | botão Desfazer; monte e descarte das cartas | desfazer é LIFO: a última jogada volta primeiro. No baralho, sempre se compra do topo. |
| Fila | ordens de construção sem recurso | as ordens devem ser atendidas na ordem em que foram pedidas (FIFO). Nenhuma passa na frente. |
| Árvore binária de busca | catálogo de 14 itens | mantém os itens ordenados por id. Busca, inserção e percurso em ordem são recursivos. |
| Grafo com ponteiros | mapa da expedição | cada nó aponta pros caminhos seguintes. A recursão `existeCaminho` garante que sempre dá pra chegar no chefe. |

Todas foram feitas à mão em `estruturas.cpp` e `expedicao.cpp`, sem `std::list`, `std::stack`,
`std::queue` ou `std::map`.

## Ordenação

Implementamos o **insertion sort** (`ordenarPorValor`, em `algoritmos.cpp`). Ele ordena o catálogo por
preço. Escolhemos ele porque é simples de explicar, ordena no próprio vetor e vai bem com listas
pequenas e quase ordenadas, que é o nosso caso.

- Melhor caso (vetor já ordenado): **O(n)**.
- Caso médio e pior caso (ordem inversa): **O(n²)**.
- Memória extra: **O(1)**.

A função conta as comparações. Com os 14 itens do jogo, foram 36.

## Buscas

A busca sequencial (O(n)) e a binária (O(log n)) procuram o mesmo id no mesmo conjunto, e o jogo mostra
quantas comparações cada uma fez. A binária usa o vetor ordenado por id, que sai do percurso em ordem
da árvore, porque ela só funciona se o vetor estiver ordenado pelo campo procurado. Exemplo com o id
43: sequencial 7 comparações, binária 1, árvore 4.

Com só 14 itens o tempo nem aparece, então a demo do terminal (`demo.cpp`) também cronometra
1000 buscas num vetor de 10 mil itens: a sequencial faz 10000 comparações por busca e levou
perto de 4 ms; a binária faz 14 e levou 0,01 ms.

## Gerenciamento de memória

- Todo nó das estruturas nasce com `new` e é apagado com `delete`.
- Cada estrutura tem a sua função de liberar: `liberarEdificios`, `limparPilha`, `limparFila`,
  `limparCartas`, `liberarArvore` e `liberarMapa`. A da árvore é em pós-ordem: filhos antes do pai.
- Ao percorrer pra apagar, o ponteiro do próximo é guardado antes do `delete`, e no fim o ponteiro
  da estrutura volta pra `nullptr`.
- O mapa guarda todos os seus nós também numa lista (`todos`), pra ser liberado sem percorrer o grafo.
- **Novo jogo** chama `iniciarJogo`, que libera reino, mapa e catálogo antes de criar tudo de novo.
  Fechar a janela chama `encerrarJogo`, que libera o que restou.
- Os 112 testes rodam com AddressSanitizer, que acusaria vazamento, acesso depois do `delete` ou
  `delete` duplo. Nenhum aparece.

## Testes e limitações

O `testes.cpp` cobre estruturas vazias, LIFO, FIFO, a árvore, a economia do turno, o desfazer, as
cartas, a batalha, 200 mapas sorteados, vitória, derrota e reinício. O código compila sem avisos com
`-Wall -Wextra -pedantic`. O `simulador.cpp` mediu o balanceamento: o jogador "esperto" vence 66%
das partidas.

Limitações: a árvore não se balanceia sozinha (itens inseridos em ordem a transformariam numa lista),
o Desfazer só vale dentro do turno e o balanceamento foi medido com jogadores simulados.
