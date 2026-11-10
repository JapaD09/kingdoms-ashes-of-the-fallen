#ifndef UI_H
#define UI_H

#include "raylib.h"

// Peças de tela usadas em todas as telas (caixas, botões, mouse).
// Tudo é desenhado em 320x180 e depois ampliado pra janela.

const int VL = 320;
const int VA = 180;

float tempo();

void caixa(Rectangle r, Color borda, Color fundo);
void caixaComSombra(Rectangle r, Color fundo);

// onde a tela do jogo fica dentro da janela
Rectangle areaNaJanela();

// mouse em coordenadas da tela do jogo (320x180)
Vector2 mouseVirtual();
bool mouseSobre(Rectangle r);
bool clicouEm(Rectangle r);

void textoCentralizado(const char* texto, int y, int escala, Color cor);
void textoCentralizadoEm(const char* texto, int centroX, int y, Color cor);

// botão pequeno de texto (ativo = destacado)
void botaozinho(Rectangle r, const char* texto, bool ativo);

// botão grande vermelho
void botaoPrincipal(Rectangle r, const char* texto);

// faixa vermelha com pontas (títulos)
void desenharFaixa(int x, int y, int largura, int altura);

// barra de vida
void barraDeVida(int x, int y, int largura, int vida, int maximo, Color cor);

#endif
