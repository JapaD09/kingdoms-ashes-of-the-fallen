#include "jogo.h"

using namespace std;

void iniciarJogo(Jogo& jogo) {
    jogo.turno = 1;
    jogo.rodando = true;
    iniciarReino(jogo.reino);
}

// Por enquanto o turno so conta. Producao e consumo entram na etapa 3.
string encerrarTurno(Jogo& jogo) {
    jogo.turno++;
    return "O sol se pos sobre " + jogo.nomeReino
         + ". Comeca o turno " + to_string(jogo.turno) + ".";
}
