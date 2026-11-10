#include "jogo.h"

using namespace std;

void iniciarJogo(Jogo& jogo) {
    jogo.turno = 1;
    jogo.rodando = true;
    iniciarReino(jogo.reino);
}

void encerrarJogo(Jogo& jogo) {
    liberarReino(jogo.reino);
}

// Ordem do guia: produção -> consumo -> consequências -> avança o turno
string encerrarTurno(Jogo& jogo) {
    string resumo = processarTurnoReino(jogo.reino);
    jogo.turno++;
    return "Turno " + to_string(jogo.turno) + ": " + resumo;
}
