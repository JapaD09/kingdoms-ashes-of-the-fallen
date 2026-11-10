#include "cartas.h"
#include "reino.h"
#include "aleatorio.h"

namespace {

// custo de prédio e tropa fica zerado aqui porque vem do edifício
const ModeloCarta CARTAS[TOTAL_CARTAS] = {
    {"Fazenda",       CARTA_PREDIO,   FAZENDA,      {}, 1, "+8 comida\npor nível"},
    {"Serraria",      CARTA_PREDIO,   SERRARIA,     {}, 1, "+6 madeira\npor nível"},
    {"Mina",          CARTA_PREDIO,   MINA,         {}, 1, "+4 pedra e\n+2 ferro"},
    {"Casa",          CARTA_PREDIO,   CASA,         {}, 1, "+6 pessoas\npor nível"},
    {"Mercado",       CARTA_PREDIO,   MERCADO,      {}, 2, "+10 ouro\npor nível"},
    {"Muralha",       CARTA_PREDIO,   MURALHA,      {}, 1, "castelo\ntoma menos\ndano"},
    {"Torre",         CARTA_PREDIO,   TORRE,        {}, 2, "atira nos\ninimigos\nna batalha"},
    {"Oficina",       CARTA_PREDIO,   OFICINA,      {}, 2, "+1 ataque\nem todas\nas tropas"},
    {"Quartel",       CARTA_TROPA,    QUARTEL,      {}, 1, "3 soldados\npor nível"},
    {"Arquearia",     CARTA_TROPA,    ARQUEARIA,    {}, 1, "2 arqueiros\npor nível"},
    {"Estábulo",      CARTA_TROPA,    ESTABULO,     {}, 2, "1 cavaleiro\npor nível"},
    {"Torre Arcana",  CARTA_TROPA,    TORRE_ARCANA, {}, 3, "1 mago por\nnível, acerta\n2 de uma vez"},
    {"Lâminas",       CARTA_ENCANTO,  LAMINAS,      {20, 0, 0, 0, 8}, 1, "+2 ataque\nnas tropas\ndo prédio"},
    {"Armaduras",     CARTA_ENCANTO,  ARMADURAS,    {20, 0, 0, 0, 8}, 1, "+5 vida\nnas tropas\ndo prédio"},
    {"Estandarte",    CARTA_ENCANTO,  ESTANDARTE,   {35, 10, 0, 0, 0}, 2, "+1 tropa\nno prédio"},
    {"Bênção Real",   CARTA_ENCANTO,  BENCAO,       {40, 0, 0, 0, 0}, 3, "+1 nível em\nqualquer\nprédio"},
    {"Expansão",      CARTA_EXPANSAO, 0,            {15, 20, 10, 0, 0}, 1, "abre um\nlote da\nfloresta"},
    {"Caravana",      CARTA_INSTANTE, CARAVANA,     {}, 2, "+50 ouro\n+20 madeira\n+10 pedra"},
};

const int MAX_CARTAS_BARALHO = 64;

}

const ModeloCarta& modeloCarta(int id) {
    return CARTAS[id];
}

Recursos custoDaCarta(int id) {
    const ModeloCarta& carta = CARTAS[id];
    if (carta.tipo == CARTA_PREDIO || carta.tipo == CARTA_TROPA) {
        return modeloEdificio(carta.alvo).custo;
    }
    return carta.custo;
}

const char* nomeTipoCarta(int tipo) {
    switch (tipo) {
        case CARTA_PREDIO: return "Prédio";
        case CARTA_TROPA: return "Tropa";
        case CARTA_ENCANTO: return "Encanto";
        case CARTA_EXPANSAO: return "Terreno";
        default: return "Instante";
    }
}

void montarBaralho(Baralho& baralho) {
    liberarBaralho(baralho);
    for (int id = 0; id < TOTAL_CARTAS; id++) {
        int copias = 4 - CARTAS[id].raridade;
        for (int i = 0; i < copias; i++) colocarCarta(baralho.monte, id);
    }
    embaralhar(baralho.monte);
}

// esvazia a pilha num vetor, embaralha (Fisher-Yates) e empilha de novo
void embaralhar(PilhaCartas& pilha) {
    int cartas[MAX_CARTAS_BARALHO];
    int total = 0;
    int carta;
    while (total < MAX_CARTAS_BARALHO && tirarCarta(pilha, carta)) cartas[total++] = carta;

    for (int i = total - 1; i > 0; i--) {
        int j = sortear(0, i);
        int troca = cartas[i];
        cartas[i] = cartas[j];
        cartas[j] = troca;
    }
    for (int i = 0; i < total; i++) colocarCarta(pilha, cartas[i]);
}

void comprarMao(Baralho& baralho) {
    while (baralho.naMao < TAMANHO_MAO) {
        if (baralho.monte.tamanho == 0) {
            int carta;
            while (tirarCarta(baralho.descarte, carta)) colocarCarta(baralho.monte, carta);
            embaralhar(baralho.monte);
        }
        int carta;
        if (!tirarCarta(baralho.monte, carta)) return;   // acabou tudo
        baralho.mao[baralho.naMao++] = carta;
    }
}

int usarCarta(Baralho& baralho, int indice) {
    if (indice < 0 || indice >= baralho.naMao) return -1;
    int carta = baralho.mao[indice];
    for (int i = indice; i < baralho.naMao - 1; i++) baralho.mao[i] = baralho.mao[i + 1];
    baralho.naMao--;
    if (baralho.totalUsadas < MAX_USADAS) baralho.usadas[baralho.totalUsadas++] = carta;
    return carta;
}

bool devolverCarta(Baralho& baralho, int carta) {
    if (baralho.naMao >= TAMANHO_MAO) return false;
    for (int i = baralho.totalUsadas - 1; i >= 0; i--) {
        if (baralho.usadas[i] != carta) continue;
        for (int j = i; j < baralho.totalUsadas - 1; j++) baralho.usadas[j] = baralho.usadas[j + 1];
        baralho.totalUsadas--;
        baralho.mao[baralho.naMao++] = carta;
        return true;
    }
    return false;
}

void descartarTudo(Baralho& baralho) {
    for (int i = 0; i < baralho.naMao; i++) colocarCarta(baralho.descarte, baralho.mao[i]);
    for (int i = 0; i < baralho.totalUsadas; i++) colocarCarta(baralho.descarte, baralho.usadas[i]);
    baralho.naMao = 0;
    baralho.totalUsadas = 0;
}

int totalDeCartas(const Baralho& baralho) {
    return baralho.monte.tamanho + baralho.descarte.tamanho + baralho.naMao + baralho.totalUsadas;
}

void liberarBaralho(Baralho& baralho) {
    limparCartas(baralho.monte);
    limparCartas(baralho.descarte);
    baralho.naMao = 0;
    baralho.totalUsadas = 0;
}
