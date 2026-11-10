#ifndef FIGURAS_H
#define FIGURAS_H

#include "raylib.h"

// Bonequinhos das batalhas: tropas do reino, heróis e inimigos.
// Todo mundo usa o mesmo corpo; muda cor, arma, chapéu e tamanho.

enum Arma { SEM_ARMA, ESPADA, LANCA, ARCO, CAJADO, MACHADO, MARTELO, CLAVA, ADAGA };

enum Chapeu { CABELO, ELMO, CAPUZ, PONTUDO, CHIFRES, COROA, ELMO_ASAS, TURBANTE, GALHADA, CAPACETE_BRONZE };

enum Especial { NORMAL, MONTADO, DRAGAO };

struct Aparencia {
    Color pele;
    Color roupa;
    Color roupaEscura;
    Color detalhe;        // metal da arma, cabelo, chapéu
    int arma = SEM_ARMA;
    int chapeu = CABELO;
    bool escudo = false;
    bool capa = false;
    Color corCapa = {0, 0, 0, 0};
    bool baixinho = false;
    int tamanho = 1;      // tamanho do pixel (2 = grandão)
    int especial = NORMAL;
    Color brilhoArma = {0, 0, 0, 0};   // ponta do cajado
};

Aparencia aparenciaTropa(int tropa);
Aparencia aparenciaHeroi(int heroi);
Aparencia aparenciaInimigo(int reino, int nivel);

// cor da bandeira de cada reino inimigo
Color corDoReino(int reino);

// x = meio do boneco, y = onde o pé encosta no chão
// passo: 0 ou 1 (pernas), brilho: 0..1 (fica branco quando apanha)
void desenharFigura(int x, int y, const Aparencia& a, bool olhaEsquerda, int passo, float brilho);

// caído no chão
void desenharFiguraCaida(int x, int y, const Aparencia& a, bool olhaEsquerda);

// bandeirinha do reino (7x9)
void desenharBandeira(int x, int y, int reino, int s);

#endif
