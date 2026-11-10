#include "jogo.h"

using namespace std;

void iniciarJogo(Jogo& jogo) {
    jogo.turno = 1;
    jogo.rodando = true;
    iniciarReino(jogo.reino);
}

// Por enquanto o turno só conta. Produção e consumo entram na etapa 3.
string encerrarTurno(Jogo& jogo) {
    jogo.turno++;
    return "O sol se pôs sobre " + jogo.nomeReino
         + ". Começa o turno " + to_string(jogo.turno) + ".";
}
