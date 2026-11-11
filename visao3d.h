#ifndef VISAO3D_H
#define VISAO3D_H

#include "raylib.h"
#include "natureza.h"
#include "reino.h"
#include "sprites.h"

// Vista "meio 3D": o mapa 2D vira o chão de uma cena 3D, e as árvores,
// prédios e galinhas viram figuras em pé (billboards) que olham pra câmera.
// Medidas do mapa continuam em pixels do 2D; 16 pixels = 1 unidade no 3D.

struct InfoMapa {
    int largura, altura;           // tamanho do mapa em pixels (a tela do jogo)
    int gradeX, gradeY, lote, grade;
    Desenho predios[TOTAL_TIPOS_EDIFICIO];
};

void iniciarVisao3D(const InfoMapa& info);
void liberarVisao3D();

// o chão é desenhado em 2D nessa textura antes da cena
RenderTexture2D& texturaDoChao();

void atualizarCamera3D(float dt);
void resetarCamera3D();

// mouse (na tela do jogo) -> ponto do mapa 2D. false se não bateu no chão
bool mouseNoMapa3D(Vector2 mouse, Vector2& ponto);

struct Previa3D {
    int linha = -1, coluna = -1, tipo = -1;   // prédio fantasma embaixo do mouse
    float crescimento[5][5];   // 1 = tamanho normal (anima prédio novo)
};

void desenharCena3D(const Cenario& cenario, const Reino& reino, const Previa3D& previa, float tempo);

#endif
