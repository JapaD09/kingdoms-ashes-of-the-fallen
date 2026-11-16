# Relatório técnico — Kingdoms: Ashes of the Fallen

Mateus e Eduarda — Programação e Sistemas Computacionais II, Centro Universitário Campo Real.

## O jogo e o que ele resolve

Kingdoms é um jogo de estratégia por turnos em C++. O jogador cuida de um reino pequeno: joga cartas
pra construir e melhorar prédios, junta recursos, invoca heróis e manda o grupo em expedições por um
mapa ramificado até o chefe de cada região. No fim de cada turno um dos 6 reinos inimigos ataca o
castelo. Derrotar os 6 chefes é a vitória; perder o castelo é a derrota, e aí a partida recomeça do zero.

O jogo serve de vitrine pras estruturas e algoritmos da disciplina. Por isso preferimos que cada
estrutura tivesse uma função de verdade no jogo, em vez de aparecer só num menu de demonstração.

## Por que cada estrutura

| Estrutura | Onde está | Por que ela |
|---|---|---|
| Lista encadeada | prédios do reino | o número de prédios cresce e diminui a partida inteira (construir, demolir, desfazer). Guardamos o ponteiro do último nó, então inserir no fim é O(1). |
| Pilha | botão Desfazer | desfazer é LIFO por natureza: a última jogada é a primeira a voltar. A pilha é limpa na virada do turno porque a produção já foi aplicada. |
| Pilha de cartas | monte e descarte | comprar sempre do topo. Pra embaralhar, esvaziamos num vetor, aplicamos Fisher-Yates e empilhamos de novo. |
| Fila | ordens de construção | quando falta recurso, a ordem espera e é atendida no fim do turno na ordem em que chegou (FIFO). Uma ordem de trás nunca passa na frente. |
| Árvore binária de busca | catálogo de itens | os 14 itens ficam ordenados por id; inserção, busca e percurso em ordem são recursivos. |
| Grafo de nós com ponteiros | mapa da expedição | cada nó aponta pros caminhos seguintes. Todos os nós também ficam numa lista (`todos`), pra liberar o mapa sem percorrer o grafo. |

Não usamos `std::list`, `std::stack`, `std::queue` nem `std::map` em nenhuma dessas estruturas: todas
foram feitas à mão em `estruturas.cpp` e `expedicao.cpp`.

## Memória dinâmica

Todo nó nasce com `new` e morre com `delete`. Cada estrutura tem a sua função de liberar
(`liberarEdificios`, `limparPilha`, `limparFila`, `limparCartas`, `liberarArvore` em pós-ordem, `liberarMapa`), e todas
seguem o mesmo cuidado: guardar o próximo antes de apagar o atual e deixar o ponteiro em `nullptr`
no fim.

Começar uma partida nova chama `iniciarJogo`, que libera o reino, o mapa e o catálogo antes de criar
tudo de novo. Ao fechar a janela, `encerrarJogo` libera o que restou. Os testes rodam com
AddressSanitizer e UndefinedBehaviorSanitizer, que acusariam qualquer vazamento, acesso depois do
`delete` ou `delete` duplo. Hoje não aparece nenhum.

## Ordenação

Escolhemos o insertion sort (`ordenarPorValor` em `algoritmos.cpp`). Ele é simples de explicar
na apresentação, ordena no próprio vetor e vai muito bem com listas pequenas e quase ordenadas, que é
o caso do catálogo.

- Melhor caso (já ordenado): O(n), cada item só é comparado com o vizinho.
- Caso médio e pior caso (ordem inversa): O(n²).
- Memória extra: O(1).

A função conta as comparações. Ordenar os 14 itens do catálogo por preço dá 36 comparações, número
que aparece na tela do jogo.

## Buscas sequencial e binária

As duas buscas procuram o mesmo id no mesmo conjunto, junto com a busca na árvore, e o jogo mostra
quantas comparações cada uma fez. A binária usa o vetor ordenado por id (que sai do percurso em ordem
da árvore), porque ela só funciona se o vetor estiver ordenado pelo mesmo campo da busca.

Exemplo real procurando o item 43 (Cota de malha) entre 14 itens:

| Busca | Comparações | Complexidade |
|---|---|---|
| Sequencial | 7 | O(n) |
| Binária | 1 | O(log n) |
| Árvore | 4 | O(altura), aqui altura 4 |

O 43 é justamente o meio do vetor, por isso a binária acerta de primeira. Com outros ids ela usa
até 4 comparações, que é o teto de log₂(14).

## Recursão

- `inserirItem`, `buscarItem`, `copiarEmOrdem`, `alturaArvore` e `liberarArvore` na árvore do catálogo.
- `existeCaminho` no mapa: parte de um nó e tenta cada caminho até chegar no chefe (o caso-base).
  Como o mapa é um grafo, ela marca os nós visitados pra não entrar em ciclo.

## Testes

`testes.cpp` tem 112 conferências que rodam sem abrir janela. Elas cobrem:

- estruturas vazias (remover de lista vazia, desempilhar pilha vazia, desenfileirar fila vazia, buscar em árvore vazia, ordenar vetor vazio);
- casos de borda da lista (primeiro, meio e último nó);
- LIFO e FIFO;
- economia do turno aplicada uma vez só;
- desfazer cada tipo de jogada;
- cartas, heróis e batalha;
- 200 mapas sorteados, todos com caminho até o chefe;
- vitória, derrota e reinício.

O sorteio do jogo usa semente própria (`aleatorio.cpp`), então os testes dão sempre o mesmo resultado.

Para o equilíbrio usamos o `simulador.cpp`, que joga partidas inteiras com quatro estilos de jogador.
Em 500 partidas novas, o jogador "esperto" vence 66% e o "casual" 11%. Os números estão no README.

Entradas inválidas são tratadas nas funções da lógica, não só na tela. Clique fora de lote, carta sem
recurso, ação de combate fora da lista, desfazer com a pilha vazia: tudo devolve uma mensagem em vez
de quebrar o estado.

O código compila sem nenhum aviso com `-Wall -Wextra -pedantic`.

## Limitações

- A árvore do catálogo não se balanceia sozinha. Com 14 itens inseridos numa ordem pensada ela fica
  com altura 4. Se os itens entrassem já ordenados, ela viraria uma lista.
- A pilha do Desfazer só vale dentro do turno. Depois que a produção acontece, não dá pra voltar.
- O balanceamento foi medido com jogadores simulados. Pessoas de verdade podem achar caminhos que o
  simulador não testa.
- O jogo depende da raylib pra janela, e no Windows o `compilar.bat` espera a raylib em `C:\raylib`.
