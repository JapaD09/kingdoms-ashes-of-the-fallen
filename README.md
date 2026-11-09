# Kingdoms: Ashes of the Fallen

Jogo de estratégia medieval para terminal, em C++. O jogador administra um reino,
monta grupos de expedição e explora mapas com caminhos ramificados até enfrentar
o chefe da região. Roguelike PvE: se o grupo cair, o saque vai junto.

Projeto Final de Programação e Sistemas Computacionais II — Centro Universitário Campo Real.

## Como compilar e executar

Precisa de um `g++` com suporte a C++17.

```
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp jogo.cpp reino.cpp expedicao.cpp combate.cpp estruturas.cpp algoritmos.cpp -o kingdoms
```

No Linux também dá pra usar `make`.

Depois é só rodar `./kingdoms` (ou `kingdoms.exe` no Windows).

## Como jogar

Tudo é feito pelo menu, digitando o número da opção. Consultar menus não gasta turno;
o reino só avança quando você escolhe **Encerrar turno**.

## Andamento

- [x] Etapa 1 — esqueleto, menu principal e validação de entrada
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
| `jogo.h/.cpp` | Estado da partida, menus e leitura de opções |
| `reino.h/.cpp` | Recursos, população e exército |
| `expedicao.h/.cpp` | Mapa, nós e saque |
| `combate.h/.cpp` | Inimigos, dano e recompensas |
| `estruturas.h/.cpp` | Lista, pilha, fila e árvore |
| `algoritmos.h/.cpp` | Ordenação e buscas |

## Integrantes

- **Eduarda** — reino, estruturas e algoritmos
- **Mateus** — jogo/menus, expedição e combate
