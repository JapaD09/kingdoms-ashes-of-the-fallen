#ifndef INTERFACE_H
#define INTERFACE_H

#include "jogo.h"

// Janela do jogo (raylib)

void abrirJanela();
bool janelaAberta();
void fecharJanela();

void atualizarInterface(Jogo& jogo);
void desenharInterface(const Jogo& jogo);

#endif
