#ifndef NATUREZA_H
#define NATUREZA_H

#include "raylib.h"

// Cenário vivo: árvores variadas, flores, pedras, riacho, galinhas e fumaça.

enum TipoArvore { PINHEIRO, CARVALHO, BETULA, ARBUSTO, SECA, TOTAL_TIPOS_ARVORE };

enum TipoEnfeite { FLOR = 10, PEDRINHA, COGUMELO, TUFO, TOCO, MANCHA };

const int MAX_OBJETOS = 320;

struct ObjetoCenario {
    int x, y;          // pé do objeto (base)
    int tipo;          // árvore (0-4) ou enfeite (10+)
    int tamanho;       // 0 pequeno, 1 médio, 2 grande
    int tom;           // variação de cor
    float fase;        // cada árvore balança num tempo diferente
};

struct Cenario {
    ObjetoCenario objetos[MAX_OBJETOS];
    int total = 0;
    float riachoX = -1;    // < 0 = sem riacho
    int largura = 0;
    int altura = 0;
};

// gera tudo uma vez, sem colocar nada dentro da área proibida
void gerarCenario(Cenario& c, Rectangle proibido, int largura, int altura,
                  float riachoX, unsigned semente);
void desenharCenario(const Cenario& c, float tempo);

// balanco: entre -1 e 1, quanto o vento empurra a copa
void desenharArvoreVariada(int x, int y, int tipo, int tamanho, int tom, float balanco);
float ventoNaArvore(float tempo, float fase);

// fumaça das chaminés
void soltarFumaca(float x, float y);
void atualizarFumaca(float dt);
void desenharFumaca();

// galinhas andando dentro de uma área
void prepararGalinhas(Rectangle area, int quantidade, bool (*podeAndar)(float x, float y));
void atualizarGalinhas(float dt);
void desenharGalinhas(float tempo);

#endif
