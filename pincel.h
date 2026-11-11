#ifndef PINCEL_H
#define PINCEL_H

#include "raylib.h"

// O jogo é pensado numa tela de 320x180 "unidades", mas desenhado em 640x360:
// cada unidade vale 2 pixels de verdade. Assim dá pra usar meia unidade
// (0.5) nos detalhes finos sem mexer no resto das contas.

const int FATOR = 2;

// liga/desliga a ampliação (tudo entre os dois é desenhado em unidades)
void comecarZoom();
void terminarZoom();

// retângulo em unidades (aceita 0.5). Todo desenho de sprite passa por aqui.
void ret(float x, float y, float w, float h, Color cor);

// Contorno fino: o desenho é feito 4 vezes só com a cor do contorno, deslocado
// meia unidade pra cada lado, e na 5ª vez normal por cima.
//   for (int p = 0; p < PASSOS_CONTORNO; p++) { passoDoContorno(p, cor); desenhar(); }
const int PASSOS_CONTORNO = 5;
void passoDoContorno(int passo, Color cor);

// estica o que for desenhado em volta de um ponto (pra animar)
void comecarEscala(float pivoX, float pivoY, float escalaX, float escalaY);
void terminarEscala();

#endif
