#include "tela_herois.h"
#include "figuras.h"
#include "fonte.h"
#include "paleta.h"
#include "sprites.h"
#include "ui.h"
#include <cmath>

using namespace std;

namespace {

struct EstadoHerois {
    bool aberto = false;
    int revelando = -1;        // herói que acabou de sair
    float tempoRevelar = 0;
    bool novo = false;
};

EstadoHerois estado;

const Rectangle BOTAO_INVOCAR = {200, 46, 90, 18};
const Rectangle BOTAO_FECHAR = {250, 157, 44, 13};

Rectangle areaHeroi(int id) {
    return {26.0f + (id % 3) * 56, 36.0f + (id / 3) * 44, 52, 40};
}

Color corDaRaridade(int raridade) {
    switch (raridade) {
        case LENDARIO: return DOURADO;
        case EPICO: return ROXO;
        case RARO: return CRISTAL_ESC;
        default: return PEDRA;
    }
}

int heroiSobMouse() {
    for (int i = 0; i < TOTAL_HEROIS; i++) {
        if (mouseSobre(areaHeroi(i))) return i;
    }
    return -1;
}

void estrelinhas(int x, int y, int quantas) {
    for (int i = 0; i < quantas; i++) {
        DrawRectangle(x + i * 4, y + 1, 3, 1, DOURADO);
        DrawRectangle(x + i * 4 + 1, y, 1, 3, DOURADO);
    }
}

void desenharCartaHeroi(const Reino& reino, int id) {
    Rectangle r = areaHeroi(id);
    const ModeloHeroi& h = modeloHeroi(id);
    int estrelas = reino.herois.estrelas[id];
    bool sobre = mouseSobre(r);
    int x = r.x, y = r.y;

    caixa(r, corDaRaridade(h.raridade), estrelas > 0 ? PERGAMINHO_ESC : Color{70, 60, 56, 255});
    DrawRectangle(x + 2, y + 2, 48, 27, estrelas > 0 ? Color{120, 160, 200, 255} : Color{50, 44, 42, 255});
    DrawRectangle(x + 2, y + 23, 48, 6, estrelas > 0 ? GRAMA : Color{44, 40, 38, 255});
    if (estrelas == 0) {
        desenharTexto("?", x + 24, y + 11, 1, PERGAMINHO_ESC);
        desenharTexto(nomeRaridade(h.raridade), x + 3, y + 31, 1, Fade(PERGAMINHO, 0.5f));
        return;
    }
    int passo = static_cast<int>(tempo() * 2 + id) % 2;
    Aparencia a = aparenciaHeroi(id);
    a.tamanho = 2;
    desenharFigura(x + 24, y + 28, a, false, sobre ? passo : 0, 0);
    desenharTexto(h.nome, x + 3, y + 31, 1, CONTORNO);
    estrelinhas(x + 3, y + 3, estrelas);
    if (reino.herois.naEquipe[id]) {
        DrawRectangle(x + 40, y + 2, 10, 9, CONTORNO);
        iconeEspadas(x + 41, y + 3, 1);
    }
    if (sobre) DrawRectangleLinesEx({r.x - 1, r.y - 1, r.width + 2, r.height + 2}, 1, TEXTO_CLARO);
}

void desenharChances(const Reino& reino) {
    desenharTexto("Chances:", 200, 72, 1, MADEIRA_ESCURA);
    for (int r = LENDARIO; r >= COMUM; r--) {
        int y = 82 + (LENDARIO - r) * 9;
        DrawRectangle(200, y + 2, 3, 3, corDaRaridade(r));
        desenharTexto(TextFormat("%s %d%%", nomeRaridade(r), chanceDaRaridade(r)), 206, y, 1, CONTORNO);
    }
    int faltam = GARANTIA_EPICO - reino.herois.semEpico;
    desenharTexto(TextFormat("Épico garantido\nem %d invocações", faltam), 200, 122, 1, PERGAMINHO_ESC);
}

void desenharDetalhes(const Reino& reino, int id) {
    const ModeloHeroi& h = modeloHeroi(id);
    desenharTexto(h.nome, 200, 72, 1, VERMELHO_ESC);
    desenharTexto(h.titulo, 200, 81, 1, MADEIRA_ESCURA);
    desenharTexto(nomeRaridade(h.raridade), 200, 90, 1, corDaRaridade(h.raridade));
    if (reino.herois.estrelas[id] == 0) {
        desenharTexto("Ainda não\napareceu.", 200, 102, 1, PERGAMINHO_ESC);
        return;
    }
    desenharTexto(TextFormat("Vida %d  Atq %d", vidaDoHeroi(reino.herois, id), ataqueDoHeroi(reino.herois, id)),
                  200, 101, 1, CONTORNO);
    desenharTexto(TextFormat("Def %d  %s", h.defesa, h.distancia ? "de longe" : "corpo a corpo"), 200, 110, 1, CONTORNO);
    desenharTexto(h.descricao, 200, 121, 1, MADEIRA_ESCURA);
    desenharTexto(reino.herois.naEquipe[id] ? "Clique: tirar" : "Clique: escalar", 200, 142, 1, PERGAMINHO_ESC);
}

// raios girando atrás do herói que saiu
void desenharRaios(int cx, int cy, Color cor) {
    float giro = tempo() * 0.8f;
    for (int i = 0; i < 12; i++) {
        float ang = giro + i * PI / 6;
        Vector2 a = {cx + cosf(ang) * 8, cy + sinf(ang) * 8};
        Vector2 b = {cx + cosf(ang) * 70, cy + sinf(ang) * 70};
        DrawLineEx(a, b, 3, Fade(cor, 0.35f));
    }
}

void desenharRevelacao() {
    int id = estado.revelando;
    const ModeloHeroi& h = modeloHeroi(id);
    Color cor = corDaRaridade(h.raridade);
    float t = estado.tempoRevelar;

    DrawRectangle(0, 0, VL, VA, Fade(CONTORNO, 0.9f));
    desenharRaios(160, 82, cor);
    if (t < 0.25f) DrawRectangle(0, 0, VL, VA, Fade(WHITE, 1.0f - t * 4));   // clarão

    Aparencia a = aparenciaHeroi(id);
    a.tamanho = 4;
    int pula = static_cast<int>(sinf(tempo() * 3) * 2);
    desenharFigura(160, 108 + pula, a, false, 0, t < 0.6f ? 1.0f - t / 0.6f : 0);

    textoCentralizado(nomeRaridade(h.raridade), 8, 2, cor);
    desenharFaixa(90, 114, 140, 18);
    textoCentralizado(TextFormat("%s, %s", h.nome, h.titulo), 119, 1, TEXTO_CLARO);
    textoCentralizado(estado.novo ? "Novo herói!" : "Repetido: +1 estrela", 136, 1, estado.novo ? DOURADO : CRISTAL);
    textoCentralizado(h.descricao, 146, 1, PERGAMINHO);
    if (t > 1.0f) textoCentralizado("clique para continuar", 170, 1, Fade(TEXTO_CLARO, 0.6f));
}

string trocarEquipe(Reino& reino, int id) {
    string problema = alternarEquipe(reino.herois, id);
    if (!problema.empty()) return problema;
    const char* nome = modeloHeroi(id).nome;
    return reino.herois.naEquipe[id] ? string(nome) + " entrou na equipe." : string(nome) + " saiu da equipe.";
}

}

void abrirHerois() {
    estado = EstadoHerois();
    estado.aberto = true;
}

bool heroisAbertos() {
    return estado.aberto;
}

string atualizarHerois(Reino& reino) {
    if (estado.revelando >= 0) {
        estado.tempoRevelar += GetFrameTime();
        bool fechar = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
        if (fechar && estado.tempoRevelar > 0.6f) estado.revelando = -1;
        return "";
    }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_H) || clicouEm(BOTAO_FECHAR)) {
        estado.aberto = false;
        return "";
    }
    if (clicouEm(BOTAO_INVOCAR) || IsKeyPressed(KEY_I)) {
        int heroi;
        string texto = invocar(reino, heroi);
        if (heroi < 0) return texto;
        estado.revelando = heroi;
        estado.tempoRevelar = 0;
        estado.novo = reino.herois.estrelas[heroi] == 1;
        return "";
    }
    int id = heroiSobMouse();
    if (id >= 0 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return trocarEquipe(reino, id);
    return "";
}

void desenharHerois(const Reino& reino) {
    DrawRectangle(0, 0, VL, VA, Fade(CONTORNO, 0.5f));
    caixaComSombra({20, 22, 280, 152}, PERGAMINHO);
    desenharTexto("Heróis", 26, 26, 1, VERMELHO_ESC);
    desenharTexto(TextFormat("equipe %d/%d", tamanhoDaEquipe(reino.herois), MAX_EQUIPE), 70, 26, 1, PERGAMINHO_ESC);
    DrawRectangle(194, 26, 1, 144, PERGAMINHO_ESC);
    for (int i = 0; i < TOTAL_HEROIS; i++) desenharCartaHeroi(reino, i);

    iconeCristal(200, 33, 1);
    desenharTexto(TextFormat("%d cristais", reino.cristais), 210, 33, 1, CRISTAL_ESC);
    botaoPrincipal(BOTAO_INVOCAR, TextFormat("Invocar (%d)", CUSTO_INVOCACAO));
    int id = heroiSobMouse();
    if (id >= 0) desenharDetalhes(reino, id);
    else desenharChances(reino);
    botaozinho(BOTAO_FECHAR, "Fechar", false);

    if (estado.revelando >= 0) desenharRevelacao();
}
