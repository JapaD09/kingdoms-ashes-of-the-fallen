#ifndef INTERFACE_H
#define INTERFACE_H

#include "jogo.h"

// Janela do jogo (raylib). Aqui só tem desenho e leitura de mouse/teclado;
// as regras continuam em jogo.cpp e reino.cpp.

void abrirJanela();
bool janelaAberta();
void fecharJanela();

void atualizarInterface(Jogo& jogo);
void desenharInterface(const Jogo& jogo);

#endif
