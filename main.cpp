// Kingdoms: Ashes of the Fallen
// Projeto Final - Programação e Sistemas Computacionais II
// Eduarda e Mateus

#include <ctime>
#include "jogo.h"
#include "interface.h"

int main() {
    Jogo jogo;
    iniciarJogo(jogo, static_cast<unsigned>(time(nullptr)));   // cada partida começa diferente

    abrirJanela();
    while (janelaAberta() && jogo.rodando) {
        atualizarInterface(jogo);
        desenharInterface(jogo);
    }
    fecharJanela();
    encerrarJogo(jogo);

    return 0;
}
