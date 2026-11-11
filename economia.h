#ifndef ECONOMIA_H
#define ECONOMIA_H

// Números de balanceamento num lugar só. O simulador (simulador.cpp) troca
// esses valores pra comparar opções; os que estão aqui são os que tiveram a
// melhor nota na última varredura (864 combinações, 40 partidas pra cada tipo
// de jogador), conferida depois com 150 partidas.

struct Economia {
    int ouroInicial = 100;
    int impostoPorDez = 5;      // ouro por turno a cada 10 habitantes
    int custoPct = 100;          // preço dos prédios, em % do preço de tabela
    int ouroPorVitoria = 15;    // ouro fixo ao vencer a defesa
    int ondaPorTurno = 3;       // quanto a onda inimiga cresce por turno
    int ondaDivisor = 10;       // crescimento extra: turno² / divisor
    int materiaisPct = 150;     // madeira, pedra e ferro do começo, em %
    int producaoPct = 130;      // produção dos prédios, em %
    int forcaPorChefe = 25;     // % de vida a mais nos inimigos da expedição a cada chefe derrotado
    int forcaBase = 4;          // "chefes de vantagem" que os inimigos da expedição já têm no começo
    int rendaMadeira = 8;       // madeira que o castelo rende por turno (pedra = metade, ferro = um quarto)
    int saquePct = 25;         // ouro achado na expedição, em %
    int vendaPct = 20;         // por quanto os itens sem uso são vendidos, em % do preço do catálogo
    int ouroPorInimigo = 1;
    int nivelPct = 150;           // quanto subir de nível encarece: +N% do preço a cada nível que o prédio já tem     // ouro por inimigo ao vencer a defesa
};

Economia& economia();

// recalcula os preços dos prédios depois de mexer na economia
void aplicarEconomia();

#endif
