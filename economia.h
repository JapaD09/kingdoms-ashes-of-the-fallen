#ifndef ECONOMIA_H
#define ECONOMIA_H

// Números de balanceamento num lugar só. O simulador (simulador.cpp) troca
// esses valores pra comparar opções; os que estão aqui são os que tiveram a
// melhor nota entre 486 combinações (60 partidas pra cada tipo de jogador).

struct Economia {
    int ouroInicial = 100;
    int impostoPorDez = 5;      // ouro por turno a cada 10 habitantes
    int custoPct = 70;          // preço dos prédios, em % do preço de tabela
    int ouroPorVitoria = 15;    // ouro fixo ao vencer a defesa
    int ondaPorTurno = 3;       // quanto a onda inimiga cresce por turno
    int ondaDivisor = 10;       // crescimento extra: turno² / divisor
    int materiaisPct = 150;     // madeira, pedra e ferro do começo, em %
    int producaoPct = 130;      // produção dos prédios, em %
    int forcaPorChefe = 45;     // % de vida a mais nos inimigos da expedição a cada chefe derrotado
};

Economia& economia();

// recalcula os preços dos prédios depois de mexer na economia
void aplicarEconomia();

#endif
