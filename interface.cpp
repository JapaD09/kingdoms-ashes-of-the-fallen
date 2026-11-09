// Parte visual do Kingdoms, feita com raylib.
// Tudo que e cenario (ceu, castelo, brasas) e desenhado com formas simples,
// sem imagens externas, pra nao depender de arquivo de sprite.

#include "interface.h"
#include "raylib.h"
#include <cmath>
#include <string>

using namespace std;

namespace {

const int LARGURA = 1100;
const int ALTURA = 680;
const int CHAO = 600;
const int MAX_BRASAS = 80;

const Color CEU_TOPO     = {16, 12, 24, 255};
const Color CEU_BAIXO    = {92, 38, 30, 255};
const Color SILHUETA     = {22, 18, 26, 255};
const Color PAINEL       = {24, 20, 26, 235};
const Color BORDA        = {128, 88, 50, 255};
const Color DOURADO      = {236, 186, 88, 255};
const Color TEXTO        = {228, 218, 202, 255};
const Color TEXTO_FRACO  = {150, 138, 124, 255};
const Color BRASA        = {255, 118, 40, 255};
const Color JANELA_ACESA = {255, 196, 96, 255};

enum Tela { TELA_TITULO, TELA_REINO };

// fagulhas subindo no fundo, da o clima de "cinzas"
struct Brasa {
    float x, y;
    float velocidade;
    float brilho;
};

struct Botao {
    Rectangle area;
    const char* texto;
    int acao;    // mesmo numero da opcao do menu de terminal
    int tecla;
};

struct EstadoTela {
    Tela atual = TELA_TITULO;
    Brasa brasas[MAX_BRASAS];
    string mensagem = "Bem-vindo a Ashenfall. O reino aguarda suas ordens.";
    float tempoMensagem = 6.0f;
    float tempoTransicao = 0.0f;
    bool painelRecursos = false;
};

EstadoTela tela;

const Botao BOTOES[] = {
    {{790, 150, 280, 50}, "Gerenciar edificios", 1, KEY_ONE},
    {{790, 210, 280, 50}, "Recrutar soldados", 2, KEY_TWO},
    {{790, 270, 280, 50}, "Consultar recursos", 3, KEY_THREE},
    {{790, 330, 280, 50}, "Preparar expedicao", 4, KEY_FOUR},
    {{790, 390, 280, 50}, "Consultar catalogo", 5, KEY_FIVE},
    {{790, 450, 280, 50}, "Encerrar turno", 6, KEY_SIX},
    {{790, 530, 280, 44}, "Sair", 0, KEY_ZERO},
};
const int TOTAL_BOTOES = sizeof(BOTOES) / sizeof(BOTOES[0]);

// ---------------- utilidades ----------------

void bloco(float x, float y, float largura, float altura, Color cor) {
    DrawRectangleRec({x, y, largura, altura}, cor);
}

bool mouseSobre(Rectangle area) {
    return CheckCollisionPointRec(GetMousePosition(), area);
}

void textoComSombra(const char* texto, int x, int y, int tamanho, Color cor) {
    DrawText(texto, x + 2, y + 2, tamanho, Fade(BLACK, 0.6f));
    DrawText(texto, x, y, tamanho, cor);
}

void textoCentralizado(const char* texto, int y, int tamanho, Color cor) {
    int largura = MeasureText(texto, tamanho);
    textoComSombra(texto, (LARGURA - largura) / 2, y, tamanho, cor);
}

void avisar(const string& mensagem) {
    tela.mensagem = mensagem;
    tela.tempoMensagem = 5.0f;
}

float tempo() {
    return static_cast<float>(GetTime());
}

// ---------------- brasas ----------------

void reiniciarBrasa(Brasa& b, bool espalhar) {
    b.x = GetRandomValue(0, LARGURA);
    b.y = espalhar ? GetRandomValue(0, ALTURA) : ALTURA + GetRandomValue(0, 40);
    b.velocidade = GetRandomValue(20, 60);
    b.brilho = GetRandomValue(50, 100) / 100.0f;
}

void atualizarBrasas(float dt) {
    for (Brasa& b : tela.brasas) {
        b.y -= b.velocidade * dt;
        b.x += sinf(b.y * 0.02f) * 0.3f;   // balanca um pouco pros lados
        if (b.y < -10) {
            reiniciarBrasa(b, false);
        }
    }
}

void desenharBrasas() {
    for (const Brasa& b : tela.brasas) {
        float alpha = b.brilho * (b.y / ALTURA);   // apaga conforme sobe
        DrawCircleV({b.x, b.y}, 2.0f, Fade(BRASA, alpha));
    }
}

// ---------------- cenario ----------------

void desenharCeu() {
    DrawRectangleGradientV(0, 0, LARGURA, ALTURA, CEU_TOPO, CEU_BAIXO);
}

void desenharLua(int x, int y) {
    DrawCircle(x, y, 70, Fade(JANELA_ACESA, 0.05f));
    DrawCircle(x, y, 52, Fade(JANELA_ACESA, 0.08f));
    DrawCircle(x, y, 38, {238, 222, 196, 255});
    DrawCircle(x - 10, y - 8, 7, {214, 198, 172, 255});
    DrawCircle(x + 12, y + 10, 5, {214, 198, 172, 255});
}

void desenharColinas() {
    DrawCircle(150, CHAO + 160, 260, {40, 24, 30, 255});
    DrawCircle(620, CHAO + 220, 330, {36, 22, 28, 255});
    DrawCircle(1000, CHAO + 170, 260, {40, 24, 30, 255});
    bloco(0, CHAO, LARGURA, ALTURA - CHAO, SILHUETA);
}

// ameias = os "dentinhos" em cima da muralha
void desenharAmeias(float x, float y, float largura, float e) {
    for (float px = x; px + 9 * e <= x + largura; px += 16 * e) {
        bloco(px, y - 10 * e, 9 * e, 10 * e, SILHUETA);
    }
}

void desenharTorre(float x, float chao, float largura, float altura, float e) {
    float topo = chao - altura;
    bloco(x, topo, largura, altura, SILHUETA);
    DrawTriangle({x + largura / 2, topo - 50 * e},
                 {x - 6 * e, topo},
                 {x + largura + 6 * e, topo}, SILHUETA);
}

void desenharBandeira(float x, float topo, float e) {
    float onda = sinf(tempo() * 4.0f) * 4 * e;
    DrawLineEx({x, topo}, {x, topo - 45 * e}, 2 * e, SILHUETA);
    DrawTriangle({x, topo - 45 * e},
                 {x, topo - 25 * e},
                 {x + 30 * e, topo - 35 * e + onda}, {150, 40, 36, 255});
}

void desenharJanela(float x, float y, float e, int semente) {
    float pisca = 0.75f + 0.25f * sinf(tempo() * 3.0f + semente);
    bloco(x, y, 8 * e, 14 * e, Fade(JANELA_ACESA, pisca));
}

void desenharDetalhesCastelo(float x, float chao, float e) {
    DrawCircleV({x + 150 * e, chao - 50 * e}, 22 * e, {10, 8, 12, 255});
    bloco(x + 128 * e, chao - 50 * e, 44 * e, 50 * e, {10, 8, 12, 255});

    desenharJanela(x + 128 * e, chao - 178 * e, e, 0);
    desenharJanela(x + 164 * e, chao - 178 * e, e, 1);
    desenharJanela(x - 4 * e, chao - 140 * e, e, 2);
    desenharJanela(x + 296 * e, chao - 140 * e, e, 3);
    desenharJanela(x + 50 * e, chao - 85 * e, e, 4);
    desenharJanela(x + 242 * e, chao - 85 * e, e, 5);
}

// x = canto esquerdo da muralha, e = escala
void desenharCastelo(float x, float chao, float e) {
    bloco(x, chao - 120 * e, 300 * e, 120 * e, SILHUETA);
    desenharAmeias(x, chao - 120 * e, 300 * e, e);

    bloco(x + 105 * e, chao - 210 * e, 90 * e, 210 * e, SILHUETA);
    desenharAmeias(x + 105 * e, chao - 210 * e, 90 * e, e);

    desenharTorre(x - 30 * e, chao, 60 * e, 180 * e, e);
    desenharTorre(x + 270 * e, chao, 60 * e, 180 * e, e);
    desenharBandeira(x + 150 * e, chao - 220 * e, e);
    desenharDetalhesCastelo(x, chao, e);
}

// ---------------- icones dos recursos ----------------

typedef void (*FuncaoIcone)(float x, float y);

void iconeOuro(float x, float y) {
    DrawCircleV({x, y}, 10, {230, 180, 60, 255});
    DrawCircleV({x, y}, 6, {250, 214, 110, 255});
}

void iconeMadeira(float x, float y) {
    bloco(x - 11, y - 6, 22, 12, {130, 82, 44, 255});
    DrawCircleV({x + 11, y}, 6, {176, 124, 74, 255});
}

void iconePedra(float x, float y) {
    DrawPoly({x, y}, 5, 11, 15, {130, 128, 136, 255});
}

void iconeComida(float x, float y) {
    DrawCircleV({x, y + 2}, 9, {196, 60, 52, 255});
    DrawCircleV({x + 4, y - 8}, 4, {90, 160, 70, 255});
}

void iconeFerro(float x, float y) {
    DrawPoly({x, y}, 6, 10, 0, {120, 140, 160, 255});
    DrawPoly({x, y}, 6, 5, 0, {170, 186, 204, 255});
}

void iconePovo(float x, float y) {
    DrawCircleV({x, y - 7}, 5, TEXTO);
    bloco(x - 6, y - 1, 12, 11, TEXTO);
}

void iconeSoldado(float x, float y) {
    bloco(x - 2, y - 12, 4, 22, {190, 196, 206, 255});
    bloco(x - 8, y + 3, 16, 3, DOURADO);
}

// ---------------- paineis ----------------

void desenharRecurso(FuncaoIcone icone, int valor, const char* nome, float x) {
    icone(x + 12, 34);
    textoComSombra(TextFormat("%d", valor), static_cast<int>(x) + 32, 14, 24, TEXTO);
    DrawText(nome, static_cast<int>(x) + 32, 42, 14, TEXTO_FRACO);
}

void desenharBarraSuperior(const Jogo& jogo) {
    const Reino& reino = jogo.reino;
    bloco(0, 0, LARGURA, 72, PAINEL);
    DrawLineEx({0, 72}, {LARGURA, 72}, 2, BORDA);
    textoComSombra(jogo.nomeReino.c_str(), 20, 10, 28, DOURADO);
    DrawText(TextFormat("Turno %d", jogo.turno), 22, 44, 18, TEXTO_FRACO);

    desenharRecurso(iconeOuro, reino.estoque.ouro, "ouro", 240);
    desenharRecurso(iconeMadeira, reino.estoque.madeira, "madeira", 355);
    desenharRecurso(iconePedra, reino.estoque.pedra, "pedra", 470);
    desenharRecurso(iconeComida, reino.estoque.comida, "comida", 585);
    desenharRecurso(iconeFerro, reino.estoque.ferro, "ferro", 700);
    desenharRecurso(iconePovo, reino.populacao, "populacao", 815);
    desenharRecurso(iconeSoldado, reino.soldados, "soldados", 950);
}

void desenharBotao(const Botao& b) {
    bool sobre = mouseSobre(b.area);
    Color fundo = sobre ? Color{74, 46, 32, 255} : Color{40, 32, 36, 255};
    DrawRectangleRec(b.area, fundo);
    DrawRectangleLinesEx(b.area, 2, sobre ? DOURADO : BORDA);

    const char* rotulo = TextFormat("[%d]  %s", b.acao, b.texto);
    int x = static_cast<int>(b.area.x) + 18;
    int y = static_cast<int>(b.area.y + b.area.height / 2) - 10;
    DrawText(rotulo, x, y, 20, sobre ? DOURADO : TEXTO);
}

void desenharPainelOrdens() {
    Rectangle painel = {770, 92, 320, 500};
    DrawRectangleRec(painel, PAINEL);
    DrawRectangleLinesEx(painel, 2, BORDA);
    textoComSombra("Ordens do Reino", 800, 110, 24, DOURADO);

    for (int i = 0; i < TOTAL_BOTOES; i++) {
        desenharBotao(BOTOES[i]);
    }
}

void desenharBarraMensagem() {
    if (tela.tempoMensagem <= 0) {
        return;
    }
    float alpha = tela.tempoMensagem > 1 ? 1.0f : tela.tempoMensagem;
    Rectangle barra = {20, ALTURA - 62, 740, 44};
    DrawRectangleRec(barra, Fade(PAINEL, alpha));
    DrawRectangleLinesEx(barra, 1, Fade(BORDA, alpha));
    DrawText(tela.mensagem.c_str(), 36, ALTURA - 50, 20, Fade(TEXTO, alpha));
}

void linhaRecurso(const char* nome, int valor, int y) {
    DrawText(nome, 120, y, 22, TEXTO);
    DrawText(TextFormat("%d", valor), 380, y, 22, DOURADO);
}

void desenharPainelRecursos(const Reino& reino) {
    Rectangle painel = {90, 120, 400, 380};
    DrawRectangleRec(painel, PAINEL);
    DrawRectangleLinesEx(painel, 2, DOURADO);
    textoComSombra("Tesouro do Reino", 120, 140, 26, DOURADO);

    linhaRecurso("Ouro", reino.estoque.ouro, 195);
    linhaRecurso("Madeira", reino.estoque.madeira, 231);
    linhaRecurso("Pedra", reino.estoque.pedra, 267);
    linhaRecurso("Comida", reino.estoque.comida, 303);
    linhaRecurso("Ferro", reino.estoque.ferro, 339);
    linhaRecurso("Populacao", reino.populacao, 375);
    linhaRecurso("Soldados", reino.soldados, 411);
    DrawText("ESC ou [3] para fechar", 120, 466, 16, TEXTO_FRACO);
}

void desenharTransicaoTurno(int turno) {
    if (tela.tempoTransicao <= 0) {
        return;
    }
    float alpha = tela.tempoTransicao / 1.2f;
    DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, alpha * 0.6f));
    textoCentralizado(TextFormat("TURNO %d", turno), ALTURA / 2 - 40, 72, Fade(DOURADO, alpha));
}

// ---------------- telas ----------------

void desenharTelaTitulo() {
    desenharCeu();
    desenharLua(960, 110);
    desenharColinas();
    desenharCastelo(LARGURA / 2 - 150, CHAO, 1.1f);
    desenharBrasas();

    textoCentralizado("KINGDOMS", 70, 96, DOURADO);
    textoCentralizado("~ Ashes of the Fallen ~", 175, 30, TEXTO);
    float pisca = 0.5f + 0.5f * sinf(tempo() * 2.5f);
    textoCentralizado("Clique ou aperte ENTER para comecar", 632, 22, Fade(TEXTO, pisca));
}

void desenharTelaReino(const Jogo& jogo) {
    desenharCeu();
    desenharLua(620, 160);
    desenharColinas();
    desenharCastelo(230, CHAO, 1.0f);
    desenharBrasas();

    desenharBarraSuperior(jogo);
    desenharPainelOrdens();
    desenharBarraMensagem();
    if (tela.painelRecursos) {
        desenharPainelRecursos(jogo.reino);
    }
    desenharTransicaoTurno(jogo.turno);
}

// ---------------- entrada ----------------

void executarAcao(Jogo& jogo, int acao) {
    switch (acao) {
        case 3:
            tela.painelRecursos = !tela.painelRecursos;
            break;
        case 6:
            tela.painelRecursos = false;
            avisar(encerrarTurno(jogo));
            tela.tempoTransicao = 1.2f;
            break;
        case 0:
            jogo.rodando = false;
            break;
        default:
            avisar("Essa ala do castelo ainda esta em obras.");
            break;
    }
}

// devolve a acao do botao clicado (ou da tecla), -1 se nada
int acaoEscolhida() {
    bool clique = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    for (int i = 0; i < TOTAL_BOTOES; i++) {
        const Botao& b = BOTOES[i];
        if ((clique && mouseSobre(b.area)) || IsKeyPressed(b.tecla)) {
            return b.acao;
        }
    }
    return -1;
}

void atualizarTelaReino(Jogo& jogo) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        tela.painelRecursos = false;
    }
    if (tela.tempoTransicao > 0) {
        return;   // segura os cliques enquanto o "TURNO X" aparece
    }
    int acao = acaoEscolhida();
    if (acao >= 0) {
        executarAcao(jogo, acao);
    }
}

}  // fim do namespace anonimo

void abrirJanela() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(LARGURA, ALTURA, "Kingdoms: Ashes of the Fallen");
    SetExitKey(KEY_NULL);   // ESC fecha painel, nao o jogo
    SetTargetFPS(60);
    for (Brasa& b : tela.brasas) {
        reiniciarBrasa(b, true);
    }
}

bool janelaAberta() {
    return !WindowShouldClose();
}

void fecharJanela() {
    CloseWindow();
}

void atualizarInterface(Jogo& jogo) {
    float dt = GetFrameTime();
    atualizarBrasas(dt);
    if (tela.tempoMensagem > 0) tela.tempoMensagem -= dt;
    if (tela.tempoTransicao > 0) tela.tempoTransicao -= dt;

    if (tela.atual == TELA_TITULO) {
        if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            tela.atual = TELA_REINO;
        }
        return;
    }
    atualizarTelaReino(jogo);
}

void desenharInterface(const Jogo& jogo) {
    BeginDrawing();
    ClearBackground(BLACK);
    if (tela.atual == TELA_TITULO) {
        desenharTelaTitulo();
    } else {
        desenharTelaReino(jogo);
    }
    EndDrawing();
}
