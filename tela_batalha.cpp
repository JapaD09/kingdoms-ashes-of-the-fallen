#include "tela_batalha.h"
#include "figuras.h"
#include "fonte.h"
#include "inimigos.h"
#include "paleta.h"
#include "pincel.h"
#include "sprites.h"
#include "ui.h"
#include <cmath>

using namespace std;

namespace {

enum Fase { CHEGANDO, LUTANDO, FIM };

const float TEMPO_CHEGADA = 1.8f;
const float TEMPO_RODADA = 0.75f;
const float MOMENTO_DO_GOLPE = 0.45f;   // quando o golpe acerta, dentro da rodada
const int CASTELO_X = 22;
const int CHAO_Y = 60;

struct EstadoBatalha {
    int fase = CHEGANDO;
    float tempoFase = 0;
    float tempoRodada = 0;
    bool rapido = false;
    int vidaAntes[MAX_LUTADORES];      // pra barra de vida cair na hora do golpe
    int x[MAX_LUTADORES];
    int y[MAX_LUTADORES];
    int ordem[MAX_LUTADORES];          // ordem de desenho (de cima pra baixo)
    float tremor = 0;
};

EstadoBatalha anim;

const Rectangle BOTAO_RAPIDO = {246, 165, 30, 12};
const Rectangle BOTAO_PULAR = {280, 165, 36, 12};
const Rectangle BOTAO_CONTINUAR = {125, 136, 70, 16};

// posições: corpo a corpo na frente, quem atira atrás.
// aliados à esquerda olhando pra direita, inimigos ao contrário
void posicionarLado(const Batalha& b, int lado) {
    int vaga = 0;
    for (int passada = 0; passada < 2; passada++) {
        for (int i = 0; i < b.total; i++) {
            const Lutador& l = b.lutadores[i];
            if (l.lado != lado || l.distancia != (passada == 1)) continue;
            if (l.chefe) {                   // o chefe fica no meio, um pouco atrás
                anim.x[i] = 160 + 50;
                anim.y[i] = 126;
                continue;
            }
            int coluna = vaga / 5, linha = vaga % 5;
            int distanciaDoMeio = 18 + coluna * 20;
            anim.x[i] = lado == ALIADO ? 160 - distanciaDoMeio : 160 + distanciaDoMeio;
            anim.y[i] = 84 + linha * 16 + (coluna % 2) * 8;
            vaga++;
        }
    }
}

// insertion sort pelo y, pra quem está na frente cobrir quem está atrás
void ordenarParaDesenhar(const Batalha& b) {
    for (int i = 0; i < b.total; i++) anim.ordem[i] = i;
    for (int i = 1; i < b.total; i++) {
        int atual = anim.ordem[i];
        int j = i - 1;
        while (j >= 0 && anim.y[anim.ordem[j]] > anim.y[atual]) {
            anim.ordem[j + 1] = anim.ordem[j];
            j--;
        }
        anim.ordem[j + 1] = atual;
    }
}

void guardarVidas(const Batalha& b) {
    for (int i = 0; i < b.total; i++) anim.vidaAntes[i] = b.lutadores[i].vida;
}

void proximaRodada(Batalha& b) {
    guardarVidas(b);
    rodadaDeBatalha(b);
    anim.tempoRodada = 0;
    for (int g = 0; g < b.totalGolpes; g++) {
        if (b.golpes[g].alvo < 0) anim.tremor = 0.35f;   // castelo apanhou
    }
}

void pularTudo(Batalha& b) {
    while (!b.acabou) rodadaDeBatalha(b);
    b.totalGolpes = 0;
    guardarVidas(b);
    anim.fase = FIM;
}

// na batalha todo mundo aparece maior (os grandões um pouco menos, senão não cabem)
Aparencia aparenciaDe(const Batalha& b, const Lutador& l) {
    Aparencia a;
    if (l.lado == INIMIGO) a = aparenciaInimigo(b.reino, l.nivelInimigo);
    else if (l.heroi >= 0) a = aparenciaHeroi(l.heroi);
    else a = aparenciaTropa(l.tropa);
    a.tamanho = a.tamanho == 1 ? 2 : 3;
    if (a.especial == DRAGAO) a.tamanho = 2;      // o dragão já é largo
    return a;
}

// quanto da rodada já passou (0 a 1)
float progresso() {
    return anim.tempoRodada / TEMPO_RODADA;
}

bool golpeJaAcertou() {
    return anim.fase == FIM || progresso() >= MOMENTO_DO_GOLPE / TEMPO_RODADA;
}

// onde o golpe vai (lutador ou castelo)
Vector2 pontoDoAlvo(const Golpe& g) {
    if (g.alvo < 0) return {static_cast<float>(CASTELO_X), 100};
    return {static_cast<float>(anim.x[g.alvo]), static_cast<float>(anim.y[g.alvo] - 12)};
}

// avanço do corpo a corpo: vai até perto do alvo e volta
Vector2 deslocamento(const Batalha& b, int i) {
    Vector2 d = {0, 0};
    if (anim.fase != LUTANDO) return d;
    const Lutador& l = b.lutadores[i];
    for (int g = 0; g < b.totalGolpes; g++) {
        const Golpe& golpe = b.golpes[g];
        if (golpe.atacante != i || golpe.dano < 0 || l.distancia) continue;
        float t = progresso();
        float ida = t < 0.6f ? sinf(t / 0.6f * PI) : 0;
        Vector2 alvo = pontoDoAlvo(golpe);
        d.x = (alvo.x - anim.x[i]) * 0.45f * ida;
        d.y = (alvo.y + 12 - anim.y[i]) * 0.45f * ida;
        break;
    }
    return d;
}

// levou golpe nesta rodada? fica branco um instante
float brilhoDoGolpe(const Batalha& b, int i) {
    if (anim.fase != LUTANDO || !golpeJaAcertou()) return 0;
    for (int g = 0; g < b.totalGolpes; g++) {
        if (b.golpes[g].alvo == i && b.golpes[g].dano > 0) {
            float t = (anim.tempoRodada - MOMENTO_DO_GOLPE) / 0.15f;
            return t < 1 ? 1 - t : 0;
        }
    }
    return 0;
}

int vidaMostrada(const Batalha& b, int i) {
    return golpeJaAcertou() ? b.lutadores[i].vida : anim.vidaAntes[i];
}

// conta pelo que está na tela, não pelo que já foi calculado
int vivosNaTela(const Batalha& b, int lado) {
    int total = 0;
    for (int i = 0; i < b.total; i++) {
        if (b.lutadores[i].lado == lado && vidaMostrada(b, i) > 0) total++;
    }
    return total;
}

// fundo

void desenharCampo(const Batalha& b) {
    DrawRectangleGradientV(0, 0, VL, CHAO_Y, Color{110, 150, 196, 255}, Color{196, 214, 222, 255});
    if (b.reino == 5) DrawRectangleGradientV(0, 0, VL, CHAO_Y, Fade(Color{120, 50, 40, 255}, 0.5f), Fade(BRASA, 0.2f));
    for (int i = 0; i < 7; i++) {                       // morros lá longe
        int x = i * 52 - 20, alto = 8 + (i * 7) % 9;
        DrawRectangle(x + 6, CHAO_Y - alto - 3, 40, 4, Color{126, 156, 128, 255});
        DrawRectangle(x, CHAO_Y - alto, 52, alto, Color{120, 150, 120, 255});
    }
    for (int i = 0; i < 26; i++) {                      // fileira de árvores
        int x = 30 + i * 11 + (i * 5) % 7, alto = 6 + (i * 3) % 5;
        DrawRectangle(x, CHAO_Y - alto, 7, alto, i % 3 ? FOLHA : FOLHA_CLARA);
        DrawRectangle(x + 1, CHAO_Y - alto - 2, 5, 2, i % 3 ? FOLHA : FOLHA_CLARA);
    }
    DrawRectangle(0, CHAO_Y, VL, VA - CHAO_Y, GRAMA);
    DrawRectangle(0, CHAO_Y, VL, 3, GRAMA_ESCURA);
    for (int i = 0; i < 70; i++) {                      // tufos
        int tx = (i * 53) % VL, ty = CHAO_Y + 6 + (i * 31) % 96;
        ret(tx, ty, 0.5f, 1, i % 3 ? GRAMA_CLARA : GRAMA_ESCURA);
        ret(tx + 1, ty + 0.5f, 0.5f, 0.5f, i % 3 ? GRAMA_CLARA : GRAMA_ESCURA);
    }
}

void desenharMuralha() {
    int tremor = anim.tremor > 0 ? static_cast<int>(sinf(tempo() * 60) * 2) : 0;
    desenharComContorno(desenharCastelo, -12 + tremor, 14, 2);
    DrawRectangle(tremor, 58, CASTELO_X, 92, PEDRA);
    for (int fy = 60; fy < 150; fy += 4) {                    // tijolos finos da muralha
        ret(tremor, fy, CASTELO_X, 0.5f, Fade(CONTORNO, 0.2f));
        for (int fx = (fy / 4 % 2) * 4 + 2; fx < CASTELO_X; fx += 8) ret(tremor + fx, fy - 4, 0.5f, 4, Fade(CONTORNO, 0.2f));
    }
    DrawRectangle(tremor, 58, 4, 92, PEDRA_CLARA);
    for (int y = 64; y < 150; y += 8) DrawRectangle(tremor, y, CASTELO_X, 1, Fade(CONTORNO, 0.3f));
    for (int y = 58; y < 150; y += 12) DrawRectangle(CASTELO_X - 2 + tremor, y, 3, 6, PEDRA);
    DrawRectangle(CASTELO_X + tremor, 58, 1, 92, CONTORNO);
}

void desenharEstandartesInimigos(const Batalha& b) {
    for (int i = 0; i < 2; i++) {
        int x = 296 + i * 10, y = 66 + i * 30;
        desenharBandeira(x, y, b.reino, 2);
    }
}

// lutadores

void desenharLutador(const Batalha& b, int i, float entrada) {
    const Lutador& l = b.lutadores[i];
    Aparencia a = aparenciaDe(b, l);
    bool esquerda = l.lado == INIMIGO;
    Vector2 d = deslocamento(b, i);
    int chegada = static_cast<int>((1 - entrada) * 140) * (esquerda ? 1 : -1);
    float x = anim.x[i] + roundf(d.x * 2) / 2 + chegada;
    float y = anim.y[i] + roundf(d.y * 2) / 2;
    int vida = vidaMostrada(b, i);

    if (vida <= 0) {
        desenharFiguraCaida(x, y, a, esquerda);
        return;
    }
    int passo = entrada < 1 ? static_cast<int>(tempo() * 8) % 2 : static_cast<int>(tempo() * 2 + i) % 2 * (d.x != 0);
    ret(x - 3 * a.tamanho, y - 1, 7 * a.tamanho, 2, Fade(CONTORNO, 0.25f));   // sombra
    // parado, o boneco "respira": sobe meia unidade de vez em quando
    float respira = (entrada >= 1 && d.x == 0 && sinf(tempo() * 3 + i * 1.3f) > 0.3f) ? 0.5f : 0;
    desenharFigura(x, y - respira, a, esquerda, passo, brilhoDoGolpe(b, i));

    int altura = (a.especial == MONTADO ? 16 : a.especial == DRAGAO ? 14 : a.baixinho ? 10 : 12) * a.tamanho;
    int largura = l.chefe ? 20 : 10;
    Color cor = l.lado == ALIADO ? (l.heroi >= 0 ? DOURADO : FOLHA_CLARA) : TELHADO_VERM;
    if (entrada >= 1) barraDeVida(x - largura / 2, y - altura - 2, largura, vida, l.vidaMax, cor);
    if (l.heroi >= 0 && entrada >= 1) DrawRectangle(x, y - altura - 5, 1, 1, DOURADO);
    if (l.chefe && entrada >= 1) desenharTexto("CHEFE", x - 14, y - altura - 11, 1, TELHADO_VERM);
}

// flechas, magias e tiros da torre voando
void desenharProjeteis(const Batalha& b) {
    if (anim.fase != LUTANDO || golpeJaAcertou()) return;
    float t = anim.tempoRodada / MOMENTO_DO_GOLPE;
    for (int g = 0; g < b.totalGolpes; g++) {
        const Golpe& golpe = b.golpes[g];
        if (golpe.dano < 0) continue;
        bool torre = golpe.atacante < 0;
        if (!torre && !b.lutadores[golpe.atacante].distancia) continue;
        Vector2 de = torre ? Vector2{18, 40} : Vector2{static_cast<float>(anim.x[golpe.atacante]),
                                                      static_cast<float>(anim.y[golpe.atacante] - 16)};
        Vector2 para = pontoDoAlvo(golpe);
        float px = de.x + (para.x - de.x) * t;
        float py = de.y + (para.y - de.y) * t - sinf(t * PI) * 10;
        if (torre) {
            DrawRectangle(px - 2, py, 5, 1, PEDRA_CLARA);
            continue;
        }
        Aparencia a = aparenciaDe(b, b.lutadores[golpe.atacante]);
        if (a.arma == CAJADO) {
            DrawRectangle(px - 2, py - 2, 5, 5, Fade(a.brilhoArma, 0.5f));
            DrawRectangle(px - 1, py - 1, 3, 3, a.brilhoArma);
        } else {
            int sentido = para.x > de.x ? 1 : -1;
            DrawRectangle(px - (sentido > 0 ? 5 : 0), py, 5, 1, MADEIRA);
            DrawRectangle(px + (sentido > 0 ? 0 : -1), py, 2, 1, PEDRA_CLARA);
        }
    }
}

// números de dano subindo
void desenharNumeros(const Batalha& b) {
    if (anim.fase != LUTANDO || !golpeJaAcertou()) return;
    float t = (anim.tempoRodada - MOMENTO_DO_GOLPE) / (TEMPO_RODADA - MOMENTO_DO_GOLPE);
    for (int g = 0; g < b.totalGolpes; g++) {
        const Golpe& golpe = b.golpes[g];
        Vector2 p = pontoDoAlvo(golpe);
        int y = static_cast<int>(p.y - 8 - t * 8);
        bool cura = golpe.dano < 0;
        const char* texto = TextFormat(cura ? "+%d" : "%d", cura ? -golpe.dano : golpe.dano);
        Color cor = cura ? GRAMA_CLARA : (golpe.alvo < 0 ? TELHADO_VERM : TEXTO_CLARO);
        int x = static_cast<int>(p.x) - larguraTexto(texto, 1) / 2 + (g % 3 - 1) * 3;
        desenharTexto(texto, x + 1, y + 1, 1, Fade(CONTORNO, 1 - t));
        desenharTexto(texto, x, y, 1, Fade(cor, 1 - t));
    }
}

// HUD

void desenharTitulo(const Batalha& b) {
    const ReinoInimigo& inimigo = reinoInimigo(b.reino);
    desenharFaixa(70, 3, 180, 16);
    textoCentralizado(TextFormat("Turno %d: %s", b.turno, inimigo.nome), 7, 1, TEXTO_CLARO);
    desenharTexto(TextFormat("Rodada %d", b.rodada), 268, 22, 1, CONTORNO);
}

void desenharChegada(const Batalha& b) {
    const ReinoInimigo& inimigo = reinoInimigo(b.reino);
    float t = anim.tempoFase / TEMPO_CHEGADA;
    float alpha = t < 0.75f ? 1.0f : (1 - t) * 4;
    caixa({60, 26, 200, 30}, Fade(CONTORNO, alpha), Fade(Color{40, 28, 24, 255}, alpha * 0.85f));
    textoCentralizado(TextFormat("%s ataca!", inimigo.rei), 30, 1, Fade(DOURADO, alpha));
    textoCentralizado(inimigo.lema, 42, 1, Fade(TEXTO_CLARO, alpha));
}

void desenharRodape(const Batalha& b, const Reino& reino) {
    DrawRectangle(0, 160, VL, 20, Fade(CONTORNO, 0.75f));
    iconeEspadas(4, 166, 1);
    desenharTexto(TextFormat("Exército %d", vivosNaTela(b, ALIADO)), 13, 166, 1, FOLHA_CLARA);
    iconeCoracao(78, 166, 1);
    int vida = reino.vidaCastelo - (golpeJaAcertou() ? b.danoNoCastelo : 0);
    if (vida < 0) vida = 0;
    barraDeVida(87, 168, 36, vida, VIDA_CASTELO, TELHADO_VERM);
    desenharTexto(TextFormat("%d", vida), 126, 166, 1, TEXTO_CLARO);
    desenharTexto(TextFormat("Inimigos %d", vivosNaTela(b, INIMIGO)), 150, 166, 1, CORACAO_CLARO);
    if (anim.fase != FIM) {
        botaozinho(BOTAO_RAPIDO, anim.rapido ? "2x" : "1x", anim.rapido);
        botaozinho(BOTAO_PULAR, "Pular", false);
    }
}

// "ouro +10  pedra +4  comida -6" grande demais vira duas linhas
string quebrarAoMeio(string texto) {
    if (larguraTexto(texto.c_str(), 1) < 160) return texto;
    size_t meio = texto.find("  ", texto.size() / 2 - 4);
    if (meio == string::npos) meio = texto.find("  ");
    if (meio != string::npos) texto.replace(meio, 2, "\n");
    return texto;
}

void desenharResultado(const Batalha& b, const string& resumoDoTurno) {
    DrawRectangle(0, 0, VL, VA, Fade(CONTORNO, 0.45f));
    caixaComSombra({70, 46, 180, 112}, PERGAMINHO);
    if (b.vitoria) {
        desenharTextoContorno("VITÓRIA!", (VL - larguraTexto("VITÓRIA!", 2)) / 2, 54, 2, BRILHO_OURO, DOURADO, CONTORNO);
        int ouro, cristais;
        recompensaDaBatalha(b, ouro, cristais);
        textoCentralizadoEm(TextFormat("+%d ouro   +%d cristais", ouro, cristais), VL / 2, 76, VERDE_TEXTO);
    } else {
        desenharTextoContorno("DERROTA", (VL - larguraTexto("DERROTA", 2)) / 2, 54, 2, CORACAO_CLARO, TELHADO_VERM, CONTORNO);
        textoCentralizadoEm(TextFormat("O castelo levou %d de dano.", b.danoNoCastelo), VL / 2, 76, CONTORNO);
    }
    int caidos = 0;
    for (int i = 0; i < b.total; i++) {
        if (b.lutadores[i].lado == INIMIGO && b.lutadores[i].vida <= 0) caidos++;
    }
    textoCentralizadoEm(TextFormat("Inimigos derrotados: %d", caidos), VL / 2, 88, MADEIRA_ESCURA);
    DrawRectangle(80, 100, 160, 1, PERGAMINHO_ESC);
    textoCentralizadoEm("Produção do turno:", VL / 2, 104, PERGAMINHO_ESC);
    string producao = quebrarAoMeio(resumoDoTurno);
    size_t quebra = producao.find('\n');
    if (quebra == string::npos) {
        textoCentralizadoEm(producao.c_str(), VL / 2, 114, MADEIRA_ESCURA);
    } else {
        textoCentralizadoEm(producao.substr(0, quebra).c_str(), VL / 2, 113, MADEIRA_ESCURA);
        textoCentralizadoEm(producao.substr(quebra + 1).c_str(), VL / 2, 122, MADEIRA_ESCURA);
    }
    botaoPrincipal(BOTAO_CONTINUAR, "Continuar");
}

}

void comecarAnimacaoBatalha(const Batalha& batalha) {
    anim = EstadoBatalha();
    posicionarLado(batalha, ALIADO);
    posicionarLado(batalha, INIMIGO);
    ordenarParaDesenhar(batalha);
    guardarVidas(batalha);
}

bool atualizarTelaBatalha(Batalha& b, float dt) {
    float passo = dt * (anim.rapido ? 2.5f : 1.0f);
    if (anim.tremor > 0) anim.tremor -= dt;

    if (anim.fase == FIM) {
        return clicouEm(BOTAO_CONTINUAR) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    }
    if (clicouEm(BOTAO_RAPIDO)) anim.rapido = !anim.rapido;
    if (clicouEm(BOTAO_PULAR) || IsKeyPressed(KEY_SPACE)) {
        pularTudo(b);
        return false;
    }
    anim.tempoFase += passo;
    if (anim.fase == CHEGANDO) {
        if (anim.tempoFase >= TEMPO_CHEGADA) {
            anim.fase = LUTANDO;
            proximaRodada(b);
        }
        return false;
    }
    anim.tempoRodada += passo;
    if (anim.tempoRodada >= TEMPO_RODADA) {
        if (b.acabou) anim.fase = FIM;
        else proximaRodada(b);
    }
    return false;
}

void desenharTelaBatalha(const Batalha& b, const Reino& reino, const string& resumoDoTurno) {
    desenharCampo(b);
    desenharMuralha();
    desenharEstandartesInimigos(b);

    float entrada = anim.fase == CHEGANDO ? fminf(anim.tempoFase / (TEMPO_CHEGADA * 0.7f), 1.0f) : 1.0f;
    for (int k = 0; k < b.total; k++) {             // caídos primeiro, ficam por baixo
        int i = anim.ordem[k];
        if (vidaMostrada(b, i) <= 0) desenharLutador(b, i, entrada);
    }
    for (int k = 0; k < b.total; k++) {
        int i = anim.ordem[k];
        if (vidaMostrada(b, i) > 0) desenharLutador(b, i, entrada);
    }
    desenharProjeteis(b);
    desenharNumeros(b);

    desenharTitulo(b);
    if (anim.fase == CHEGANDO) desenharChegada(b);
    desenharRodape(b, reino);
    if (anim.fase == FIM) desenharResultado(b, resumoDoTurno);
}
