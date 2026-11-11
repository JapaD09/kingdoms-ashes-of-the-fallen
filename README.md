# Kingdoms: Ashes of the Fallen

Jogo de estratégia medieval em C++, com janela gráfica feita em [raylib](https://www.raylib.com/). O jogador administra um reino,
monta grupos de expedição e explora mapas com caminhos ramificados até enfrentar
o chefe da região. Roguelike PvE: se o grupo cair, o saque vai junto.

Projeto Final de Programação e Sistemas Computacionais II — Centro Universitário Campo Real.

## Como compilar e executar

### Windows
1. Baixe e instale a raylib pelo instalador oficial (raylib.com → *Download* → *raylib installer for Windows*).
   Ele instala em `C:\raylib` e já traz o compilador junto.
2. Dê dois cliques em `compilar.bat` (ou rode ele pelo terminal dentro da pasta do projeto).

### Linux
Com a raylib instalada no sistema, é só rodar `make` e depois `./kingdoms`.

## Como jogar

O reino é uma grade 5x5 com o castelo no meio; no começo só o 3x3 central está aberto.
A ideia de cartas e reinos inimigos veio do *9 Kings*.

**Cartas.** Todo turno vem uma mão de 5 cartas tiradas do monte (44 cartas no começo).
Clique numa carta (ou aperte 1 a 5) e depois num lote. As cartas que sobram vão pro descarte
no fim do turno, e quando o monte acaba o descarte é embaralhado de volta.

| Tipo | Cartas | Como usa |
|---|---|---|
| Prédio | Fazenda, Serraria, Mina, Casa, Mercado, Muralha, Torre, Oficina | lote livre. Em cima de um igual, sobe de nível (até 3) |
| Tropa | Quartel, Arquearia, Estábulo, Torre Arcana | igual ao prédio; cada nível manda mais tropas pra batalha |
| Encanto | Lâminas, Armaduras, Estandarte, Bênção Real | em cima de um prédio (os três primeiros só em prédio de tropa) |
| Terreno | Expansão | abre um lote de floresta colado no reino |
| Instante | Caravana | clicou, usou |

Se faltar recurso pra um prédio, a carta vira uma ordem na fila de construção.

**Batalha.** No fim de cada turno um dos 6 reinos inimigos ataca. A ordem é sorteada e muda
a cada volta, então em 6 turnos os 6 aparecem. O painel da direita mostra quem vem e a força da onda.
A cada 6 turnos vem um chefe.

| Reino | Tropas |
|---|---|
| Clã Goblin | goblin, goblin fundeiro, troll do pântano |
| Horda Orc | orc, orc lanceiro, chefe orc |
| Necrópole | esqueleto, arqueiro ossudo, cavaleiro da morte |
| Império das Areias | lanceiro de bronze, arqueiro do sol, guarda de bronze |
| Elfos Prateados | batedor, arqueiro élfico, druida antigo |
| Senhores das Cinzas | cultista, mago de fogo, draco das cinzas |

A batalha é automática e por rodadas: torres atiram, depois atacam os heróis e tropas do reino,
depois os inimigos. O dano é `ataque - defesa`, no mínimo 1. Se o exército cair, cada inimigo
que sobrou bate no castelo (as muralhas diminuem esse dano). Vencer dá ouro e cristais;
perder tira vida do castelo e parte do ouro. Castelo com 0 de vida = fim de jogo, e o
**Novo jogo** libera tudo da partida velha antes de começar outra.

**Heróis.** Na tela **Heróis** (`H`) cada invocação custa 5 cristais. Chances: comum 60%,
raro 28%, épico 10%, lendário 2%, e a 10ª invocação sem épico vem épica. Herói repetido ganha
estrela (+25% de vida e ataque). Até 3 heróis vão pra batalha; clique num herói pra escalar ou tirar.

**Outros controles**

- **Recrutar**: 2 guardas do castelo por 20 de ouro (precisa de Quartel). Guarda que morre não volta.
- Botão direito em cima de um prédio demole e devolve metade do ouro.
- `Z` desfaz a última jogada do turno, e a carta volta pra mão.
- **Fim do turno** (ou `Espaço`): produção, fila e batalha. Na batalha, `Espaço` pula a animação.

Todo turno o povo paga 1 de ouro a cada 2 habitantes e come 1 de comida a cada
5 habitantes + 1 a cada 3 guardas. Se a comida acabar, parte do povo vai embora.

### Vista 3D

O mapa abre em **3D** (dá pra voltar pro 2D no botão **Vista 2D** ou na tecla `V`).

| Tecla | O que faz |
|---|---|
| `Q` / `E` ou `←` / `→` | gira o mapa |
| `W` / `S` ou `↑` / `↓` | inclina a câmera |
| roda do mouse | zoom |
| botão do meio + arrastar | gira com o mouse |
| `R` | volta a câmera pro começo |
| `F11` | sai da tela cheia (o jogo já abre nela) ou volta |
| `Z` | desfaz a última jogada do turno |
| `H` | heróis |

A janela pode ser redimensionada à vontade.

## Andamento

- [x] Etapa 1 — esqueleto, menu principal e validação de entrada
- [x] Interface gráfica em pixel art — título, grade do reino, cartas e painéis
- [x] Cenário vivo — bosques com 5 tipos de árvore, vento, riacho, galinhas e fumaça
- [x] Resolução interna 640x360 com detalhes finos, contorno nos sprites e animações leves
- [x] Vista meio 3D com câmera que gira, janela redimensionável e tela cheia
- [x] Etapa 3 — economia (produção, consumo, construções)
- [x] Etapa 4 — lista encadeada de edifícios
- [x] Etapa 5 — pilha (desfazer) e fila de construção
- [x] Etapa 6 — catálogo de itens (árvore binária)
- [x] Etapa 7 — ordenação e buscas
- [x] Cartas com variantes, níveis de prédio e encantos
- [x] Heróis com invocação e equipe
- [x] Etapa 9 — combate contra os 6 reinos no fim de cada turno
- [x] Etapa 10 — derrota, novo jogo e liberação de memória
- [ ] Etapa 8 — mapa da expedição

## Estruturas de dados

- **Lista encadeada (R5)** — `estruturas.cpp`: os edifícios construídos. Inserção no fim em O(1)
  (guardamos o ponteiro do último nó), remoção por id tratando lista vazia / primeiro / meio / fim,
  travessia pra desenhar e somar a produção, e liberação de todos os nós ao sair.

- **Pilha (R6)** — `estruturas.cpp`: guarda as jogadas do turno (construir, subir nível, encanto, expansão...). O botão Desfazer (ou `Z`)
  tira do topo e desfaz. Quando vira o turno a pilha é limpa, porque a produção já aconteceu.
- **Fila (R7)** — `estruturas.cpp`: se faltar recurso, a construção entra na fila e é feita no fim do turno,
  na ordem em que entrou. A primeira ordem espera até ter recurso e as de trás não passam na frente;
  se o lote ficar ocupado, a ordem é cancelada.
- **Pilha de cartas** — `estruturas.cpp`/`cartas.cpp`: o monte de compra e o descarte. Pra embaralhar, a pilha
  é esvaziada num vetor, passa pelo Fisher-Yates e é empilhada de novo.
- **Árvore binária de busca (R8)** — `estruturas.cpp`: catálogo de 14 itens organizado por id, com inserção,
  busca e percurso in-ordem, todos recursivos (R3).

## Algoritmos

Na tela **Catálogo** (`algoritmos.cpp`):

- **Insertion sort (R9)** ordena por preço e mostra quantas comparações fez.
  Melhor caso O(n) (já ordenado), médio e pior caso O(n²), memória extra O(1).
- **Busca sequencial e binária (R10)** procuram o mesmo id e mostram as comparações de cada uma,
  junto com a busca na árvore. A binária usa o vetor ordenado por id (o in-ordem da árvore).

## Testes

`testes.cpp` testa as estruturas, a economia, as cartas, os heróis e a batalha sem abrir a janela.
A lógica usa um sorteio próprio com semente (`aleatorio.cpp`), então os testes dão sempre o mesmo resultado.

```
g++ -std=c++17 -Wall -Wextra testes.cpp jogo.cpp reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp \
    estruturas.cpp algoritmos.cpp aleatorio.cpp -o testes
./testes
```

## Organização do código

| Arquivo | O que tem |
|---|---|
| `main.cpp` | Ponto de entrada |
| `jogo.h/.cpp` | Estado da partida e regras de turno |
| `interface.h/.cpp` | Janela, tela do reino, catálogo e troca de telas (raylib) |
| `ui.h/.cpp` | Peças de tela usadas em todo lugar: caixas, botões, mouse |
| `tela_batalha.h/.cpp` | Tela da batalha: anima cada rodada |
| `tela_herois.h/.cpp` | Tela dos heróis: coleção, equipe e invocação |
| `figuras.h/.cpp` | Bonecos das tropas, heróis e inimigos |
| `fonte.h/.cpp` | Fonte própria em pixel art, com acentos |
| `sprites.h/.cpp` | Desenhos do castelo, construções e ícones |
| `natureza.h/.cpp` | Cenário vivo: árvores variadas, riacho, flores, galinhas e fumaça |
| `visao3d.h/.cpp` | Vista meio 3D: chão em relevo, figuras em pé, câmera que gira |
| `paleta.h` | Cores do jogo |
| `pincel.h/.cpp` | Retângulo em meia unidade, contorno fino dos sprites e escala das animações |
| `reino.h/.cpp` | Edifícios, jogadas das cartas, desfazer, fila e turno |
| `cartas.h/.cpp` | As 18 cartas e o baralho (monte, mão, descarte) |
| `herois.h/.cpp` | Os 9 heróis e a invocação |
| `inimigos.h/.cpp` | Os 6 reinos inimigos e a força das ondas |
| `aleatorio.h/.cpp` | Sorteio com semente |
| `expedicao.h/.cpp` | Mapa, nós e saque |
| `combate.h/.cpp` | Batalha em rodadas, dano e recompensas |
| `estruturas.h/.cpp` | Lista encadeada, pilha, fila e árvore binária de busca |
| `algoritmos.h/.cpp` | Insertion sort, busca sequencial e busca binária |

## Integrantes

- **Eduarda** — reino, estruturas e algoritmos
- **Mateus** — jogo/menus, expedição e combate
