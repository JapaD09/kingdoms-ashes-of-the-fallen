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

// libera a memória da partida
void encerrarJogo(Jogo& jogo);

// devolve o resumo do turno
std::string encerrarTurno(Jogo& jogo);

#endif
