#ifndef MODELOS3D_H
#define MODELOS3D_H

#include "raylib.h"

// Modelos 3D do mapa (vista 3D): construções e árvores montadas com blocos,
// telhados e cones. As medidas são nas mesmas unidades do mapa 2D
// (um lote tem 22 de lado) e o centro do modelo fica no meio do lote.

const float UNIDADES_POR_METRO = 16.0f;   // unidades do mapa por unidade do mundo 3D

// base = ponto do chão no meio do lote. cresce: 1 = tamanho normal (animação de nascer)
// alfa: 1 = sólido, menos = fantasma (fila e pré-visualização)
void desenharPredio3D(int tipo, Vector3 base, float cresce, float alfa, float tempo);
void desenharCastelo3D(Vector3 base, float tempo);

// vento: -1 a 1, inclina a árvore
void desenharArvore3D(int tipo, int tamanho, int tom, Vector3 base, float vento);

#endif
