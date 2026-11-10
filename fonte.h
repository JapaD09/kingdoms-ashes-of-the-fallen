#ifndef FONTE_H
#define FONTE_H

#include "raylib.h"

// Fonte própria em pixel art (letras de 5x7), com os acentos do português.
// A fonte padrão da raylib não mostra "ç", "ã" etc. do jeito que a gente queria,
// então cada letra é desenhada com retângulos.
// "escala" = tamanho de cada pixel da letra.

int larguraTexto(const char* texto, int escala);

void desenharTexto(const char* texto, int x, int y, int escala, Color cor);
void desenharTextoSombra(const char* texto, int x, int y, int escala, Color cor);

// Letra com duas cores (metade de cima e de baixo) e contorno em volta, pro logo
void desenharTextoContorno(const char* texto, int x, int y, int escala,
                           Color cima, Color baixo, Color contorno);

#endif
