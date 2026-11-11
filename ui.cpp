#include "ui.h"
#include "fonte.h"
#include "paleta.h"
#include <cmath>

float tempo() {
    return static_cast<float>(GetTime());
}

// caixa com cantos arredondados
void caixa(Rectangle r, Color borda, Color fundo) {
    int x = r.x, y = r.y, w = r.width, h = r.height;
    DrawRectangle(x + 1, y, w - 2, h, borda);
    DrawRectangle(x, y + 1, w, h - 2, borda);
    DrawRectangle(x + 1, y + 1, w - 2, h - 2, fundo);
}

void caixaComSombra(Rectangle r, Color fundo) {
    DrawRectangle(r.x + 1, r.y + r.height, r.width - 1, 2, Fade(CONTORNO, 0.45f));
    caixa(r, CONTORNO, fundo);
}

Rectangle areaNaJanela() {
    float w = static_cast<float>(GetScreenWidth()), h = static_cast<float>(GetScreenHeight());
    float escala = fminf(w / VL, h / VA);
    // perto de um número inteiro, usa o inteiro (pixel certinho); senão enche a tela
    if (escala - floorf(escala) < 0.05f && escala >= 1.0f) escala = floorf(escala);
    float largura = VL * escala, altura = VA * escala;
    return {(w - largura) / 2, (h - altura) / 2, largura, altura};
}

Vector2 mouseVirtual() {
    Vector2 m = GetMousePosition();
    Rectangle area = areaNaJanela();
    return {(m.x - area.x) * VL / area.width, (m.y - area.y) * VA / area.height};
}

bool mouseSobre(Rectangle r) {
    return CheckCollisionPointRec(mouseVirtual(), r);
}

bool clicouEm(Rectangle r) {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouseSobre(r);
}

void textoCentralizado(const char* texto, int y, int escala, Color cor) {
    int x = (VL - larguraTexto(texto, escala)) / 2;
    desenharTextoSombra(texto, x, y, escala, cor);
}

void textoCentralizadoEm(const char* texto, int centroX, int y, Color cor) {
    desenharTexto(texto, centroX - larguraTexto(texto, 1) / 2, y, 1, cor);
}

void botaozinho(Rectangle r, const char* texto, bool ativo) {
    bool sobre = mouseSobre(r);
    caixa(r, CONTORNO, ativo ? DOURADO : (sobre ? MADEIRA : MADEIRA_ESCURA));
    int x = static_cast<int>(r.x + (r.width - larguraTexto(texto, 1)) / 2);
    desenharTexto(texto, x, static_cast<int>(r.y + (r.height - 7) / 2), 1, ativo ? CONTORNO : TEXTO_CLARO);
}

void botaoPrincipal(Rectangle r, const char* texto) {
    bool sobre = mouseSobre(r);
    caixaComSombra(r, sobre ? Color{214, 84, 64, 255} : TELHADO_VERM);
    DrawRectangle(r.x + 2, r.y + 1, r.width - 4, 1, Fade(TEXTO_CLARO, 0.3f));
    int x = static_cast<int>(r.x + (r.width - larguraTexto(texto, 1)) / 2);
    desenharTextoSombra(texto, x, static_cast<int>(r.y + (r.height - 7) / 2), 1, sobre ? DOURADO : TEXTO_CLARO);
}

void desenharFaixa(int x, int y, int largura, int altura) {
    DrawRectangle(x - 8, y + 4, 10, altura, VERMELHO_ESC);
    DrawRectangle(x + largura - 2, y + 4, 10, altura, VERMELHO_ESC);
    caixa({static_cast<float>(x), static_cast<float>(y),
           static_cast<float>(largura), static_cast<float>(altura)}, CONTORNO, TELHADO_VERM);
    DrawRectangle(x + 1, y + altura - 3, largura - 2, 2, VERMELHO_ESC);
    DrawRectangle(x + 2, y + 1, largura - 4, 1, Fade(TEXTO_CLARO, 0.3f));
}

void barraDeVida(int x, int y, int largura, int vida, int maximo, Color cor) {
    DrawRectangle(x, y, largura, 2, CONTORNO);
    int cheio = maximo > 0 ? (largura * vida + maximo - 1) / maximo : 0;
    if (vida > 0 && cheio < 1) cheio = 1;
    DrawRectangle(x, y, cheio, 2, cor);
}
