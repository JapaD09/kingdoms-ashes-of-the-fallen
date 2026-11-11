#include "sprites.h"
#include "paleta.h"
#include "pincel.h"
#include <cmath>

// desenha um retângulo dentro do sprite
static const Color ROXO_CLARO = {140, 104, 196, 255};
static const Color AZUL_CLARO = {110, 130, 210, 255};

#define PX(a, b, w, h, ...) ret(x + (a) * s, y + (b) * s, (w) * s, (h) * s, __VA_ARGS__)

// Detalhes finos (usam meia unidade). Todos recebem a área em unidades do sprite.

static const Color RISCO = {38, 26, 22, 60};
static const Color RISCO_FORTE = {38, 26, 22, 110};
static const Color LUZ = {255, 250, 235, 70};

// fileiras de telha com as emendas desencontradas
static void telhas(int x, int y, int s, float a, float b, float w, float h) {
    PX(a, b, w, 0.5f, LUZ);
    for (float l = 1; l <= h; l += 1) {
        PX(a, b + l - 0.5f, w, 0.5f, RISCO);
        float comeco = (static_cast<int>(l) % 2) ? 1.0f : 2.0f;
        for (float c = comeco; c < w; c += 2) PX(a + c, b + l - 1, 0.5f, 0.5f, RISCO);
    }
}

static void tijolos(int x, int y, int s, float a, float b, float w, float h) {
    int fileira = 0;
    for (float l = 2; l <= h; l += 2, fileira++) {
        PX(a, b + l - 0.5f, w, 0.5f, RISCO);
        for (float c = (fileira % 2) ? 1.5f : 3.0f; c < w; c += 3) PX(a + c, b + l - 2, 0.5f, 1.5f, RISCO);
    }
    PX(a, b, 0.5f, h, LUZ);
    PX(a + w - 1, b, 1, h, RISCO);          // lado da sombra
}

static void tabuas(int x, int y, int s, float a, float b, float w, float h) {
    for (float c = 2; c < w; c += 2) PX(a + c - 0.5f, b, 0.5f, h, RISCO);
    PX(a, b, w, 0.5f, LUZ);
    PX(a + w - 1, b, 1, h, RISCO);
}

static void janela(int x, int y, int s, float a, float b, float w, float h) {
    PX(a - 0.5f, b - 0.5f, w + 1, 0.5f, RISCO_FORTE);
    PX(a - 0.5f, b + h, w + 1, 0.5f, RISCO_FORTE);
    PX(a - 0.5f, b, 0.5f, h, RISCO_FORTE);
    PX(a + w, b, 0.5f, h, RISCO_FORTE);
    if (w >= 2) PX(a + w / 2 - 0.25f, b, 0.5f, h, RISCO_FORTE);
    PX(a, b + h / 2 - 0.25f, w, 0.5f, RISCO_FORTE);
    PX(a, b, 0.5f, 0.5f, Color{255, 255, 240, 200});
}

static void porta(int x, int y, int s, float a, float b, float w, float h) {
    PX(a, b, w, 0.5f, RISCO_FORTE);
    for (float c = 1; c < w; c += 1) PX(a + c - 0.25f, b + 0.5f, 0.5f, h - 0.5f, RISCO);
    PX(a + w - 1, b + h / 2, 0.5f, 0.5f, DOURADO);     // maçaneta
}

#define TELHAS(a, b, w, h) telhas(x, y, s, a, b, w, h)
#define TIJOLOS(a, b, w, h) tijolos(x, y, s, a, b, w, h)
#define TABUAS(a, b, w, h) tabuas(x, y, s, a, b, w, h)
#define JANELA_FINA(a, b, w, h) janela(x, y, s, a, b, w, h)
#define PORTA_FINA(a, b, w, h) porta(x, y, s, a, b, w, h)

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
    TIJOLOS(3, 10, 16, 11);
    TIJOLOS(1, 6, 5, 15);
    TIJOLOS(16, 6, 5, 15);
    TIJOLOS(8, 4, 6, 6);
    TELHAS(1, 4, 5, 2);
    TELHAS(16, 4, 5, 2);
    TELHAS(8, 2, 6, 2);
    JANELA_FINA(10, 7, 2, 2);
    JANELA_FINA(3, 11, 1, 2);
    JANELA_FINA(18, 11, 1, 2);
    PX(9.5f, 15.5f, 3, 0.5f, PEDRA_CLARA);   // arco do portão
    PX(9.5f, 16, 0.5f, 5, PEDRA_CLARA);
    PX(12, 16, 0.5f, 5, PEDRA_CLARA);
    PX(10.75f, 16, 0.5f, 5, Color{90, 84, 92, 255});      // grade
    PX(10, 18, 2, 0.5f, Color{90, 84, 92, 255});
    PX(12, -3.5f, 3, 0.5f, Color{226, 108, 88, 255});     // dobra da bandeira
    PX(1, 20.5f, 20, 0.5f, RISCO_FORTE);
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
    for (int i = 6; i < 19; i += 4) {
        PX(3, i + 1, 16, 0.5f, RISCO);                    // sulco
        for (float c = 3.5f; c < 19; c += 1.5f) PX(c, i - 0.5f, 0.5f, 0.5f, FOLHA_CLARA);
        PX(4.25f, i - 1.5f, 0.5f, 0.5f, BRILHO_OURO);
        PX(10.25f, i - 1.5f, 0.5f, 0.5f, BRILHO_OURO);
        PX(16.25f, i - 1.5f, 0.5f, 0.5f, BRILHO_OURO);
    }
    PX(2, 4, 18, 0.5f, MADEIRA_ESCURA);      // cerca
    for (float c = 2; c <= 19.5f; c += 2.5f) PX(c, 3.5f, 0.5f, 1.5f, MADEIRA);
    PX(2, 4, 0.5f, 15, RISCO);
    PX(19.5f, 4, 0.5f, 15, RISCO);
}

void desenharSerraria(int x, int y, int s) {
    PX(11, 1, 2, 4, PEDRA);                  // chaminé
    PX(3, 9, 12, 9, MADEIRA);
    PX(2, 6, 14, 3, TELHADO_VERM);
    PX(4, 4, 10, 2, TELHADO_VERM);
    PX(8, 13, 3, 5, MADEIRA_ESCURA);
    PX(14, 15, 7, 3, MADEIRA);               // toras
    PX(15, 12, 5, 3, MADEIRA);
    PX(20, 15, 1, 3, PERGAMINHO);
    PX(19, 12, 1, 3, PERGAMINHO);
    TABUAS(3, 9, 12, 9);
    TELHAS(2, 6, 14, 3);
    TELHAS(4, 4, 10, 2);
    PORTA_FINA(8, 13, 3, 5);
    PX(4.5f, 10.5f, 2, 2, JANELA);
    JANELA_FINA(4.5f, 10.5f, 2, 2);
    PX(11, 1, 2, 0.5f, PEDRA_CLARA);
    PX(14, 15, 6, 0.5f, LUZ);
    PX(15, 12, 4, 0.5f, LUZ);
    PX(20.25f, 15.75f, 0.5f, 1.5f, MADEIRA);              // anéis da tora
    PX(19.25f, 12.75f, 0.5f, 1.5f, MADEIRA);
    PX(14, 16.5f, 6, 0.5f, RISCO);
    PX(2, 18, 13, 0.5f, RISCO_FORTE);
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
    TIJOLOS(2, 9, 16, 10);
    TELHAS(1, 6, 18, 3);
    TELHAS(3, 4, 14, 2);
    PORTA_FINA(8, 13, 4, 6);
    JANELA_FINA(4, 11, 2, 2);
    JANELA_FINA(14, 11, 2, 2);
    PX(20, 1, 2, 0.5f, AZUL_CLARO);
    PX(9.5f, 10, 1, 2, PEDRA_CLARA);         // espadinha em cima da porta
    PX(9, 11.5f, 2, 0.5f, DOURADO);
}

void desenharMuralha(int x, int y, int s) {
    PX(1, 8, 20, 11, PEDRA);
    for (int i = 1; i < 21; i += 4) PX(i, 6, 2, 2, PEDRA);
    for (int i = 10; i < 19; i += 3) PX(1, i, 20, 1, PEDRA_CLARA);
    PX(6, 11, 1, 2, CONTORNO);
    PX(14, 14, 1, 2, CONTORNO);
    PX(1, 19, 20, 1, CONTORNO);
    TIJOLOS(1, 8, 20, 11);
    for (int i = 1; i < 21; i += 4) {
        PX(i, 6, 2, 0.5f, PEDRA_CLARA);
        PX(i + 1.5f, 6, 0.5f, 2, RISCO);
    }
    PX(6, 10.5f, 1, 0.5f, PEDRA_CLARA);
    PX(14, 13.5f, 1, 0.5f, PEDRA_CLARA);
    PX(2, 18, 1, 1, FOLHA);                  // musgo
    PX(17, 17.5f, 1.5f, 1.5f, FOLHA);
    PX(9, 18.5f, 0.5f, 0.5f, FOLHA_CLARA);
}

void desenharOficina(int x, int y, int s) {
    PX(3, 9, 13, 10, PEDRA);
    PX(2, 6, 15, 3, MADEIRA_ESCURA);
    PX(12, 2, 2, 5, PEDRA_CLARA);            // chaminé
    PX(7, 13, 4, 6, CONTORNO);
    PX(16, 15, 5, 2, CONTORNO);              // bigorna
    PX(17, 17, 3, 2, CONTORNO);
    TIJOLOS(3, 9, 13, 10);
    TELHAS(2, 6, 15, 3);
    PX(12, 2, 2, 0.5f, TEXTO_CLARO);
    PX(7.5f, 15.5f, 3, 3.5f, Color{160, 60, 30, 255});    // fogo da forja
    PX(8, 16.5f, 2, 2.5f, BRASA);
    PX(8.5f, 17.5f + piscada(6.0f) * 0.5f, 1, 1, JANELA);
    PX(16, 15, 5, 0.5f, PEDRA);              // brilho da bigorna
    PX(17, 17, 3, 0.5f, PEDRA);
    PX(4.5f, 11, 1.5f, 1.5f, JANELA);
    JANELA_FINA(4.5f, 11, 1.5f, 1.5f);
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

// seta curva pra trás
void iconeDesfazer(int x, int y, int s) {
    PX(2, 0, 1, 3, PERGAMINHO);
    PX(1, 1, 1, 1, PERGAMINHO);
    PX(3, 1, 3, 1, PERGAMINHO);
    PX(6, 2, 1, 3, PERGAMINHO);
    PX(2, 5, 4, 1, PERGAMINHO);
}

// olho, pra trocar a vista
void iconeVista(int x, int y, int s) {
    PX(1, 2, 5, 3, PERGAMINHO);
    PX(0, 3, 7, 1, PERGAMINHO);
    PX(3, 2, 1, 3, TELHADO_AZUL);
    PX(2, 3, 3, 1, TELHADO_AZUL);
    PX(3, 3, 1, 1, CONTORNO);
}

// prédios novos (cartas)

void desenharMina(int x, int y, int s) {
    PX(2, 9, 18, 11, PEDRA);
    PX(4, 6, 14, 3, PEDRA);
    PX(7, 4, 8, 2, PEDRA_CLARA);
    PX(4, 7, 4, 2, PEDRA_CLARA);
    PX(15, 10, 3, 2, PEDRA_CLARA);
    PX(8, 11, 6, 8, CONTORNO);               // entrada
    PX(7, 10, 1, 9, MADEIRA);
    PX(14, 10, 1, 9, MADEIRA);
    PX(7, 10, 8, 1, MADEIRA);
    PX(4, 12, 1, 1, DOURADO);                // veio de minério
    PX(17, 14, 1, 1, PEDRA_CLARA);
    PX(15, 17, 5, 2, MADEIRA_ESCURA);        // carrinho
    PX(16, 16, 3, 1, PEDRA_CLARA);
    PX(15, 19, 1, 1, CONTORNO);
    PX(19, 19, 1, 1, CONTORNO);
    PX(2, 20, 18, 1, TERRA_ESCURA);
    PX(5, 6, 6, 0.5f, LUZ);
    PX(3, 13, 2, 0.5f, RISCO_FORTE);         // rachaduras
    PX(4.5f, 13.5f, 0.5f, 2, RISCO_FORTE);
    PX(16, 12.5f, 2.5f, 0.5f, RISCO_FORTE);
    PX(17, 16, 0.5f, 2, RISCO);
    PX(18, 9, 2, 11, RISCO);
    PX(7, 10, 8, 0.5f, PERGAMINHO_ESC);      // viga
    PX(7.25f, 10.25f, 0.5f, 0.5f, CONTORNO);
    PX(14.25f, 10.25f, 0.5f, 0.5f, CONTORNO);
    PX(8, 19, 12, 0.5f, PEDRA_CLARA);        // trilho
    PX(6, 12, 1, 1.5f, JANELA);              // lanterna
    PX(6, 11.5f, 1, 0.5f, CONTORNO);
    PX(16.5f, 15.5f, 0.5f, 0.5f, TEXTO_CLARO);
    PX(15, 17, 5, 0.5f, MADEIRA);
}

void desenharMercado(int x, int y, int s) {
    PX(3, 8, 1, 11, MADEIRA_ESCURA);
    PX(18, 8, 1, 11, MADEIRA_ESCURA);
    for (int i = 0; i < 18; i += 4) {        // toldo listrado
        PX(2 + i, 5, 2, 3, TELHADO_VERM);
        PX(4 + i, 5, 2, 3, PERGAMINHO);
    }
    for (int i = 0; i < 18; i += 2) PX(2 + i, 8, 1, 1, TELHADO_VERM);
    PX(3, 14, 16, 5, MADEIRA);
    PX(3, 13, 16, 1, MADEIRA_ESCURA);
    PX(5, 12, 2, 1, TELHADO_VERM);           // maçãs
    PX(9, 12, 3, 1, DOURADO_ESCURO);         // pão
    PX(14, 12, 3, 1, TELHADO_AZUL);          // tecido
    PX(10, 2, 2, 2, DOURADO);                // placa de moeda
    PX(10, 4, 2, 1, MADEIRA_ESCURA);
    PX(15, 17, 3, 2, PERGAMINHO_ESC);
    for (int i = 0; i < 18; i += 2) PX(2 + i, 7.5f, 2, 0.5f, RISCO);
    PX(2, 5, 18, 0.5f, LUZ);
    TABUAS(3, 14, 16, 5);
    PX(5, 11.5f, 0.5f, 0.5f, CORACAO_CLARO);
    PX(6, 11.5f, 0.5f, 0.5f, FOLHA_CLARA);
    PX(9, 12, 3, 0.5f, DOURADO);
    PX(14, 12.5f, 3, 0.5f, AZUL_CLARO);
    PX(10.5f, 2.5f, 0.5f, 0.5f, BRILHO_OURO);
    PX(3.5f, 8, 0.5f, 6, RISCO);
    PX(18.5f, 8, 0.5f, 6, RISCO);
    PX(7, 16, 4, 1.5f, PERGAMINHO);          // plaquinha de preço
    PX(7.5f, 16.5f, 3, 0.5f, MADEIRA_ESCURA);
}

void desenharCasa(int x, int y, int s) {
    PX(14, 2, 2, 4, PEDRA);                  // chaminé
    PX(5, 10, 12, 9, PERGAMINHO);
    PX(5, 10, 1, 9, MADEIRA_ESCURA);
    PX(16, 10, 1, 9, MADEIRA_ESCURA);
    PX(5, 14, 12, 1, MADEIRA_ESCURA);
    PX(3, 8, 16, 2, DOURADO_ESCURO);         // telhado de palha
    PX(5, 6, 12, 2, DOURADO_ESCURO);
    PX(7, 4, 8, 2, DOURADO_ESCURO);
    PX(5, 7, 12, 1, DOURADO);
    PX(9, 15, 3, 4, MADEIRA_ESCURA);
    PX(13, 11, 2, 2, JANELA);
    PX(1, 17, 3, 1, MADEIRA);                // cerquinha
    PX(1, 16, 1, 3, MADEIRA);
    PX(18, 18, 1, 1, FLOR_AMARELA);
    PX(19, 17, 1, 1, FLOR_BRANCA);
    for (float l = 0.5f; l < 6; l += 1) PX(4 + (5 - l) * 0.4f, 4 + l, 14 - (5 - l) * 0.8f, 0.5f, RISCO);
    PX(7, 4, 8, 0.5f, BRILHO_OURO);
    PX(10.5f, 10, 0.5f, 4, MADEIRA_ESCURA);  // enxaimel
    PX(6, 10, 4.5f, 0.5f, RISCO);
    PX(15, 10, 1, 9, RISCO);
    PORTA_FINA(9, 15, 3, 4);
    JANELA_FINA(13, 11, 2, 2);
    PX(13, 13.5f, 2, 0.5f, MADEIRA);         // floreira
    PX(13.25f, 13, 0.5f, 0.5f, FLOR_AMARELA);
    PX(14.25f, 13, 0.5f, 0.5f, CORACAO_CLARO);
    PX(14, 2, 2, 0.5f, PEDRA_CLARA);
    PX(2, 16.5f, 0.5f, 2.5f, MADEIRA);
    PX(5, 19, 12, 0.5f, RISCO_FORTE);
}

void desenharTorre(int x, int y, int s) {
    PX(10, -3, 1, 7, CONTORNO);              // mastro
    PX(11, -3, 3, 2, TELHADO_VERM);
    PX(14, -3 + piscada(3.0f), 1, 1, TELHADO_VERM);
    PX(6, 4, 10, 2, PEDRA);
    for (int i = 6; i < 16; i += 4) PX(i, 3, 2, 1, PEDRA);
    PX(7, 6, 8, 14, PEDRA);
    PX(7, 6, 2, 14, PEDRA_CLARA);
    PX(7, 11, 8, 1, PEDRA_CLARA);
    PX(10, 8, 2, 3, CONTORNO);               // seteiras
    PX(10, 13, 2, 2, CONTORNO);
    PX(9, 17, 4, 3, MADEIRA_ESCURA);
    PX(6, 19, 10, 1, CONTORNO);
    TIJOLOS(7, 6, 8, 14);
    PX(6, 4, 10, 0.5f, PEDRA_CLARA);
    PX(6, 5.5f, 10, 0.5f, RISCO_FORTE);
    for (int i = 6; i < 16; i += 4) PX(i, 3, 2, 0.5f, PEDRA_CLARA);
    PX(10, 7.5f, 2, 0.5f, PEDRA_CLARA);
    PX(10, 12.5f, 2, 0.5f, PEDRA_CLARA);
    PORTA_FINA(9, 17, 4, 3);
    PX(11, -3, 3, 0.5f, CORACAO_CLARO);
    PX(8, 15, 1, 1, FOLHA);
}

void desenharArquearia(int x, int y, int s) {
    PX(2, 10, 9, 8, MADEIRA);
    PX(1, 8, 11, 2, FOLHA);
    PX(3, 6, 7, 2, FOLHA);
    PX(1, 9, 11, 1, FOLHA_CLARA);
    PX(5, 14, 3, 4, MADEIRA_ESCURA);
    PX(3, 11, 1, 3, MADEIRA_ESCURA);         // arco pendurado
    PX(14, 14, 1, 5, MADEIRA_ESCURA);        // alvo
    PX(18, 14, 1, 5, MADEIRA_ESCURA);
    PX(14, 7, 5, 7, PERGAMINHO);
    PX(13, 8, 7, 5, PERGAMINHO);
    PX(14, 8, 5, 5, TELHADO_VERM);
    PX(15, 9, 3, 3, PERGAMINHO);
    PX(16, 10, 1, 1, TELHADO_VERM);
    PX(17, 9, 4, 1, MADEIRA);                // flecha cravada
    PX(20, 8, 1, 1, TEXTO_CLARO);
    PX(2, 18, 18, 1, TERRA_ESCURA);
    TABUAS(2, 10, 9, 8);
    TELHAS(1, 8, 11, 2);
    TELHAS(3, 6, 7, 2);
    PORTA_FINA(5, 14, 3, 4);
    PX(8.5f, 11, 1.5f, 1.5f, JANELA);
    JANELA_FINA(8.5f, 11, 1.5f, 1.5f);
    PX(14, 8, 5, 0.5f, VERMELHO_ESC);        // anéis do alvo
    PX(14, 12.5f, 5, 0.5f, VERMELHO_ESC);
    PX(15, 9, 3, 0.5f, PERGAMINHO_ESC);
    PX(15.75f, 9.75f, 1.5f, 1.5f, TELHADO_VERM);
    PX(16.25f, 10.25f, 0.5f, 0.5f, DOURADO);
    PX(17, 9.25f, 4, 0.5f, MADEIRA_ESCURA);
    PX(20.5f, 8.5f, 0.5f, 1.5f, TEXTO_CLARO);
}

void desenharEstabulo(int x, int y, int s) {
    PX(3, 9, 16, 10, TELHADO_VERM);
    PX(3, 17, 16, 2, VERMELHO_ESC);
    PX(2, 6, 18, 3, MADEIRA_ESCURA);
    PX(4, 4, 14, 2, MADEIRA_ESCURA);
    PX(7, 2, 8, 2, MADEIRA_ESCURA);
    PX(10, 6, 2, 2, DOURADO);                // janela do feno
    PX(8, 12, 6, 7, MADEIRA);
    for (int i = 0; i < 6; i++) {            // X da porta
        PX(8 + i, 12 + i, 1, 1, PERGAMINHO);
        PX(13 - i, 12 + i, 1, 1, PERGAMINHO);
    }
    PX(17, 16, 4, 3, DOURADO);               // fardo de feno
    PX(17, 17, 4, 1, DOURADO_ESCURO);
    PX(4, 12, 2, 2, CONTORNO);
    PX(4, 12, 1, 1, MADEIRA);                // cavalo espiando
    TABUAS(3, 9, 16, 10);
    TELHAS(2, 6, 18, 3);
    TELHAS(4, 4, 14, 2);
    TELHAS(7, 2, 8, 2);
    PX(8, 12, 6, 0.5f, PERGAMINHO);
    PX(8, 12, 0.5f, 7, PERGAMINHO);
    PX(13.5f, 12, 0.5f, 7, PERGAMINHO);
    PX(10.75f, 12.5f, 0.5f, 6.5f, RISCO_FORTE);
    JANELA_FINA(10, 6, 2, 2);
    PX(17, 16, 4, 0.5f, BRILHO_OURO);
    PX(17.5f, 16.5f, 0.5f, 2.5f, DOURADO_ESCURO);
    PX(19.5f, 16.5f, 0.5f, 2.5f, DOURADO_ESCURO);
    PX(4.5f, 12.5f, 0.5f, 0.5f, TEXTO_CLARO);  // olho do cavalo
    PX(16, 12, 2, 2, CONTORNO);
    JANELA_FINA(16, 12, 2, 2);
}

void desenharTorreArcana(int x, int y, int s) {
    int flutua = piscada(1.5f);
    PX(10, -5 - flutua, 2, 2, CRISTAL);      // cristal flutuando
    PX(10, -4 - flutua, 1, 1, TEXTO_CLARO);
    PX(10, -2, 2, 2, ROXO_ESC);
    PX(9, 0, 4, 2, ROXO_ESC);
    PX(8, 2, 6, 2, ROXO_ESC);
    PX(7, 4, 8, 2, ROXO_ESC);
    PX(8, 6, 6, 13, ROXO);
    PX(8, 6, 2, 13, ROXO_CLARO);
    PX(10, 9, 2, 3, CRISTAL);                // janela brilhando
    PX(9, 14, 1, 1, CRISTAL);
    PX(12, 15, 1, 1, CRISTAL);
    PX(7, 18, 8, 2, PEDRA);
    PX(10, 16, 2, 2, CONTORNO);
    TIJOLOS(8, 6, 6, 13);
    for (float l = 0; l < 8; l += 1) PX(7 + (7 - l) * 0.4f, -2 + l + 0.5f, 8 - (7 - l) * 0.8f, 0.5f, RISCO);
    PX(10, -2, 0.5f, 8, Color{140, 104, 196, 120});
    JANELA_FINA(10, 9, 2, 3);
    PX(9.25f, 14.25f, 0.5f, 0.5f, TEXTO_CLARO);
    PX(12.25f, 15.25f, 0.5f, 0.5f, TEXTO_CLARO);
    float gira = static_cast<float>(GetTime()) * 2;
    PX(11 + cosf(gira) * 3, -4 + sinf(gira) * 1.5f, 0.5f, 0.5f, CRISTAL);      // faíscas em volta do cristal
    PX(11 + cosf(gira + 3.14f) * 3, -4 + sinf(gira + 3.14f) * 1.5f, 0.5f, 0.5f, TEXTO_CLARO);
    PORTA_FINA(10, 16, 2, 2);
    PX(7, 18, 8, 0.5f, PEDRA_CLARA);
}

// arte das cartas que não são prédio

void artLaminas(int x, int y, int s) {
    for (int i = 0; i < 11; i++) {
        PX(5 + i, 15 - i, 2, 1, PEDRA_CLARA);
        PX(6 + i, 15 - i, 1, 1, TEXTO_CLARO);
    }
    PX(3, 15, 5, 2, DOURADO);                // guarda
    PX(4, 17, 2, 3, MADEIRA_ESCURA);
    PX(3, 20, 3, 1, DOURADO_ESCURO);
    if (piscada(2.0f)) PX(15, 3, 1, 1, BRILHO_OURO);
    else PX(12, 6, 1, 1, BRILHO_OURO);
    PX(18, 8, 1, 1, BRILHO_OURO);
}

void artArmaduras(int x, int y, int s) {
    PX(4, 5, 4, 4, PEDRA);                   // ombreiras
    PX(14, 5, 4, 4, PEDRA);
    PX(6, 7, 10, 10, PEDRA_CLARA);
    PX(8, 5, 6, 2, PEDRA_CLARA);
    PX(9, 5, 4, 1, CONTORNO);                // gola
    PX(10, 8, 2, 9, PEDRA);
    PX(6, 14, 10, 1, MADEIRA_ESCURA);
    PX(10, 14, 2, 1, DOURADO);
    PX(7, 8, 1, 1, DOURADO);
    PX(14, 8, 1, 1, DOURADO);
    PX(7, 17, 8, 2, PEDRA);
}

void artEstandarte(int x, int y, int s) {
    PX(5, 1, 3, 1, DOURADO);
    PX(6, 2, 1, 18, MADEIRA_ESCURA);
    int balanco = piscada(2.0f);
    PX(7, 3, 10 + balanco, 9, TELHADO_AZUL);
    PX(7, 12, 3, 3, TELHADO_AZUL);
    PX(14 + balanco, 12, 3, 3, TELHADO_AZUL);
    PX(10, 6, 4, 3, DOURADO);                // coroa no pano
    PX(10, 5, 1, 1, DOURADO);
    PX(13, 5, 1, 1, DOURADO);
    PX(7, 3, 10 + balanco, 1, AZUL_CLARO);
}

void artBencao(int x, int y, int s) {
    PX(10, 0, 2, 4, BRILHO_OURO);            // raios
    PX(3, 3, 2, 2, BRILHO_OURO);
    PX(17, 3, 2, 2, BRILHO_OURO);
    PX(1, 9, 2, 1, BRILHO_OURO);
    PX(19, 9, 2, 1, BRILHO_OURO);
    PX(5, 10, 12, 5, DOURADO);
    PX(5, 7, 2, 3, DOURADO);
    PX(10, 6, 2, 4, DOURADO);
    PX(15, 7, 2, 3, DOURADO);
    PX(7, 12, 2, 2, TELHADO_VERM);
    PX(13, 12, 2, 2, TELHADO_AZUL);
    PX(10, 11, 2, 2, CRISTAL);
    PX(5, 15, 12, 2, DOURADO_ESCURO);
    if (piscada(2.5f)) PX(16, 16, 1, 1, TEXTO_CLARO);
}

void artExpansao(int x, int y, int s) {
    PX(14, 4, 3, 2, FOLHA);                  // pinheiro ao fundo
    PX(13, 6, 5, 3, FOLHA);
    PX(12, 9, 7, 3, FOLHA);
    PX(15, 12, 1, 2, MADEIRA_ESCURA);
    PX(4, 13, 8, 5, MADEIRA);                // toco
    PX(4, 12, 8, 2, PERGAMINHO_ESC);
    PX(6, 12, 4, 1, MADEIRA);
    PX(4, 17, 8, 1, MADEIRA_ESCURA);
    for (int i = 0; i < 6; i++) PX(4 + i, 4 + i, 1, 1, MADEIRA_ESCURA);   // machado
    PX(8, 8, 4, 3, PEDRA_CLARA);
    PX(11, 9, 1, 2, TEXTO_CLARO);
    PX(1, 18, 20, 1, GRAMA_ESCURA);
}

void artCaravana(int x, int y, int s) {
    PX(5, 4, 10, 1, PERGAMINHO);             // lona
    PX(4, 5, 12, 5, PERGAMINHO);
    PX(4, 9, 12, 1, PERGAMINHO_ESC);
    PX(3, 10, 14, 5, MADEIRA);
    PX(3, 14, 14, 1, MADEIRA_ESCURA);
    PX(13, 8, 4, 2, DOURADO);                // baú com ouro
    PX(14, 7, 2, 1, BRILHO_OURO);
    PX(4, 15, 4, 4, MADEIRA_ESCURA);         // rodas
    PX(12, 15, 4, 4, MADEIRA_ESCURA);
    PX(5, 16, 2, 2, MADEIRA);
    PX(13, 16, 2, 2, MADEIRA);
    PX(17, 12, 4, 1, MADEIRA_ESCURA);        // cabo
}

// ícones novos

void iconeCoracao(int x, int y, int s) {
    PX(0, 1, 3, 3, TELHADO_VERM);
    PX(4, 1, 3, 3, TELHADO_VERM);
    PX(1, 0, 2, 1, TELHADO_VERM);
    PX(4, 0, 2, 1, TELHADO_VERM);
    PX(1, 4, 5, 1, TELHADO_VERM);
    PX(2, 5, 3, 1, TELHADO_VERM);
    PX(3, 6, 1, 1, TELHADO_VERM);
    PX(1, 1, 1, 1, CORACAO_CLARO);
}

void iconeCristal(int x, int y, int s) {
    PX(2, 0, 3, 1, CRISTAL);
    PX(1, 1, 5, 2, CRISTAL);
    PX(2, 3, 3, 2, CRISTAL_ESC);
    PX(3, 5, 1, 2, CRISTAL_ESC);
    PX(2, 1, 1, 1, TEXTO_CLARO);
}

void iconeEspadas(int x, int y, int s) {
    for (int i = 0; i < 6; i++) {
        PX(i, i, 1, 1, PEDRA_CLARA);
        PX(6 - i, i, 1, 1, PEDRA_CLARA);
    }
    PX(0, 6, 2, 1, DOURADO);
    PX(5, 6, 2, 1, DOURADO);
}

void iconeEstrela(int x, int y, int s) {
    PX(3, 0, 1, 2, DOURADO);
    PX(0, 2, 7, 1, DOURADO);
    PX(1, 3, 5, 2, DOURADO);
    PX(1, 5, 2, 1, DOURADO);
    PX(4, 5, 2, 1, DOURADO);
    PX(0, 6, 1, 1, DOURADO);
    PX(6, 6, 1, 1, DOURADO);
    PX(3, 2, 1, 1, BRILHO_OURO);
}

Desenho desenhoDoEdificio(int tipo) {
    const Desenho desenhos[] = {desenharFazenda, desenharSerraria, desenharQuartel, desenharMuralha,
                                desenharOficina, desenharMina, desenharMercado, desenharCasa,
                                desenharTorre, desenharArquearia, desenharEstabulo, desenharTorreArcana};
    return desenhos[tipo];
}

void desenharComContorno(Desenho desenho, int x, int y, int s) {
    for (int passo = 0; passo < PASSOS_CONTORNO; passo++) {
        passoDoContorno(passo, CONTORNO);
        desenho(x, y, s);
    }
}
