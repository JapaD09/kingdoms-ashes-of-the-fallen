// Parte visual do Kingdoms, feita com raylib.
//
// Tudo é desenhado numa tela pequena de 320x180 e depois ampliado 4x com
// filtro POINT, assim cada retângulo vira um pixel de verdade.
// As regras ficam em jogo.cpp / reino.cpp; aqui só tem desenho e clique.

#include "interface.h"
#include "fonte.h"
#include "paleta.h"
#include "sprites.h"
#include "raylib.h"
#include <cmath>
#include <string>

using namespace std;

namespace {

const int VL = 320;       // largura virtual
const int VA = 180;       // altura virtual
const int ESCALA = 4;
const int GRADE = 5;      // reino 5x5, começa com o 3x3 do meio liberado
const int LOTE = 22;
const int GRADE_X = 105;
const int GRADE_Y = 19;
const int MAX_BRASAS = 40;

enum Tela { TELA_TITULO, TELA_REINO };

// fagulhas subindo: o "ashes" do nome
struct Brasa {
    float x, y;
    float velocidade;
};

// Carta = um tipo de edifício. Nome e custo vêm do catálogo do reino;
// aqui só fica o que é visual.
struct Carta {
    int tipo;
    const char* efeito;
    Desenho desenho;
};

struct Botao {
    Rectangle area;
    const char* texto;
    int acao;    // mesmos números do menu antigo de terminal
    Desenho icone;
};

struct EstadoTela {
    Tela atual = TELA_TITULO;
    Brasa brasas[MAX_BRASAS];
    string mensagem;
    float tempoMensagem = 0.0f;
    float tempoTransicao = 0.0f;
    int cartaSelecionada = -1;
    bool painelRecursos = false;
    const char* dica = nullptr;   // texto que aparece do lado do mouse
};

EstadoTela tela;
RenderTexture2D alvo;

const Carta CARTAS[] = {
    {FAZENDA, "+8 comida\npor turno", desenharFazenda},
    {SERRARIA, "+6 madeira\npor turno", desenharSerraria},
    {QUARTEL, "libera\nrecrutar", desenharQuartel},
    {MURALHA, "+5 defesa\ndo reino", desenharMuralha},
    {OFICINA, "armas e\narmaduras", desenharOficina},
};
const int TOTAL_CARTAS = sizeof(CARTAS) / sizeof(CARTAS[0]);

const Botao BOTOES[] = {
    {{4, 22, 70, 17}, "Recrutar", 2, iconeSoldado},
    {{4, 42, 70, 17}, "Expedição", 4, iconeMapa},
    {{4, 62, 70, 17}, "Catálogo", 5, iconeLivro},
    {{4, 82, 70, 17}, "Recursos", 3, iconeBau},
    {{246, 150, 70, 27}, "Fim do turno", 6, iconeAmpulheta},
    {{4, 160, 44, 17}, "Sair", 0, iconePorta},
};
const int TOTAL_BOTOES = sizeof(BOTOES) / sizeof(BOTOES[0]);

// ================= utilidades =================

float tempo() {
    return static_cast<float>(GetTime());
}

// caixa com os cantos "cortados", fica com cara de pixel art arredondada
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

Vector2 mouseVirtual() {
    Vector2 m = GetMousePosition();
    return {m.x / ESCALA, m.y / ESCALA};
}

bool mouseSobre(Rectangle r) {
    return CheckCollisionPointRec(mouseVirtual(), r);
}

void textoCentralizado(const char* texto, int y, int escala, Color cor) {
    int x = (VL - larguraTexto(texto, escala)) / 2;
    desenharTextoSombra(texto, x, y, escala, cor);
}

void avisar(const string& mensagem) {
    tela.mensagem = mensagem;
    tela.tempoMensagem = 3.5f;
}

bool podePagar(const Reino& reino, const Carta& carta) {
    return temRecursos(reino.estoque, modeloEdificio(carta.tipo).custo);
}

Rectangle areaLote(int linha, int coluna) {
    return {static_cast<float>(GRADE_X + coluna * LOTE),
            static_cast<float>(GRADE_Y + linha * LOTE),
            static_cast<float>(LOTE), static_cast<float>(LOTE)};
}

Rectangle areaCarta(int i) {
    return {static_cast<float>(75 + i * 34), 138, 32, 42};
}

int cartaSobMouse() {
    for (int i = 0; i < TOTAL_CARTAS; i++) {
        if (mouseSobre(areaCarta(i))) return i;
    }
    return -1;
}

// true se o mouse está em cima da grade; preenche linha e coluna
bool loteSobMouse(int& linha, int& coluna) {
    Vector2 m = mouseVirtual();
    if (m.x < GRADE_X || m.y < GRADE_Y) return false;
    coluna = static_cast<int>(m.x - GRADE_X) / LOTE;
    linha = static_cast<int>(m.y - GRADE_Y) / LOTE;
    return linha < GRADE && coluna < GRADE && cartaSobMouse() < 0;
}

// ================= brasas =================

void reiniciarBrasa(Brasa& b, bool espalhar) {
    b.x = GetRandomValue(0, VL);
    b.y = espalhar ? GetRandomValue(0, VA) : VA + GetRandomValue(0, 20);
    b.velocidade = GetRandomValue(6, 16);
}

void atualizarBrasas(float dt) {
    for (Brasa& b : tela.brasas) {
        b.y -= b.velocidade * dt;
        b.x += sinf(b.y * 0.1f) * 0.05f;
        if (b.y < 0) reiniciarBrasa(b, false);
    }
}

void desenharBrasas() {
    for (const Brasa& b : tela.brasas) {
        float alpha = b.y / VA;   // vai apagando conforme sobe
        DrawRectangle(static_cast<int>(b.x), static_cast<int>(b.y), 1, 1, Fade(BRASA, alpha));
    }
}

// ================= cenário =================

void desenharFundo() {
    ClearBackground(FUNDO);
    for (int i = 0; i < 220; i++) {
        int x = (i * 73 + i / 7) % VL;
        int y = (i * 151 + i * i) % VA;
        Color cor = FUNDO_PONTO;
        if (i % 23 == 0) cor = FLOR_AMARELA;
        else if (i % 31 == 0) cor = FLOR_BRANCA;
        DrawRectangle(x, y, (i % 3 == 0) ? 2 : 1, 1, cor);
    }
}

// sombras de nuvem passando devagar por cima do mapa
void desenharSombrasNuvens() {
    Color sombra = Fade(CONTORNO, 0.13f);
    for (int i = 0; i < 3; i++) {
        float andou = fmodf(tempo() * 4.0f + i * 140.0f, VL + 120.0f);
        int x = static_cast<int>(andou) - 60;
        int y = 34 + i * 48;
        DrawRectangle(x + 8, y - 4, 30, 4, sombra);
        DrawRectangle(x, y, 50, 10, sombra);
        DrawRectangle(x + 4, y + 10, 36, 4, sombra);
    }
}

// escurece as bordas da tela
void desenharVinheta() {
    for (int i = 0; i < 8; i++) {
        Rectangle borda = {static_cast<float>(i), static_cast<float>(i),
                           static_cast<float>(VL - 2 * i), static_cast<float>(VA - 2 * i)};
        DrawRectangleLinesEx(borda, 1, Fade(CONTORNO, 0.32f - i * 0.04f));
    }
}

void desenharArvoresEmVolta() {
    const int pos[][2] = {{80, 26}, {92, 56}, {79, 90}, {93, 112}, {84, 128},
                          {222, 28}, {232, 62}, {221, 98}, {234, 118}};
    for (const auto& p : pos) {
        desenharArvore(p[0], p[1], 1);
    }
}

void desenharLote(int linha, int coluna) {
    Rectangle r = areaLote(linha, coluna);
    int x = r.x, y = r.y;
    bool livre = loteLiberado(linha, coluna);
    Color base = (linha + coluna) % 2 ? GRAMA : GRAMA_ALT;

    DrawRectangle(x, y, LOTE, LOTE, livre ? base : GRAMA_ESCURA);
    int semente = linha * 7 + coluna * 13 + 5;
    for (int i = 0; i < 5; i++) {         // tufinhos em posições fixas
        int tx = x + 2 + (semente * (i + 3)) % 18;
        int ty = y + 2 + (semente * (i + 5) + i * 7) % 18;
        DrawRectangle(tx, ty, 1, 1, livre ? GRAMA_CLARA : GRAMA);
    }
    if (!livre) {
        desenharArvore(x + 2, y + 2, 1);
        desenharArvore(x + 12, y + 6, 1);
        desenharArvore(x + 5, y + 11, 1);
        DrawRectangle(x, y, LOTE, LOTE, Fade(CONTORNO, 0.18f));
    }
    DrawRectangleLines(x, y, LOTE, LOTE, Fade(CONTORNO, 0.25f));
}

// a grade fica num "planalto" com barranco de terra embaixo
void desenharPlanalto() {
    int lado = GRADE * LOTE + 6;
    int x = GRADE_X - 3, y = GRADE_Y - 3;
    DrawRectangle(x + 3, y + lado + 6, lado, 4, Fade(CONTORNO, 0.35f));
    DrawRectangle(x, y, lado, lado, GRAMA_ESCURA);
    DrawRectangle(x, y + lado, lado, 6, TERRA);
    for (int i = 0; i < lado - 4; i += 7) {
        DrawRectangle(x + 2 + i, y + lado + 1 + (i / 7 % 2) * 2, 4, 2, TERRA_ESCURA);
    }
    DrawRectangleLines(x - 1, y - 1, lado + 2, lado + 8, CONTORNO);
}

void desenharDestaqueLote(const Reino& reino) {
    int linha, coluna;
    if (!loteSobMouse(linha, coluna)) return;
    Rectangle r = areaLote(linha, coluna);
    const Edificio* ocupante = buscarEdificioNoLote(reino.edificios, linha, coluna);
    if (ocupante != nullptr) tela.dica = modeloEdificio(ocupante->tipo).nome;
    if (ehCastelo(linha, coluna)) tela.dica = "Castelo";
    bool podeConstruir = loteLiberado(linha, coluna) && !ehCastelo(linha, coluna)
                      && ocupante == nullptr;
    float pulso = 0.55f + 0.45f * sinf(tempo() * 6.0f);

    if (tela.cartaSelecionada >= 0 && podeConstruir) {
        CARTAS[tela.cartaSelecionada].desenho(r.x, r.y, 1);   // prévia da construção
        DrawRectangle(r.x, r.y, LOTE, LOTE, Fade(TEXTO_CLARO, 0.15f * pulso));
        DrawRectangleLinesEx(r, 1, Fade(DOURADO, pulso));
        return;
    }
    DrawRectangleLinesEx(r, 1, Fade(TEXTO_CLARO, 0.6f));
}

// percorre a lista encadeada e desenha cada edifício no seu lote
void desenharEdificios(const Reino& reino) {
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        const Edificio& e = no->dados;
        Rectangle r = areaLote(e.linha, e.coluna);
        CARTAS[e.tipo].desenho(r.x, r.y, 1);
    }
}

void desenharGrade(const Reino& reino) {
    desenharPlanalto();
    for (int l = 0; l < GRADE; l++) {
        for (int c = 0; c < GRADE; c++) {
            desenharLote(l, c);
        }
    }
    desenharCastelo(GRADE_X + 2 * LOTE, GRADE_Y + 2 * LOTE, 1);
    desenharEdificios(reino);
    desenharDestaqueLote(reino);
}

// ================= HUD =================

int itemTopo(Desenho icone, int valor, const char* nome, int x) {
    const char* numero = TextFormat("%d", valor);
    int largura = 13 + larguraTexto(numero, 1);
    Rectangle r = {static_cast<float>(x), 2, static_cast<float>(largura), 11};

    caixa(r, Fade(CONTORNO, 0.6f), Fade(CONTORNO, 0.35f));
    icone(x + 2, 4, 1);
    desenharTexto(numero, x + 11, 4, 1, TEXTO_CLARO);
    if (mouseSobre(r)) tela.dica = nome;
    return x + largura + 3;
}

void desenharBarraTopo(const Jogo& jogo) {
    const Reino& reino = jogo.reino;
    DrawRectangle(0, 0, VL, 15, MADEIRA_ESCURA);
    for (int i = 37; i < VL; i += 46) DrawRectangle(i, 0, 1, 15, Fade(CONTORNO, 0.4f));
    DrawRectangle(0, 0, VL, 1, MADEIRA);
    DrawRectangle(0, 15, VL, 1, CONTORNO);

    iconeCoroa(4, 4, 1);
    desenharTextoSombra(TextFormat("Turno %d", jogo.turno), 14, 4, 1, DOURADO);

    int x = 64;
    x = itemTopo(iconeOuro, reino.estoque.ouro, "Ouro", x);
    x = itemTopo(iconeMadeira, reino.estoque.madeira, "Madeira", x);
    x = itemTopo(iconePedra, reino.estoque.pedra, "Pedra", x);
    x = itemTopo(iconeComida, reino.estoque.comida, "Comida", x);
    x = itemTopo(iconeFerro, reino.estoque.ferro, "Ferro", x);
    x = itemTopo(iconePovo, reino.populacao, "População", x);
    itemTopo(iconeSoldado, reino.soldados, "Soldados", x);
}

void desenharBotao(const Botao& b) {
    bool sobre = mouseSobre(b.area);
    bool principal = b.acao == 6;
    Color fundo = principal ? TELHADO_VERM : MADEIRA_ESCURA;
    if (sobre) fundo = principal ? Color{214, 84, 64, 255} : MADEIRA;
    Color corTexto = sobre ? DOURADO : TEXTO_CLARO;
    int x = b.area.x, y = b.area.y, w = b.area.width;

    caixaComSombra(b.area, fundo);
    DrawRectangle(x + 2, y + 1, w - 4, 1, Fade(TEXTO_CLARO, 0.25f));
    if (principal) {   // ícone em cima, texto embaixo
        b.icone(x + (w - 7) / 2, y + 4, 1);
        desenharTextoSombra(b.texto, x + (w - larguraTexto(b.texto, 1)) / 2, y + 14, 1, corTexto);
    } else {
        b.icone(x + 4, y + 5, 1);
        desenharTextoSombra(b.texto, x + 14, y + 5, 1, corTexto);
    }
}

// setinha balançando em cima da carta escolhida
void desenharSeta(int x, int y) {
    int pulo = static_cast<int>(sinf(tempo() * 6.0f) * 1.5f);
    DrawRectangle(x - 2, y + pulo, 5, 1, DOURADO);
    DrawRectangle(x - 1, y + 1 + pulo, 3, 1, DOURADO);
    DrawRectangle(x, y + 2 + pulo, 1, 1, DOURADO);
}

void desenharCarta(int i, const Reino& reino) {
    Rectangle r = areaCarta(i);
    bool selecionada = i == tela.cartaSelecionada;
    bool podeComprar = podePagar(reino, CARTAS[i]);
    if (selecionada || mouseSobre(r)) r.y -= 5;    // a carta "sobe" na mão
    int x = r.x, y = r.y;

    caixaComSombra(r, PERGAMINHO);
    DrawRectangle(x + 2, y + 2, 28, 26, PERGAMINHO_ESC);
    DrawRectangle(x + 3, y + 3, 26, 24, (i % 2) ? GRAMA : GRAMA_ALT);
    CARTAS[i].desenho(x + 5, y + 5, 1);
    if (!podeComprar) DrawRectangle(x + 3, y + 3, 26, 24, Fade(CONTORNO, 0.45f));

    iconeOuro(x + 5, y + 31, 1);
    const char* custo = TextFormat("%d", modeloEdificio(CARTAS[i].tipo).custo.ouro);
    desenharTexto(custo, x + 14, y + 31, 1, podeComprar ? CONTORNO : TELHADO_VERM);
    if (selecionada) {
        DrawRectangleLinesEx({r.x - 1, r.y - 1, r.width + 2, r.height + 2}, 1, DOURADO);
        desenharSeta(x + 16, y - 6);
    }
}

int linhaCusto(Desenho icone, int custo, int tenho, int y) {
    if (custo <= 0) return y;
    icone(251, y, 1);
    Color cor = tenho >= custo ? CONTORNO : TELHADO_VERM;
    desenharTexto(TextFormat("%d", custo), 261, y, 1, cor);
    return y + 10;
}

void desenharInfoCarta(const Carta& carta, const Reino& reino) {
    const Recursos& r = reino.estoque;
    const ModeloEdificio& modelo = modeloEdificio(carta.tipo);
    desenharTexto(modelo.nome, 251, 25, 1, VERMELHO_ESC);
    DrawRectangle(251, 35, 60, 1, PERGAMINHO_ESC);
    int y = linhaCusto(iconeOuro, modelo.custo.ouro, r.ouro, 39);
    y = linhaCusto(iconeMadeira, modelo.custo.madeira, r.madeira, y);
    y = linhaCusto(iconePedra, modelo.custo.pedra, r.pedra, y);
    y = linhaCusto(iconeFerro, modelo.custo.ferro, r.ferro, y);
    DrawRectangle(251, y + 2, 60, 1, PERGAMINHO_ESC);
    desenharTexto(carta.efeito, 251, y + 7, 1, MADEIRA_ESCURA);
}

// mostra +X / -X com a cor certa (verde ganhando, vermelho perdendo)
int linhaPrevisao(Desenho icone, int valor, int y) {
    if (valor == 0) return y;
    icone(251, y, 1);
    Color cor = valor > 0 ? FOLHA : TELHADO_VERM;
    desenharTexto(TextFormat("%+d", valor), 261, y, 1, cor);
    return y + 10;
}

void desenharResumoReino(const Reino& reino) {
    desenharTexto("Ashenfall", 251, 25, 1, VERMELHO_ESC);
    DrawRectangle(251, 35, 60, 1, PERGAMINHO_ESC);
    desenharTexto("Por turno:", 251, 39, 1, MADEIRA_ESCURA);

    Recursos p = previsaoDoTurno(reino);
    int y = linhaPrevisao(iconeOuro, p.ouro, 50);
    y = linhaPrevisao(iconeMadeira, p.madeira, y);
    y = linhaPrevisao(iconePedra, p.pedra, y);
    y = linhaPrevisao(iconeComida, p.comida, y);
    y = linhaPrevisao(iconeFerro, p.ferro, y);

    DrawRectangle(251, y + 2, 60, 1, PERGAMINHO_ESC);
    desenharTexto(TextFormat("Edifícios: %d", reino.edificios.tamanho), 251, y + 7, 1, MADEIRA_ESCURA);
    desenharTexto(TextFormat("Defesa: %d", reino.defesa), 251, y + 18, 1, MADEIRA_ESCURA);
    desenharTexto("1-5: cartas\nEspaço: fim", 251, 115, 1, PERGAMINHO_ESC);
}

void desenharPainelInfo(const Reino& reino) {
    caixaComSombra({246, 20, 70, 124}, PERGAMINHO);
    DrawRectangle(248, 22, 66, 120, Fade(PERGAMINHO_ESC, 0.25f));
    int i = cartaSobMouse();
    if (i < 0) i = tela.cartaSelecionada;
    if (i >= 0) {
        desenharInfoCarta(CARTAS[i], reino);
        return;
    }
    desenharResumoReino(reino);
}

void linhaRecurso(Desenho icone, const char* nome, int valor, int y) {
    icone(108, y, 1);
    desenharTexto(nome, 118, y, 1, CONTORNO);
    const char* numero = TextFormat("%d", valor);
    desenharTexto(numero, 212 - larguraTexto(numero, 1), y, 1, VERMELHO_ESC);
}

void desenharPainelRecursos(const Reino& reino) {
    caixaComSombra({100, 30, 120, 104}, PERGAMINHO);
    desenharTexto("Tesouro do Reino", 108, 35, 1, VERMELHO_ESC);
    DrawRectangle(108, 45, 104, 1, PERGAMINHO_ESC);
    linhaRecurso(iconeOuro, "Ouro", reino.estoque.ouro, 50);
    linhaRecurso(iconeMadeira, "Madeira", reino.estoque.madeira, 61);
    linhaRecurso(iconePedra, "Pedra", reino.estoque.pedra, 72);
    linhaRecurso(iconeComida, "Comida", reino.estoque.comida, 83);
    linhaRecurso(iconeFerro, "Ferro", reino.estoque.ferro, 94);
    linhaRecurso(iconePovo, "População", reino.populacao, 105);
    linhaRecurso(iconeSoldado, "Soldados", reino.soldados, 116);
}

void desenharMensagem() {
    if (tela.tempoMensagem <= 0) return;
    float alpha = tela.tempoMensagem > 1 ? 1.0f : tela.tempoMensagem;
    const char* texto = tela.mensagem.c_str();
    int largura = larguraTexto(texto, 1) + 12;
    Rectangle r = {static_cast<float>((VL - largura) / 2), 116, static_cast<float>(largura), 14};
    caixa(r, Fade(DOURADO_ESCURO, alpha), Fade(CONTORNO, alpha * 0.9f));
    desenharTexto(texto, r.x + 6, 120, 1, Fade(TEXTO_CLARO, alpha));
}

void desenharDica() {
    if (tela.dica == nullptr) return;
    Vector2 m = mouseVirtual();
    int largura = larguraTexto(tela.dica, 1) + 8;
    int x = static_cast<int>(m.x) + 6;
    if (x + largura > VL) x = VL - largura;
    caixa({static_cast<float>(x), m.y + 6, static_cast<float>(largura), 13}, CONTORNO, PERGAMINHO);
    desenharTexto(tela.dica, x + 4, static_cast<int>(m.y) + 9, 1, CONTORNO);
}

// faixa vermelha estilo estandarte
void desenharFaixa(int x, int y, int largura, int altura) {
    DrawRectangle(x - 8, y + 4, 10, altura, VERMELHO_ESC);           // pontas de trás
    DrawRectangle(x + largura - 2, y + 4, 10, altura, VERMELHO_ESC);
    caixa({static_cast<float>(x), static_cast<float>(y),
           static_cast<float>(largura), static_cast<float>(altura)}, CONTORNO, TELHADO_VERM);
    DrawRectangle(x + 1, y + altura - 3, largura - 2, 2, VERMELHO_ESC);
    DrawRectangle(x + 2, y + 1, largura - 4, 1, Fade(TEXTO_CLARO, 0.3f));
}

void desenharTransicao(int turno) {
    if (tela.tempoTransicao <= 0) return;
    float alpha = tela.tempoTransicao > 0.4f ? 1.0f : tela.tempoTransicao / 0.4f;
    DrawRectangle(0, 0, VL, VA, Fade(CONTORNO, alpha * 0.55f));
    if (alpha < 1.0f) return;
    desenharFaixa(100, 74, 120, 26);
    const char* texto = TextFormat("Turno %d", turno);
    desenharTextoContorno(texto, (VL - larguraTexto(texto, 2)) / 2, 80, 2,
                          BRILHO_OURO, DOURADO, CONTORNO);
}

// ================= telas =================

void desenharIlhaTitulo() {
    DrawRectangle(106, 143, 112, 4, Fade(CONTORNO, 0.35f));
    DrawRectangle(104, 131, 112, 8, GRAMA);
    DrawRectangle(100, 133, 120, 4, GRAMA);
    DrawRectangle(104, 139, 112, 4, TERRA);
    DrawRectangle(104, 143, 112, 1, TERRA_ESCURA);
    for (int i = 0; i < 112; i += 9) DrawRectangle(106 + i, 132 + i % 4, 1, 1, GRAMA_CLARA);
}

void desenharTelaTitulo() {
    desenharFundo();
    desenharSombrasNuvens();
    const int arvores[][2] = {{36, 92}, {66, 122}, {24, 140}, {252, 96}, {274, 128}, {228, 148}};
    for (const auto& a : arvores) desenharArvore(a[0], a[1], 2);
    desenharIlhaTitulo();
    desenharCastelo(127, 74, 3);
    desenharBrasas();

    int x = (VL - larguraTexto("KINGDOMS", 4)) / 2;
    desenharTextoContorno("KINGDOMS", x, 10, 4, BRILHO_OURO, DOURADO_ESCURO, CONTORNO);
    desenharFaixa(96, 46, 128, 15);
    textoCentralizado("Ashes of the Fallen", 50, 1, TEXTO_CLARO);
    if (static_cast<int>(tempo() * 2) % 2 == 0) {
        textoCentralizado("Clique para começar", 158, 1, TEXTO_CLARO);
    }
    desenharTexto("Eduarda & Mateus", 4, 170, 1, Fade(TEXTO_CLARO, 0.35f));
    desenharVinheta();
}

void desenharTelaReino(const Jogo& jogo) {
    tela.dica = nullptr;
    desenharFundo();
    desenharArvoresEmVolta();
    desenharGrade(jogo.reino);
    desenharSombrasNuvens();
    desenharBrasas();
    desenharVinheta();

    for (int i = 0; i < TOTAL_BOTOES; i++) desenharBotao(BOTOES[i]);
    desenharPainelInfo(jogo.reino);
    for (int i = 0; i < TOTAL_CARTAS; i++) desenharCarta(i, jogo.reino);
    desenharBarraTopo(jogo);
    if (tela.painelRecursos) desenharPainelRecursos(jogo.reino);
    desenharMensagem();
    desenharDica();
    desenharTransicao(jogo.turno);
}

// ================= entrada =================

void executarAcao(Jogo& jogo, int acao) {
    switch (acao) {
        case 3:
            tela.painelRecursos = !tela.painelRecursos;
            break;
        case 6:
            tela.cartaSelecionada = -1;
            tela.painelRecursos = false;
            avisar(encerrarTurno(jogo));
            tela.tempoTransicao = 1.3f;
            break;
        case 0:
            jogo.rodando = false;
            break;
        default:
            avisar("Essa parte do reino ainda está em obras.");
            break;
    }
}

void cliqueNoLote(Reino& reino, int linha, int coluna) {
    const Edificio* ocupante = buscarEdificioNoLote(reino.edificios, linha, coluna);
    if (ehCastelo(linha, coluna)) {
        avisar("O castelo de Ashenfall.");
    } else if (ocupante != nullptr && tela.cartaSelecionada < 0) {
        avisar(string(modeloEdificio(ocupante->tipo).nome) + ". Botão direito demole.");
    } else if (tela.cartaSelecionada < 0) {
        avisar("Escolha uma carta lá embaixo.");
    } else {
        avisar(construirEdificio(reino, CARTAS[tela.cartaSelecionada].tipo, linha, coluna));
    }
}

// botão direito: em cima de um edifício demole, em qualquer outro lugar cancela a carta
void cliqueDireito(Reino& reino) {
    int linha, coluna;
    bool emLote = loteSobMouse(linha, coluna);
    if (emLote && buscarEdificioNoLote(reino.edificios, linha, coluna) != nullptr) {
        avisar(demolirEdificio(reino, linha, coluna));
        return;
    }
    tela.cartaSelecionada = -1;
    tela.painelRecursos = false;
}

void tratarClique(Jogo& jogo) {
    int carta = cartaSobMouse();
    if (carta >= 0) {
        tela.cartaSelecionada = (carta == tela.cartaSelecionada) ? -1 : carta;
        return;
    }
    int linha, coluna;
    if (loteSobMouse(linha, coluna)) {
        cliqueNoLote(jogo.reino, linha, coluna);
        return;
    }
    for (int i = 0; i < TOTAL_BOTOES; i++) {
        if (mouseSobre(BOTOES[i].area)) {
            executarAcao(jogo, BOTOES[i].acao);
            return;
        }
    }
}

// teclas 1 a 5 escolhem a carta direto
void tratarTeclasCartas() {
    for (int i = 0; i < TOTAL_CARTAS; i++) {
        if (IsKeyPressed(KEY_ONE + i)) {
            tela.cartaSelecionada = (tela.cartaSelecionada == i) ? -1 : i;
        }
    }
}

void atualizarTelaReino(Jogo& jogo) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        tela.cartaSelecionada = -1;
        tela.painelRecursos = false;
    }
    if (tela.tempoTransicao > 0) return;   // espera o "Turno X" sumir

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) cliqueDireito(jogo.reino);
    tratarTeclasCartas();
    if (IsKeyPressed(KEY_SPACE)) executarAcao(jogo, 6);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) tratarClique(jogo);
}

}  // fim do namespace anônimo

void abrirJanela() {
    InitWindow(VL * ESCALA, VA * ESCALA, "Kingdoms: Ashes of the Fallen");
    SetExitKey(KEY_NULL);   // ESC cancela a carta, não fecha o jogo
    SetTargetFPS(60);
    alvo = LoadRenderTexture(VL, VA);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_POINT);
    for (Brasa& b : tela.brasas) reiniciarBrasa(b, true);
}

bool janelaAberta() {
    return !WindowShouldClose();
}

void fecharJanela() {
    UnloadRenderTexture(alvo);
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
    BeginTextureMode(alvo);
    if (tela.atual == TELA_TITULO) {
        desenharTelaTitulo();
    } else {
        desenharTelaReino(jogo);
    }
    EndTextureMode();

    // a textura vem de cabeça pra baixo, por isso a altura negativa
    BeginDrawing();
    Rectangle origem = {0, 0, static_cast<float>(VL), -static_cast<float>(VA)};
    Rectangle destino = {0, 0, static_cast<float>(VL * ESCALA), static_cast<float>(VA * ESCALA)};
    DrawTexturePro(alvo.texture, origem, destino, {0, 0}, 0.0f, WHITE);
    EndDrawing();
}
