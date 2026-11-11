#include "natureza.h"
#include "paleta.h"
#include "pincel.h"
#include <cmath>

namespace {

// cores da natureza
const Color CAMPO        = {66, 108, 64, 255};
const Color CAMPO_ESCURO = {56, 94, 58, 255};
const Color CAMPO_CLARO  = {80, 122, 70, 255};
const Color TRONCO       = {92, 62, 42, 255};
const Color TRONCO_SECO  = {104, 88, 76, 255};
const Color BETULA_CASCA = {226, 222, 208, 255};
const Color AGUA         = {52, 98, 142, 255};
const Color AGUA_CLARA   = {120, 170, 204, 255};
const Color MARGEM       = {44, 76, 52, 255};
const Color FUMACA       = {206, 200, 192, 255};

struct TomFolha {
    Color escuro, medio, claro;
};

const TomFolha TONS[] = {
    {{30, 78, 50, 255}, {48, 110, 60, 255}, {84, 148, 72, 255}},     // verde
    {{26, 70, 62, 255}, {40, 100, 82, 255}, {70, 138, 104, 255}},    // verde-azulado
    {{78, 82, 38, 255}, {120, 116, 50, 255}, {164, 152, 68, 255}},   // oliva
    {{112, 54, 34, 255}, {164, 88, 42, 255}, {210, 134, 62, 255}},   // outono
};

const Color CORES_FLOR[] = {
    {236, 206, 92, 255}, {230, 228, 220, 255}, {226, 128, 160, 255},
    {170, 140, 220, 255}, {214, 78, 64, 255},
};

// sorteio com semente fixa: o cenário sai igual toda vez que abre o jogo
unsigned estado = 1;

int sortear(int minimo, int maximo) {
    estado = estado * 1103515245u + 12345u;
    return minimo + static_cast<int>((estado >> 16) % static_cast<unsigned>(maximo - minimo + 1));
}

void px(float x, float y, float w, float h, Color cor) {
    ret(x, y, w, h, cor);
}

// quanto a linha da copa anda com o vento (o topo anda mais), de meia em meia unidade
float desvio(float linha, float altura, float balanco) {
    return roundf(balanco * (altura - linha) / altura * 2) / 2;
}

// árvores

void pinheiro(int x, int y, int tamanho, const TomFolha& t, float balanco) {
    int altura = 8 + tamanho * 3;
    float topo = y - 2 - altura;
    px(x - 2 - tamanho, y, 5 + 2 * tamanho, 1, Fade(CONTORNO, 0.25f));
    px(x, y - 2, 1, 2, TRONCO);
    px(x, y - 2, 0.5f, 2, Fade(CONTORNO, 0.3f));
    for (float l = 0; l < altura; l += 0.5f) {      // cada "saia" abre aos poucos e fecha de uma vez
        float naSaia = fmodf(l, 3);
        float meia = naSaia * 0.8f + floorf(l / 3) * 0.9f + 0.5f;
        meia = roundf(meia * 2) / 2;
        float cx = x + 0.5f + desvio(l, altura, balanco);
        px(cx - meia, topo + l, 2 * meia, 0.5f, naSaia >= 2 ? t.escuro : t.medio);
        px(cx - meia, topo + l, fminf(meia * 0.6f, 1.5f), 0.5f, t.claro);      // lado do sol
        if (meia >= 1) px(cx + meia - 0.5f, topo + l, 0.5f, 0.5f, t.escuro);
        if (naSaia == 2.5f) px(cx - meia + 1, topo + l, 2 * meia - 1.5f, 0.5f, Fade(CONTORNO, 0.25f));
    }
}

// copa redonda (carvalho, bétula e arbusto usam)
void copaRedonda(int cx, int cy, int raio, const TomFolha& t, float balanco) {
    float r = raio + 0.5f;
    for (float dy = -r; dy < r; dy += 0.5f) {
        float meia = roundf(sqrtf(r * r - (dy + 0.25f) * (dy + 0.25f)) * 2) / 2;
        float x = cx + 0.5f + desvio(dy + r, 2 * r, balanco);
        Color cor = (dy < -r * 0.4f) ? t.claro : (dy >= r * 0.45f) ? t.escuro : t.medio;
        px(x - meia, cy + dy, 2 * meia, 0.5f, cor);
        if (dy >= -r * 0.4f) px(x - meia, cy + dy, fminf(meia * 0.5f, 1.5f), 0.5f, dy < r * 0.45f ? t.claro : t.medio);
        if (dy < r * 0.45f) px(x + meia - fminf(meia * 0.4f, 1), cy + dy, fminf(meia * 0.4f, 1), 0.5f, dy < -r * 0.4f ? t.medio : t.escuro);
    }
    // tufos de folha: sempre nos mesmos lugares pra copa não "ferver"
    for (int i = 0; i < raio * 3; i++) {
        float ang = i * 2.4f, dist = (0.25f + (i % 4) * 0.2f) * raio;
        float fx = roundf((cx + cosf(ang) * dist) * 2) / 2, fy = roundf((cy + sinf(ang) * dist * 0.9f) * 2) / 2;
        px(fx, fy, 1, 0.5f, i % 2 ? t.escuro : t.claro);
        px(fx + 0.5f, fy + 0.5f, 0.5f, 0.5f, i % 2 ? t.escuro : t.claro);
    }
}

void carvalho(int x, int y, int tamanho, const TomFolha& t, float balanco) {
    int raio = 3 + tamanho;
    int tronco = 2 + tamanho / 2;
    px(x - raio, y, 2 * raio + 1, 1, Fade(CONTORNO, 0.25f));
    px(x, y - tronco, 1 + tamanho / 2, tronco, TRONCO);
    px(x, y - tronco, 0.5f, tronco, Fade(CONTORNO, 0.3f));
    copaRedonda(x, y - tronco - raio, raio, t, balanco);
}

void betula(int x, int y, int tamanho, const TomFolha& t, float balanco) {
    int tronco = 4 + tamanho * 2;
    int raio = 2 + tamanho;
    px(x - raio, y, 2 * raio + 1, 1, Fade(CONTORNO, 0.2f));
    px(x, y - tronco, 1, tronco, BETULA_CASCA);
    for (int i = 1; i < tronco; i += 2) px(x + (i % 4 == 1 ? 0 : 0.5f), y - i, 0.5f, 0.5f, CONTORNO);   // manchas da casca
    copaRedonda(x, y - tronco - raio + 1, raio, t, balanco);
}

void arbusto(int x, int y, int tamanho, const TomFolha& t, float balanco) {
    int raio = 2 + tamanho;
    copaRedonda(x, y - raio, raio, t, balanco * 0.4f);
    if (tamanho == 1) {
        px(x - 1, y - raio, 1, 1, {196, 52, 52, 255});     // frutinhas
        px(x + 2, y - raio - 1, 1, 1, {196, 52, 52, 255});
    }
}

void seca(int x, int y, int tamanho) {
    int altura = 6 + tamanho * 2;
    px(x - 2, y, 5, 1, Fade(CONTORNO, 0.2f));
    px(x, y - altura, 1, altura, TRONCO_SECO);
    px(x - 1, y - altura + 3, 1, 1, TRONCO_SECO);       // galhos
    px(x - 2, y - altura + 2, 1, 1, TRONCO_SECO);
    px(x + 1, y - altura + 1, 1, 1, TRONCO_SECO);
    px(x + 2, y - altura, 1, 1, TRONCO_SECO);
    px(x + 1, y - altura / 2, 1, 1, TRONCO_SECO);
}

// enfeites do chão

void flores(int x, int y, int tamanho, int tom) {
    Color cor = CORES_FLOR[tom % 5];
    for (int i = 0; i <= tamanho + 1; i++) {
        int fx = x + (i * 3) % 5 - 2, fy = y - (i * 2) % 3;
        px(fx, fy, 1, 1, TONS[0].medio);
        px(fx, fy - 1, 1, 1, cor);
    }
}

void pedrinha(int x, int y, int tamanho) {
    int w = 2 + tamanho, h = 1 + tamanho / 2;
    px(x, y - h, w, h + 1, PEDRA);
    px(x, y - h, w - 1, 1, PEDRA_CLARA);
    px(x + 1, y + 1, w, 1, Fade(CONTORNO, 0.25f));
}

void cogumelo(int x, int y) {
    px(x, y - 1, 1, 2, FLOR_BRANCA);
    px(x - 1, y - 2, 3, 1, TELHADO_VERM);
    px(x, y - 2, 1, 1, FLOR_BRANCA);
}

void tufo(int x, int y, float balanco) {
    int d = balanco > 0.6f ? 1 : 0;
    px(x, y - 1, 1, 1, CAMPO_CLARO);
    px(x + 1 + d, y - 2, 1, 2, CAMPO_CLARO);
    px(x + 2, y - 1, 1, 1, CAMPO_CLARO);
}

void toco(int x, int y) {
    px(x, y - 2, 3, 3, TRONCO);
    px(x, y - 2, 3, 1, MADEIRA);
    px(x + 1, y - 2, 1, 1, PERGAMINHO_ESC);
}

void mancha(int x, int y, int tamanho, int tom) {
    Color cor = tom % 2 ? CAMPO_ESCURO : CAMPO_CLARO;
    int w = 10 + tamanho * 8, h = 3 + tamanho * 2;
    px(x - w / 2, y, w, h, cor);
    px(x - w / 2 + 2, y - 1, w - 4, h + 2, cor);
}

// riacho

float riachoEm(const Cenario& c, int y) {
    return c.riachoX + sinf(y * 0.045f) * 5 + sinf(y * 0.13f) * 1.5f;
}

void desenharRiacho(const Cenario& c, float tempo) {
    if (c.riachoX < 0) return;
    int brilho = static_cast<int>(tempo * 14);
    for (int y = 0; y < c.altura; y++) {
        for (int meio = 0; meio < 2; meio++) {       // duas meias linhas: a margem fica mais lisa
            float fy = y + meio * 0.5f;
            float x = roundf((riachoEm(c, y) * (2 - meio) + riachoEm(c, y + 1) * meio)) / 2;
            px(x - 1, fy, 6, 0.5f, MARGEM);
            px(x, fy, 4, 0.5f, AGUA);
            px(x, fy, 0.5f, 0.5f, AGUA_CLARA);       // espuma na beira
        }
        float x = roundf(riachoEm(c, y) * 2) / 2;
        if ((y + brilho) % 9 == 0) px(x + 1, y, 1.5f, 0.5f, AGUA_CLARA);
        if ((y + brilho / 2) % 13 == 0) px(x + 2, y, 1, 0.5f, AGUA_CLARA);
        if ((y * 7 + brilho / 3) % 17 == 0) px(x + 2.5f, y + 0.5f, 0.5f, 0.5f, TEXTO_CLARO);
    }
}

// geração

bool lugarLivre(const Cenario& c, Rectangle proibido, int x, int y) {
    if (x < -4 || y < 0 || x > c.largura + 4 || y > c.altura + 8) return false;
    if (CheckCollisionPointRec({static_cast<float>(x), static_cast<float>(y)}, proibido)) return false;
    return c.riachoX < 0 || fabsf(x - riachoEm(c, y)) > 6;
}

void adicionar(Cenario& c, int x, int y, int tipo, int tamanho, int tom) {
    if (c.total >= MAX_OBJETOS) return;
    c.objetos[c.total++] = {x, y, tipo, tamanho, tom, sortear(0, 628) / 100.0f};
}

int tomDaArvore(int tipo) {
    switch (tipo) {
        case PINHEIRO: return sortear(0, 1);
        case CARVALHO: return sortear(0, 6) == 0 ? 3 : sortear(0, 2);
        case BETULA:   return sortear(0, 1) * 2;
        default:       return sortear(0, 2);
    }
}

// bosque: árvores parecidas juntas, como numa floresta de verdade
void adicionarBosque(Cenario& c, Rectangle proibido) {
    int cx = sortear(0, c.largura), cy = sortear(10, c.altura);
    int principal = sortear(0, 2);           // pinheiro, carvalho ou bétula
    int quantas = sortear(5, 11);
    for (int i = 0; i < quantas; i++) {
        int x = cx + sortear(-16, 16), y = cy + sortear(-10, 10);
        if (!lugarLivre(c, proibido, x, y)) continue;
        int tipo = sortear(0, 4) == 0 ? sortear(0, 4) : principal;
        if (tipo == SECA && sortear(0, 2) != 0) tipo = ARBUSTO;
        adicionar(c, x, y, tipo, sortear(0, 2), tomDaArvore(tipo));
    }
}

void adicionarEnfeites(Cenario& c, Rectangle proibido, int quantos) {
    for (int i = 0; i < quantos; i++) {
        int x = sortear(0, c.largura), y = sortear(2, c.altura);
        if (!lugarLivre(c, proibido, x, y)) continue;
        int tipo = FLOR + sortear(0, 4);
        if (tipo == COGUMELO && sortear(0, 1)) tipo = FLOR;
        adicionar(c, x, y, tipo, sortear(0, 2), sortear(0, 4));
    }
}

// ordena pela altura (y): quem está mais embaixo é desenhado por cima
void ordenarPorY(Cenario& c) {
    for (int i = 1; i < c.total; i++) {
        ObjetoCenario atual = c.objetos[i];
        int j = i - 1;
        while (j >= 0 && c.objetos[j].y > atual.y) {
            c.objetos[j + 1] = c.objetos[j];
            j--;
        }
        c.objetos[j + 1] = atual;
    }
}

void desenharEnfeite(const ObjetoCenario& o, float tempo) {
    switch (o.tipo) {
        case FLOR:     flores(o.x, o.y, o.tamanho, o.tom); break;
        case PEDRINHA: pedrinha(o.x, o.y, o.tamanho); break;
        case COGUMELO: cogumelo(o.x, o.y); break;
        case TUFO:     tufo(o.x, o.y, ventoNaArvore(tempo, o.fase)); break;
        case TOCO:     toco(o.x, o.y); break;
        default: break;
    }
}

}

// árvores (públicas, o reino usa nos lotes de floresta)

float ventoNaArvore(float tempo, float fase) {
    float rajada = 0.6f + 0.6f * sinf(tempo * 0.35f + fase * 0.2f);
    return sinf(tempo * 1.6f + fase) * rajada;
}

static void arvoreSemContorno(int x, int y, int tipo, int tamanho, int tom, float balanco) {
    const TomFolha& t = TONS[tom % 4];
    switch (tipo) {
        case PINHEIRO: pinheiro(x, y, tamanho, t, balanco); break;
        case CARVALHO: carvalho(x, y, tamanho, t, balanco); break;
        case BETULA:   betula(x, y, tamanho, t, balanco); break;
        case ARBUSTO:  arbusto(x, y, tamanho, t, balanco); break;
        default:       seca(x, y, tamanho); break;
    }
}

void desenharArvoreVariada(int x, int y, int tipo, int tamanho, int tom, float balanco) {
    const Color borda = {26, 40, 36, 255};
    for (int passo = 0; passo < PASSOS_CONTORNO; passo++) {
        passoDoContorno(passo, borda);
        arvoreSemContorno(x, y, tipo, tamanho, tom, balanco);
    }
}

void gerarCenario(Cenario& c, Rectangle proibido, int largura, int altura,
                  float riachoX, unsigned semente) {
    estado = semente;
    c.total = 0;
    c.largura = largura;
    c.altura = altura;
    c.riachoX = riachoX;
    for (int i = 0; i < 26; i++) {
        adicionar(c, sortear(0, largura), sortear(0, altura), MANCHA, sortear(0, 2), sortear(0, 1));
    }
    for (int i = 0; i < 24; i++) adicionarBosque(c, proibido);
    adicionarEnfeites(c, proibido, 110);
    ordenarPorY(c);
}

static void desenharFundoCampo(const Cenario& c, float tempo) {
    ClearBackground(CAMPO);
    for (int i = 0; i < c.total; i++) {
        const ObjetoCenario& o = c.objetos[i];
        if (o.tipo == MANCHA) mancha(o.x, o.y, o.tamanho, o.tom);
    }
    desenharRiacho(c, tempo);
}

void desenharCenario(const Cenario& c, float tempo) {
    desenharFundoCampo(c, tempo);
    for (int i = 0; i < c.total; i++) {
        const ObjetoCenario& o = c.objetos[i];
        if (o.tipo == MANCHA) continue;
        if (o.tipo < FLOR) {
            desenharArvoreVariada(o.x, o.y, o.tipo, o.tamanho, o.tom, ventoNaArvore(tempo, o.fase));
        } else {
            desenharEnfeite(o, tempo);
        }
    }
}

void desenharChaoCenario(const Cenario& c, float tempo) {
    desenharFundoCampo(c, tempo);
    for (int i = 0; i < c.total; i++) {
        const ObjetoCenario& o = c.objetos[i];
        if (o.tipo > FLOR - 1 && o.tipo != MANCHA) desenharEnfeite(o, tempo);
    }
}

void arvoreDoLote(int semente, int i, int& dx, int& dy, int& tipo, int& tamanho, int& tom) {
    const int pos[3][2] = {{5, 11}, {15, 14}, {9, 21}};
    dx = pos[i][0];
    dy = pos[i][1];
    tipo = (semente + i * 7) % 4;
    tamanho = (semente + i) % 2;
    tom = (semente * 3 + i) % 3;
}

// fumaça

namespace {

struct Fumaca {
    float x, y, origemY, vida, tamanho;
};

Fumaca fumacas[80] = {};

}

void soltarFumaca(float x, float y) {
    for (Fumaca& f : fumacas) {
        if (f.vida > 0) continue;
        f = {x + GetRandomValue(-1, 1) * 0.5f, y, y, 2.6f, 1.5f};
        return;
    }
}

void atualizarFumaca(float dt) {
    for (Fumaca& f : fumacas) {
        if (f.vida <= 0) continue;
        f.vida -= dt;
        f.y -= 5 * dt;
        f.x += 2.5f * dt;             // vento leva pro lado
        f.tamanho += 0.8f * dt;
    }
}

int obterFumacas(EstadoFumaca saida[], int maximo) {
    int total = 0;
    for (const Fumaca& f : fumacas) {
        if (f.vida <= 0 || total >= maximo) continue;
        saida[total++] = {f.x, f.y, f.origemY, f.vida, f.tamanho};
    }
    return total;
}

void desenharFumaca() {
    for (const Fumaca& f : fumacas) {
        if (f.vida <= 0) continue;
        float lado = roundf(f.tamanho * 2) / 2;
        ret(roundf(f.x * 2) / 2, roundf(f.y * 2) / 2, lado, lado, Fade(FUMACA, f.vida / 2.6f * 0.85f));
        ret(roundf(f.x * 2) / 2, roundf(f.y * 2) / 2, lado / 2, 0.5f, Fade(TEXTO_CLARO, f.vida / 2.6f * 0.3f));
    }
}

// galinhas

namespace {

struct Galinha {
    float x, y, alvoX, alvoY, espera;
    bool bicando;
    int direcao;
};

Galinha galinhas[10];
int totalGalinhas = 0;
Rectangle areaGalinhas;
bool (*galinhaPodeAndar)(float, float) = nullptr;

void novoDestino(Galinha& g) {
    for (int tentativa = 0; tentativa < 8; tentativa++) {
        float x = areaGalinhas.x + GetRandomValue(0, static_cast<int>(areaGalinhas.width));
        float y = areaGalinhas.y + GetRandomValue(0, static_cast<int>(areaGalinhas.height));
        if (galinhaPodeAndar == nullptr || galinhaPodeAndar(x, y)) {
            g.alvoX = x;
            g.alvoY = y;
            return;
        }
    }
}

EstadoGalinha estadoDaGalinha(const Galinha& g, float tempo) {
    bool andando = g.espera <= 0;
    int passo = andando ? static_cast<int>(tempo * 8) % 2 : 0;
    bool baixa = g.bicando && static_cast<int>(tempo * 6) % 2;
    return {g.x, g.y, g.direcao, passo, baixa};
}

}

void desenharGalinhaEm(int x, int y, int d, int passo, bool cabecaBaixa) {
    int cabecaY = cabecaBaixa ? y - 1 : y - 3;
    px(x - 1, y + 1, 4, 1, Fade(CONTORNO, 0.25f));            // sombra
    px(x - 1, y - 2, 3, 2, FLOR_BRANCA);                      // corpo
    px(x - d * 2, y - 3, 1, 1, FLOR_BRANCA);                  // rabo
    px(x + d * 2, cabecaY, 1, 1, FLOR_BRANCA);                // cabeça
    px(x + d * 2, cabecaY - 1, 1, 1, TELHADO_VERM);           // crista
    px(x + d * 3, cabecaY, 1, 1, DOURADO);                    // bico
    px(x + d * 2 + (d > 0 ? 0.5f : 0), cabecaY, 0.5f, 0.5f, CONTORNO);   // olho
    px(x - 0.5f, y - 1.5f, 1.5f, 1, Fade(CONTORNO, 0.15f));  // asa
    px(x - 1 + passo, y, 1, 1, DOURADO_ESCURO);               // pé
    px(x + 1 - passo, y, 1, 1, DOURADO_ESCURO);
}

int obterGalinhas(EstadoGalinha saida[], int maximo, float tempo) {
    int total = 0;
    for (int i = 0; i < totalGalinhas && total < maximo; i++) {
        saida[total++] = estadoDaGalinha(galinhas[i], tempo);
    }
    return total;
}

void prepararGalinhas(Rectangle area, int quantidade, bool (*podeAndar)(float x, float y)) {
    areaGalinhas = area;
    galinhaPodeAndar = podeAndar;
    totalGalinhas = quantidade > 10 ? 10 : quantidade;
    for (int i = 0; i < totalGalinhas; i++) {
        Galinha& g = galinhas[i];
        novoDestino(g);
        g.x = g.alvoX;
        g.y = g.alvoY;
        g.espera = GetRandomValue(0, 20) / 10.0f;
        g.bicando = false;
        g.direcao = 1;
    }
}

void atualizarGalinhas(float dt) {
    for (int i = 0; i < totalGalinhas; i++) {
        Galinha& g = galinhas[i];
        if (g.espera > 0) {
            g.espera -= dt;
            continue;
        }
        float dx = g.alvoX - g.x, dy = g.alvoY - g.y;
        float distancia = sqrtf(dx * dx + dy * dy);
        if (distancia < 1) {
            g.espera = GetRandomValue(5, 35) / 10.0f;
            g.bicando = GetRandomValue(0, 1);
            novoDestino(g);
            continue;
        }
        g.direcao = dx < 0 ? -1 : 1;
        g.x += dx / distancia * 7 * dt;
        g.y += dy / distancia * 7 * dt;
    }
}

void desenharGalinhas(float tempo) {
    for (int i = 0; i < totalGalinhas; i++) {
        EstadoGalinha e = estadoDaGalinha(galinhas[i], tempo);
        desenharGalinhaEm(static_cast<int>(e.x), static_cast<int>(e.y), e.direcao, e.passo, e.cabecaBaixa);
    }
}
