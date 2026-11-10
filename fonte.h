#ifndef FONTE_H
#define FONTE_H

#include "raylib.h"

// Fonte em pixel art com acentos. escala = tamanho do pixel.

int larguraTexto(const char* texto, int escala);

void desenharTexto(const char* texto, int x, int y, int escala, Color cor);
void desenharTextoSombra(const char* texto, int x, int y, int escala, Color cor);

// usada no logo
void desenharTextoContorno(const char* texto, int x, int y, int escala,
                           Color cima, Color baixo, Color contorno);

#endif
