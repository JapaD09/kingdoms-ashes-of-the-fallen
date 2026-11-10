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

O reino é uma grade 5x5 com o castelo no meio; no começo só o 3x3 central está liberado.

- Clique numa carta de construção (ou aperte 1 a 5) e depois num lote livre pra construir.
- Botão direito em cima de um edifício demole e devolve metade do ouro.
- **Fim do turno** (ou `Espaço`) aplica a produção, cobra os impostos e consome comida.
- O painel da direita mostra quanto cada recurso vai mudar no próximo turno.
- `ESC` ou botão direito fora de um edifício cancelam a carta.

### Economia (valores iniciais)

| Edifício | Custo | Efeito |
|---|---|---|
| Fazenda | 30 ouro, 20 madeira | +8 comida por turno |
| Serraria | 35 ouro, 15 madeira | +6 madeira por turno |
| Quartel | 50 ouro, 30 madeira, 10 pedra | libera recrutamento |
| Muralha | 40 ouro, 25 pedra | +5 defesa |
| Oficina | 45 ouro, 20 madeira, 15 ferro | libera equipamentos |

Todo turno o povo paga 1 de ouro a cada 2 habitantes e come 1 de comida a cada
5 habitantes + 1 a cada 3 soldados. Se a comida acabar, parte do povo vai embora.

## Andamento

- [x] Etapa 1 — esqueleto, menu principal e validação de entrada
- [x] Interface gráfica em pixel art — título, grade do reino, cartas e painéis
- [x] Etapa 3 — economia (produção, consumo, construções)
- [x] Etapa 4 — lista encadeada de edifícios
- [ ] Etapa 5 — pilha e fila
- [ ] Etapa 6 — catálogo de itens (árvore binária)
- [ ] Etapa 7 — ordenação e buscas
- [ ] Etapas 8 e 9 — mapa da expedição e combate
- [ ] Etapa 10 — reinício e liberação de memória

## Estruturas de dados

- **Lista encadeada (R5)** — `estruturas.cpp`: os edifícios construídos. Inserção no fim em O(1)
  (guardamos o ponteiro do último nó), remoção por id tratando lista vazia / primeiro / meio / fim,
  travessia pra desenhar e somar a produção, e liberação de todos os nós ao sair.

## Testes

`testes.cpp` testa a lista e a economia sem abrir a janela:

```
g++ -std=c++17 -Wall -Wextra testes.cpp reino.cpp estruturas.cpp -o testes
./testes
```

## Organização do código

| Arquivo | O que tem |
|---|---|
| `main.cpp` | Ponto de entrada |
| `jogo.h/.cpp` | Estado da partida e regras de turno |
| `interface.h/.cpp` | Janela, telas e cliques (raylib) |
| `fonte.h/.cpp` | Fonte própria em pixel art, com acentos |
| `sprites.h/.cpp` | Desenhos do castelo, construções e ícones |
| `paleta.h` | Cores do jogo |
| `reino.h/.cpp` | Recursos, catálogo de edifícios, construção e turno |
| `expedicao.h/.cpp` | Mapa, nós e saque |
| `combate.h/.cpp` | Inimigos, dano e recompensas |
| `estruturas.h/.cpp` | Lista encadeada (pilha, fila e árvore nas próximas etapas) |
| `algoritmos.h/.cpp` | Ordenação e buscas |

## Integrantes

- **Eduarda** — reino, estruturas e algoritmos
- **Mateus** — jogo/menus, expedição e combate
