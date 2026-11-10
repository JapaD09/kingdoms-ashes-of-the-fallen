#include "jogo.h"
#include "aleatorio.h"

using namespace std;

void iniciarJogo(Jogo& jogo, unsigned semente) {
    semearAleatorio(semente);
    jogo.turno = 1;
    jogo.rodando = true;
    jogo.derrota = false;
    jogo.emBatalha = false;
    iniciarReino(jogo.reino);
    sortearOrdemDosReinos(jogo.ordemInimigos);
    liberarArvore(jogo.catalogo);
    montarCatalogo(jogo.catalogo);
}

void encerrarJogo(Jogo& jogo) {
    liberarReino(jogo.reino);
    liberarArvore(jogo.catalogo);
}

// ids embaralhados de propósito pra árvore não virar uma "lista torta"
void montarCatalogo(NoArvore*& catalogo) {
    const Item itens[] = {
        {50, "Espada longa", "arma", 120, 2},     {25, "Arco curto", "arma", 70, 1},
        {75, "Elmo de ferro", "armadura", 90, 2}, {12, "Poção de vida", "poção", 25, 1},
        {37, "Machado", "arma", 80, 1},           {62, "Escudo", "armadura", 60, 1},
        {87, "Cajado rúnico", "arma", 200, 3},    {6, "Pão de viagem", "comida", 8, 1},
        {18, "Poção de força", "poção", 45, 2},   {31, "Lança", "arma", 65, 1},
        {43, "Cota de malha", "armadura", 150, 3}, {56, "Tocha", "ferramenta", 10, 1},
        {68, "Mapa antigo", "ferramenta", 35, 2}, {93, "Coroa das cinzas", "tesouro", 500, 3},
    };
    for (const Item& item : itens) inserirItem(catalogo, item);
}

int reinoDaVez(const Jogo& jogo) {
    return jogo.ordemInimigos[(jogo.turno - 1) % TOTAL_REINOS];
}

string encerrarTurno(Jogo& jogo) {
    if (jogo.emBatalha || jogo.derrota) return "";
    string resumo = processarTurnoReino(jogo.reino);
    string fila = processarFila(jogo.reino);
    montarBatalha(jogo.batalha, jogo.reino, reinoDaVez(jogo), jogo.turno);
    jogo.emBatalha = true;
    return fila.empty() ? resumo : resumo + "  " + fila;
}

string concluirBatalha(Jogo& jogo) {
    if (!jogo.emBatalha) return "";
    while (!jogo.batalha.acabou) rodadaDeBatalha(jogo.batalha);   // se pulou a animação
    string resultado = aplicarResultado(jogo.batalha, jogo.reino);
    jogo.emBatalha = false;

    if (jogo.reino.vidaCastelo <= 0) {
        jogo.derrota = true;
        return resultado;
    }
    descartarTudo(jogo.reino.baralho);
    comprarMao(jogo.reino.baralho);
    jogo.turno++;
    // fechou a volta nos 6 reinos: sorteia outra ordem
    if ((jogo.turno - 1) % TOTAL_REINOS == 0) sortearOrdemDosReinos(jogo.ordemInimigos);
    return resultado;
}
