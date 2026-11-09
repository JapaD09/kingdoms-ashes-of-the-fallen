// Kingdoms: Ashes of the Fallen
// Projeto Final - Programacao e Sistemas Computacionais II
// Eduarda e Mateus

#include <cstdlib>
#include <ctime>
#include "jogo.h"

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    Jogo jogo;
    iniciarJogo(jogo);
    loopPrincipal(jogo);

    return 0;
}
