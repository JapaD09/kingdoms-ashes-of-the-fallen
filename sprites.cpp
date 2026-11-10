#include "sprites.h"
#include "paleta.h"
#include <cmath>

// desenha um retângulo dentro do sprite
#define PX(a, b, w, h, cor) DrawRectangle(x + (a) * s, y + (b) * s, (w) * s, (h) * s, cor)

static int piscada(float velocidade) {
    return static_cast<int>(GetTime() * velocidade) % 2;
}

void desenharCastelo(int x, int y, int s) {
    PX(3, 10, 16, 11, PEDRA);
    for (int i = 3; i < 19; i += 3) PX(i, 9, 2, 1, PEDRA);
    PX(1, 6, 5, 15, PEDRA_CLARA);
    PX(16, 6, 5, 15, PEDRA_CLARA);
    PX(5, 6, 1, 15, PEDRA);
    PX(20, 6, 1, 15, PEDRA);
    PX(1, 4, 5, 2, TELHADO_AZUL);
    PX(2, 2, 3, 2, TELHADO_AZUL);
    PX(16, 4, 5, 2, TELHADO_AZUL);
    PX(17, 2, 3, 2, TELHADO_AZUL);
    PX(8, 4, 6, 10, PEDRA_CLARA);
    PX(8, 2, 6, 2, TELHADO_VERM);
    PX(9, 0, 4, 2, TELHADO_VERM);
    PX(11, -4, 1, 4, CONTORNO);              // mastro
    PX(12, -4, 3, 2, TELHADO_VERM);          // bandeira
    PX(15, -4 + piscada(3.0f), 1, 1, TELHADO_VERM);
    PX(10, 16, 2, 5, CONTORNO);              // portão
    PX(10, 7, 2, 2, JANELA);
    PX(3, 11, 1, 2, JANELA);
    PX(18, 11, 1, 2, JANELA);
}

void desenharFazenda(int x, int y, int s) {
    PX(2, 4, 18, 15, TERRA);
    for (int i = 6; i < 19; i += 4) {
        PX(3, i, 16, 1, GRAMA_CLARA);
        PX(4, i - 1, 1, 1, DOURADO);
        PX(10, i - 1, 1, 1, DOURADO);
        PX(16, i - 1, 1, 1, DOURADO);
    }
    PX(2, 19, 18, 1, TERRA_ESCURA);
}

void desenharSerraria(int x, int y, int s) {
    PX(3, 9, 12, 9, MADEIRA);
    PX(2, 6, 14, 3, TELHADO_VERM);
    PX(4, 4, 10, 2, TELHADO_VERM);
    PX(8, 13, 3, 5, MADEIRA_ESCURA);
    PX(14, 15, 7, 3, MADEIRA);               // toras
    PX(15, 12, 5, 3, MADEIRA);
    PX(20, 15, 1, 3, PERGAMINHO);
    PX(19, 12, 1, 3, PERGAMINHO);
}

void desenharQuartel(int x, int y, int s) {
    PX(2, 9, 16, 10, PERGAMINHO);
    PX(1, 6, 18, 3, TELHADO_VERM);
    PX(3, 4, 14, 2, TELHADO_VERM);
    PX(8, 13, 4, 6, MADEIRA_ESCURA);
    PX(19, 1, 1, 18, CONTORNO);              // mastro
    PX(20, 1, 2, 3, TELHADO_AZUL);
    PX(4, 11, 2, 2, JANELA);
    PX(14, 11, 2, 2, JANELA);
}

void desenharMuralha(int x, int y, int s) {
    PX(1, 8, 20, 11, PEDRA);
    for (int i = 1; i < 21; i += 4) PX(i, 6, 2, 2, PEDRA);
    for (int i = 10; i < 19; i += 3) PX(1, i, 20, 1, PEDRA_CLARA);
    PX(6, 11, 1, 2, CONTORNO);
    PX(14, 14, 1, 2, CONTORNO);
    PX(1, 19, 20, 1, CONTORNO);
}

void desenharOficina(int x, int y, int s) {
    int fumaca = static_cast<int>(fmod(GetTime() * 3.0, 4.0));
    PX(3, 9, 13, 10, PEDRA);
    PX(2, 6, 15, 3, MADEIRA_ESCURA);
    PX(12, 2, 2, 5, PEDRA_CLARA);            // chaminé
    PX(12, 1 - fumaca, 2, 1, Fade(TEXTO_CLARO, 0.6f));
    PX(7, 13, 4, 6, CONTORNO);
    PX(16, 15, 5, 2, CONTORNO);              // bigorna
    PX(17, 17, 3, 2, CONTORNO);
}

void desenharArvore(int x, int y, int s) {
    PX(1, 9, 6, 1, Fade(CONTORNO, 0.35f));   // sombra
    PX(3, 6, 1, 3, MADEIRA_ESCURA);
    PX(1, 2, 5, 4, FOLHA);
    PX(0, 3, 7, 2, FOLHA);
    PX(2, 0, 3, 2, FOLHA);
    PX(2, 1, 2, 2, FOLHA_CLARA);
}

// ícones

void iconeOuro(int x, int y, int s) {
    PX(1, 0, 5, 7, DOURADO_ESCURO);
    PX(0, 1, 7, 5, DOURADO_ESCURO);
    PX(1, 1, 5, 5, DOURADO);
    PX(2, 2, 2, 2, BRILHO_OURO);
}

void iconeMadeira(int x, int y, int s) {
    PX(0, 2, 6, 3, MADEIRA);
    PX(0, 4, 6, 1, MADEIRA_ESCURA);
    PX(6, 2, 1, 3, PERGAMINHO);
}

void iconePedra(int x, int y, int s) {
    PX(1, 1, 5, 5, PEDRA);
    PX(0, 3, 7, 3, PEDRA);
    PX(2, 2, 2, 1, PEDRA_CLARA);
}

void iconeComida(int x, int y, int s) {
    PX(1, 2, 5, 5, TELHADO_VERM);
    PX(0, 3, 7, 3, TELHADO_VERM);
    PX(2, 3, 1, 1, JANELA);
    PX(3, 0, 2, 2, GRAMA_CLARA);
}

void iconeFerro(int x, int y, int s) {
    PX(0, 3, 7, 3, PEDRA);
    PX(1, 2, 5, 1, PEDRA_CLARA);
    PX(1, 3, 5, 1, PEDRA_CLARA);
}

void iconePovo(int x, int y, int s) {
    PX(2, 0, 3, 3, PERGAMINHO);
    PX(1, 3, 5, 4, TELHADO_AZUL);
}

void iconeSoldado(int x, int y, int s) {
    PX(3, 0, 1, 5, PEDRA_CLARA);
    PX(1, 4, 5, 1, DOURADO);
    PX(3, 5, 1, 2, MADEIRA_ESCURA);
}

void iconeCoroa(int x, int y, int s) {
    PX(0, 3, 7, 3, DOURADO);
    PX(0, 1, 1, 2, DOURADO);
    PX(3, 0, 1, 3, DOURADO);
    PX(6, 1, 1, 2, DOURADO);
    PX(3, 4, 1, 1, TELHADO_VERM);
    PX(0, 6, 7, 1, DOURADO_ESCURO);
}

void iconeMapa(int x, int y, int s) {
    PX(0, 1, 7, 5, PERGAMINHO);
    PX(2, 1, 1, 5, PERGAMINHO_ESC);
    PX(4, 1, 1, 5, PERGAMINHO_ESC);
    PX(1, 4, 1, 1, MADEIRA_ESCURA);
    PX(3, 3, 1, 1, MADEIRA_ESCURA);
    PX(5, 2, 1, 1, TELHADO_VERM);
}

void iconeLivro(int x, int y, int s) {
    PX(1, 0, 6, 7, TELHADO_VERM);
    PX(1, 0, 1, 7, VERMELHO_ESC);
    PX(3, 2, 3, 1, DOURADO);
    PX(2, 6, 5, 1, PERGAMINHO);
}

void iconeBau(int x, int y, int s) {
    PX(0, 1, 7, 2, MADEIRA_ESCURA);
    PX(0, 3, 7, 4, MADEIRA);
    PX(0, 3, 7, 1, DOURADO_ESCURO);
    PX(3, 3, 1, 2, DOURADO);
}

void iconePorta(int x, int y, int s) {
    PX(2, 0, 3, 1, MADEIRA);
    PX(1, 1, 5, 6, MADEIRA);
    PX(3, 1, 1, 6, MADEIRA_ESCURA);
    PX(4, 4, 1, 1, DOURADO);
}

void iconeAmpulheta(int x, int y, int s) {
    PX(0, 0, 7, 1, MADEIRA_ESCURA);
    PX(0, 6, 7, 1, MADEIRA_ESCURA);
    PX(1, 1, 5, 1, PERGAMINHO);
    PX(2, 2, 3, 1, PERGAMINHO);
    PX(3, 3, 1, 1, DOURADO);
    PX(2, 4, 3, 1, DOURADO);
    PX(1, 5, 5, 1, DOURADO);
}
