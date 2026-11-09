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

Clique nos botões do painel **Ordens do Reino** ou use as teclas de 0 a 6.
Consultar recursos não gasta turno; o reino só avança quando você escolhe **Encerrar turno**.
`ESC` fecha os painéis abertos.

## Andamento

- [x] Etapa 1 — esqueleto, menu principal e validação de entrada
- [x] Interface gráfica — tela de título, painel de recursos e botões
- [ ] Etapa 3 — economia (produção, consumo, construções)
- [ ] Etapa 4 — lista encadeada de edifícios
- [ ] Etapa 5 — pilha e fila
- [ ] Etapa 6 — catálogo de itens (árvore binária)
- [ ] Etapa 7 — ordenação e buscas
- [ ] Etapas 8 e 9 — mapa da expedição e combate
- [ ] Etapa 10 — reinício e liberação de memória

## Organização do código

| Arquivo | O que tem |
|---|---|
| `main.cpp` | Ponto de entrada |
| `jogo.h/.cpp` | Estado da partida e regras de turno |
| `interface.h/.cpp` | Janela, desenhos e cliques (raylib) |
| `reino.h/.cpp` | Recursos, população e exército |
| `expedicao.h/.cpp` | Mapa, nós e saque |
| `combate.h/.cpp` | Inimigos, dano e recompensas |
| `estruturas.h/.cpp` | Lista, pilha, fila e árvore |
| `algoritmos.h/.cpp` | Ordenação e buscas |

## Integrantes

- **Eduarda** — reino, estruturas e algoritmos
- **Mateus** — jogo/menus, expedição e combate
