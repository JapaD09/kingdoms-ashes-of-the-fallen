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

string encerrarTurno(Jogo& jogo) {
    string resumo = processarTurnoReino(jogo.reino);
    string fila = processarFila(jogo.reino);
    jogo.turno++;
    string texto = "Turno " + to_string(jogo.turno) + ": " + resumo;
    return fila.empty() ? texto : texto + "  " + fila;
}
