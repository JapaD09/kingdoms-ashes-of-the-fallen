#include "figuras.h"
#include "paleta.h"
#include "reino.h"

namespace {

const Color PELE      = {236, 188, 140, 255};
const Color OSSO      = {226, 220, 198, 255};
const Color CAVALO    = {128, 84, 52, 255};
const Color CAVALO_ESC = {84, 54, 34, 255};
const Color AZUL_ESC  = {48, 64, 128, 255};

// desenha "espelhado" quando o boneco olha pra esquerda
struct Pincel {
    int x, y, s;
    bool esquerda;
    float brilho;

    void px(int dx, int dy, int w, int h, Color cor) const {
        if (brilho > 0) cor = ColorLerp(cor, WHITE, brilho);
        int left = esquerda ? x - (dx + w - 1) * s : x + dx * s;
        DrawRectangle(left, y + dy * s, w * s, h * s, cor);
    }
};

Aparencia base(Color pele, Color roupa, Color escura, Color detalhe, int arma, int chapeu) {
    Aparencia a;
    a.pele = pele;
    a.roupa = roupa;
    a.roupaEscura = escura;
    a.detalhe = detalhe;
    a.arma = arma;
    a.chapeu = chapeu;
    return a;
}

void desenharArma(const Pincel& p, const Aparencia& a, int topo) {
    switch (a.arma) {
        case ESPADA:
            p.px(4, topo + 1, 1, 4, PEDRA_CLARA);
            p.px(3, topo + 5, 3, 1, DOURADO_ESCURO);
            p.px(4, topo + 6, 1, 1, MADEIRA_ESCURA);
            break;
        case LANCA:
            p.px(4, topo - 2, 1, 2 - topo, MADEIRA);
            p.px(4, topo - 4, 1, 2, PEDRA_CLARA);
            break;
        case ARCO:
            p.px(4, topo, 1, 1, MADEIRA);
            p.px(5, topo + 1, 1, 6, MADEIRA);
            p.px(4, topo + 7, 1, 1, MADEIRA);
            p.px(4, topo + 1, 1, 6, Fade(PERGAMINHO, 0.7f));
            break;
        case CAJADO:
            p.px(4, topo - 1, 1, -topo, MADEIRA_ESCURA);
            p.px(3, topo - 3, 3, 3, Fade(a.brilhoArma, 0.35f));
            p.px(4, topo - 2, 1, 1, a.brilhoArma);
            break;
        case MACHADO:
            p.px(4, topo + 1, 1, 7, MADEIRA);
            p.px(5, topo + 1, 2, 3, PEDRA_CLARA);
            break;
        case MARTELO:
            p.px(4, topo + 2, 1, 6, MADEIRA);
            p.px(3, topo, 3, 2, PEDRA);
            break;
        case CLAVA:
            p.px(4, topo + 3, 1, 5, MADEIRA);
            p.px(4, topo, 2, 3, MADEIRA_ESCURA);
            break;
        case ADAGA:
            p.px(4, topo + 4, 1, 2, PEDRA_CLARA);
            break;
        default:
            break;
    }
}

// cabeça: linhas topo..topo+2, chapéu em cima
void desenharCabeca(const Pincel& p, const Aparencia& a, int topo) {
    p.px(-1, topo, 3, 3, a.pele);
    p.px(1, topo + 1, 1, 1, CONTORNO);                 // olho
    switch (a.chapeu) {
        case CABELO:
            p.px(-1, topo - 1, 3, 1, a.detalhe);
            p.px(-1, topo, 1, 2, a.detalhe);
            break;
        case ELMO:
            p.px(-2, topo - 1, 5, 2, a.detalhe);
            p.px(-2, topo + 1, 1, 2, a.detalhe);
            break;
        case CAPUZ:
            p.px(-1, topo - 1, 3, 1, a.roupa);
            p.px(-2, topo, 2, 3, a.roupa);
            break;
        case PONTUDO:
            p.px(-2, topo - 1, 5, 1, a.roupa);
            p.px(-1, topo - 2, 3, 1, a.roupa);
            p.px(-1, topo - 3, 2, 1, a.roupa);
            p.px(-2, topo - 4, 2, 1, a.roupa);
            break;
        case CHIFRES:
            p.px(-2, topo - 1, 5, 2, a.detalhe);
            p.px(-3, topo - 2, 1, 2, PERGAMINHO);
            p.px(3, topo - 2, 1, 2, PERGAMINHO);
            break;
        case COROA:
            p.px(-1, topo, 1, 3, a.detalhe);
            p.px(-1, topo - 1, 3, 1, DOURADO);
            p.px(-1, topo - 2, 1, 1, DOURADO);
            p.px(1, topo - 2, 1, 1, DOURADO);
            p.px(0, topo - 1, 1, 1, TELHADO_VERM);
            break;
        case ELMO_ASAS:
            p.px(-2, topo - 1, 5, 2, a.detalhe);
            p.px(-3, topo - 2, 1, 2, TEXTO_CLARO);
            p.px(3, topo - 2, 1, 2, TEXTO_CLARO);
            break;
        case TURBANTE:
            p.px(-2, topo - 2, 4, 2, a.detalhe);
            p.px(1, topo - 1, 1, 1, TELHADO_VERM);
            break;
        case GALHADA:
            p.px(-1, topo - 1, 3, 1, a.roupaEscura);
            p.px(-2, topo - 3, 1, 3, MADEIRA);
            p.px(2, topo - 3, 1, 3, MADEIRA);
            p.px(-3, topo - 3, 1, 1, MADEIRA);
            p.px(3, topo - 3, 1, 1, MADEIRA);
            break;
        case CAPACETE_BRONZE:
            p.px(-2, topo - 1, 5, 2, a.detalhe);
            p.px(-1, topo - 2, 3, 1, TELHADO_VERM);
            break;
        default:
            break;
    }
}

void desenharCavalo(const Pincel& p, int passo) {
    p.px(-5, -6, 1, 4, CAVALO_ESC);                    // rabo
    p.px(-4, -6, 9, 4, CAVALO);
    p.px(-4, -3, 9, 1, CAVALO_ESC);
    int perna = passo ? 1 : 0;
    p.px(-4, -2, 1, 2 - perna, CAVALO_ESC);
    p.px(-2, -2, 1, 2, CAVALO_ESC);
    p.px(2, -2, 1, 2, CAVALO_ESC);
    p.px(4, -2, 1, 2 - (1 - perna), CAVALO_ESC);
    p.px(4, -8, 2, 3, CAVALO);                         // pescoço
    p.px(5, -10, 3, 2, CAVALO);                        // cabeça
    p.px(4, -9, 1, 2, CONTORNO);                       // crina
    p.px(6, -10, 1, 1, CONTORNO);
}

void desenharDragao(const Pincel& p, const Aparencia& a, int passo) {
    p.px(-10, -3, 2, 1, a.roupaEscura);                // rabo
    p.px(-8, -4, 3, 1, a.roupaEscura);
    p.px(-6, -5, 2, 1, a.roupa);
    p.px(-5, -7, 10, 5, a.roupa);
    p.px(-4, -3, 8, 1, a.detalhe);                     // barriga
    p.px(-4, -2, 2, 2, a.roupaEscura);
    p.px(2, -2, 2, 2, a.roupaEscura);
    p.px(4, -9, 2, 4, a.roupa);                        // pescoço
    p.px(5, -11, 4, 3, a.roupa);                       // cabeça
    p.px(9, -10, 1, 2, a.roupa);
    p.px(6, -12, 1, 1, PERGAMINHO);                    // chifre
    p.px(7, -10, 1, 1, BRASA);                         // olho
    p.px(8, -8, 2, 1, Fade(BRASA, 0.6f));              // fumacinha
    if (passo) {                                       // asa batendo
        p.px(-3, -13, 2, 6, a.roupaEscura);
        p.px(-1, -11, 2, 4, a.roupaEscura);
        p.px(-5, -14, 2, 2, a.roupaEscura);
    } else {
        p.px(-4, -9, 7, 2, a.roupaEscura);
        p.px(-6, -8, 2, 2, a.roupaEscura);
    }
}

}

void desenharFigura(int x, int y, const Aparencia& a, bool olhaEsquerda, int passo, float brilho) {
    Pincel p = {x, y, a.tamanho, olhaEsquerda, brilho};
    if (a.especial == DRAGAO) {
        desenharDragao(p, a, passo);
        return;
    }
    int sobe = 0;
    if (a.especial == MONTADO) {
        desenharCavalo(p, passo);
        sobe = 5;
    }
    int corpo = a.baixinho ? 3 : 5;
    int cintura = -3 - sobe;
    int ombro = cintura - corpo + 1;
    int cabeca = ombro - 3;

    if (a.capa) p.px(-3, ombro - 1, 1, corpo + 3, a.corCapa);
    if (a.especial != MONTADO) {
        p.px(-2, -2, 2, passo ? 1 : 2, a.roupaEscura);  // pernas
        p.px(1, -2, 2, passo ? 2 : 1, a.roupaEscura);
    } else {
        p.px(-1, cintura + 1, 2, 2, a.roupaEscura);      // perna montada
    }
    p.px(-2, ombro, 5, corpo, a.roupa);
    p.px(-2, cintura, 5, 1, a.roupaEscura);              // cinto
    p.px(-3, ombro + 1, 1, 2, a.roupaEscura);            // braço de trás
    p.px(3, ombro + 1, 1, 2, a.roupa);                   // braço da frente
    p.px(3, ombro + 3, 1, 1, a.pele);
    if (a.escudo) {
        p.px(-4, ombro, 2, 4, a.detalhe);
        p.px(-4, ombro + 1, 1, 2, a.roupa);
    }
    desenharCabeca(p, a, cabeca);
    desenharArma(p, a, cabeca + 1);
}

void desenharFiguraCaida(int x, int y, const Aparencia& a, bool olhaEsquerda) {
    Pincel p = {x, y, a.tamanho, olhaEsquerda, 0};
    if (a.especial == DRAGAO) {
        p.px(-9, -1, 3, 1, Fade(a.roupaEscura, 0.8f));       // rabo
        p.px(-6, -3, 10, 3, Fade(a.roupa, 0.8f));
        p.px(-3, -5, 6, 2, Fade(a.roupaEscura, 0.7f));       // asa caída
        p.px(4, -2, 4, 2, Fade(a.roupa, 0.8f));              // cabeça no chão
        p.px(6, -3, 1, 1, Fade(PERGAMINHO, 0.8f));
        return;
    }
    p.px(-4, -2, 5, 2, Fade(a.roupa, 0.8f));
    p.px(1, -3, 3, 3, Fade(a.pele, 0.8f));
    p.px(-6, -1, 2, 1, Fade(a.roupaEscura, 0.8f));
    if (a.arma != SEM_ARMA) p.px(-3, -4, 5, 1, Fade(PEDRA, 0.6f));
}

Aparencia aparenciaTropa(int tropa) {
    Aparencia a;
    switch (tropa) {
        case SOLDADO:
            a = base(PELE, TELHADO_AZUL, AZUL_ESC, PEDRA_CLARA, ESPADA, ELMO);
            a.escudo = true;
            break;
        case ARQUEIRO:
            a = base(PELE, FOLHA_CLARA, FOLHA, MADEIRA, ARCO, CAPUZ);
            break;
        case CAVALEIRO:
            a = base(PELE, PEDRA_CLARA, PEDRA, PEDRA_CLARA, LANCA, ELMO);
            a.especial = MONTADO;
            a.capa = true;
            a.corCapa = TELHADO_AZUL;
            break;
        case MAGO:
            a = base(PELE, ROXO, ROXO_ESC, ROXO, CAJADO, PONTUDO);
            a.brilhoArma = {120, 200, 255, 255};
            break;
        default:      // guarda do castelo
            a = base(PELE, TELHADO_AZUL, AZUL_ESC, PEDRA, LANCA, ELMO);
            break;
    }
    return a;
}

Aparencia aparenciaHeroi(int heroi) {
    Aparencia a;
    switch (heroi) {
        case 0:   // Bran
            a = base(PELE, PEDRA_CLARA, PEDRA, PEDRA, ESPADA, ELMO);
            a.escudo = true;
            break;
        case 1:   // Lia
            a = base(PELE, {78, 136, 74, 255}, FOLHA, {204, 120, 52, 255}, ARCO, CABELO);
            break;
        case 2:   // Tomas
            a = base({214, 160, 116, 255}, MADEIRA, MADEIRA_ESCURA, {64, 42, 30, 255}, MARTELO, CABELO);
            break;
        case 3:   // Mira
            a = base(PELE, {236, 234, 224, 255}, {190, 190, 204, 255}, DOURADO, CAJADO, CABELO);
            a.brilhoArma = {120, 255, 150, 255};
            break;
        case 4:   // Kael
            a = base(PELE, {58, 112, 150, 255}, {36, 70, 100, 255}, PEDRA_CLARA, LANCA, ELMO);
            break;
        case 5:   // Sigrid
            a = base(PELE, PEDRA_CLARA, PEDRA, PEDRA, ESPADA, ELMO_ASAS);
            a.escudo = true;
            break;
        case 6:   // Oren
            a = base(PELE, {52, 72, 164, 255}, {30, 44, 110, 255}, DOURADO, CAJADO, PONTUDO);
            a.brilhoArma = BRASA;
            break;
        case 7:   // Draven
            a = base(PELE, {54, 50, 62, 255}, {32, 30, 38, 255}, {54, 50, 62, 255}, ESPADA, CHIFRES);
            a.especial = MONTADO;
            break;
        default:  // Aurora
            a = base(PELE, {170, 44, 44, 255}, VERMELHO_ESC, {240, 240, 244, 255}, CAJADO, COROA);
            a.brilhoArma = BRASA;
            break;
    }
    a.capa = true;
    const Color capas[TOTAL_HEROIS] = {TELHADO_VERM, FOLHA, MADEIRA_ESCURA, {200, 200, 214, 255},
                                       DOURADO_ESCURO, TELHADO_AZUL, ROXO_ESC, VERMELHO_ESC, {90, 20, 30, 255}};
    a.corCapa = capas[heroi];
    return a;
}

Aparencia aparenciaInimigo(int reino, int nivel) {
    Aparencia a;
    const Color goblin = {122, 172, 72, 255}, orc = {94, 132, 62, 255}, areia = {200, 150, 104, 255};
    const Color elfo = {240, 222, 204, 255}, cinza = {204, 162, 142, 255};
    switch (reino * 3 + nivel) {
        case 0: a = base(goblin, MADEIRA, MADEIRA_ESCURA, goblin, ADAGA, CABELO); a.baixinho = true; break;
        case 1: a = base(goblin, {112, 92, 62, 255}, MADEIRA_ESCURA, goblin, ARCO, CAPUZ); a.baixinho = true; break;
        case 2: a = base({112, 132, 104, 255}, MADEIRA_ESCURA, CONTORNO, {112, 132, 104, 255}, CLAVA, CABELO); a.tamanho = 2; break;
        case 3: a = base(orc, {122, 62, 42, 255}, {80, 40, 30, 255}, CONTORNO, MACHADO, CABELO); break;
        case 4: a = base(orc, {122, 62, 42, 255}, {80, 40, 30, 255}, {100, 90, 80, 255}, LANCA, ELMO); break;
        case 5: a = base(orc, {100, 40, 34, 255}, {60, 24, 20, 255}, {90, 80, 70, 255}, MACHADO, CHIFRES);
                a.tamanho = 2; a.capa = true; a.corCapa = {60, 20, 20, 255}; break;
        case 6: a = base(OSSO, {176, 172, 158, 255}, {120, 116, 106, 255}, OSSO, ESPADA, CABELO); break;
        case 7: a = base(OSSO, {72, 62, 84, 255}, {44, 38, 52, 255}, OSSO, ARCO, CAPUZ); break;
        case 8: a = base(OSSO, {42, 42, 54, 255}, {26, 26, 34, 255}, {64, 64, 80, 255}, ESPADA, ELMO);
                a.especial = MONTADO; a.tamanho = 2; a.capa = true; a.corCapa = {92, 40, 124, 255}; break;
        case 9: a = base(areia, {204, 172, 112, 255}, {150, 120, 72, 255}, {204, 142, 62, 255}, LANCA, CAPACETE_BRONZE);
                a.escudo = true; break;
        case 10: a = base(areia, {222, 192, 124, 255}, {160, 130, 80, 255}, {242, 232, 204, 255}, ARCO, TURBANTE); break;
        case 11: a = base(areia, {192, 132, 62, 255}, {140, 90, 40, 255}, {204, 142, 62, 255}, ESPADA, CAPACETE_BRONZE);
                 a.escudo = true; a.tamanho = 2; break;
        case 12: a = base(elfo, {152, 172, 192, 255}, {100, 116, 140, 255}, {242, 242, 250, 255}, ESPADA, CABELO); break;
        case 13: a = base(elfo, {102, 142, 112, 255}, {62, 96, 72, 255}, {242, 242, 250, 255}, ARCO, CAPUZ); break;
        case 14: a = base(elfo, {112, 82, 52, 255}, {72, 52, 32, 255}, {242, 242, 250, 255}, CAJADO, GALHADA);
                 a.brilhoArma = {140, 255, 140, 255}; a.tamanho = 2; break;
        case 15: a = base(cinza, {124, 32, 32, 255}, {74, 20, 20, 255}, CONTORNO, ADAGA, CAPUZ); break;
        case 16: a = base(cinza, {62, 32, 32, 255}, {34, 18, 18, 255}, CONTORNO, CAJADO, PONTUDO);
                 a.brilhoArma = BRASA; break;
        default: a = base(cinza, {112, 42, 40, 255}, {70, 26, 24, 255}, {204, 124, 64, 255}, SEM_ARMA, CABELO);
                 a.especial = DRAGAO; a.tamanho = 2; break;
    }
    return a;
}

Color corDoReino(int reino) {
    const Color cores[6] = {{112, 162, 62, 255}, {172, 52, 42, 255}, {122, 72, 162, 255},
                            {222, 172, 62, 255}, {192, 202, 222, 255}, {62, 52, 52, 255}};
    return cores[reino % 6];
}

void desenharBandeira(int x, int y, int reino, int s) {
    Color cor = corDoReino(reino);
    DrawRectangle(x, y, s, 9 * s, CONTORNO);
    DrawRectangle(x + s, y, 6 * s, 6 * s, cor);
    DrawRectangle(x + s, y + 6 * s, 2 * s, s, cor);
    DrawRectangle(x + 5 * s, y + 6 * s, 2 * s, s, cor);
    DrawRectangle(x + 3 * s, y + 2 * s, 2 * s, 2 * s, reino == 5 ? BRASA : CONTORNO);   // símbolo
}
