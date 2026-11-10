// Kingdoms: Ashes of the Fallen
// Projeto Final - Programação e Sistemas Computacionais II
// Eduarda e Mateus

#include "jogo.h"
#include "interface.h"

int main() {
    Jogo jogo;
    iniciarJogo(jogo);

    abrirJanela();
    while (janelaAberta() && jogo.rodando) {
        atualizarInterface(jogo);
        desenharInterface(jogo);
    }
    fecharJanela();
    encerrarJogo(jogo);

    return 0;
}
