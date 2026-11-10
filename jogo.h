#ifndef JOGO_H
#define JOGO_H

#include <string>
#include "reino.h"

struct Jogo {
    std::string nomeReino = "Ashenfall";
    int turno = 1;
    bool rodando = true;
    Reino reino;
};

void iniciarJogo(Jogo& jogo);

// Libera tudo que foi alocado com new durante a partida
void encerrarJogo(Jogo& jogo);

// Avança o reino um turno e devolve um resumo pra mostrar na tela
std::string encerrarTurno(Jogo& jogo);

#endif
