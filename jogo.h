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

// Le um numero entre minimo e maximo; insiste ate vir algo valido
int lerOpcao(int minimo, int maximo);

void pausar();

void iniciarJogo(Jogo& jogo);
void loopPrincipal(Jogo& jogo);

#endif
