#include "aleatorio.h"

static unsigned estado = 12345;

void semearAleatorio(unsigned semente) {
    estado = semente == 0 ? 1 : semente;
}

// gerador congruente linear (mesmas constantes do glibc)
static unsigned proximo() {
    estado = estado * 1103515245u + 12345u;
    return (estado >> 16) & 0x7fff;
}

int sortear(int minimo, int maximo) {
    if (maximo <= minimo) return minimo;
    return minimo + static_cast<int>(proximo() % static_cast<unsigned>(maximo - minimo + 1));
}

bool chance(int porcento) {
    return sortear(1, 100) <= porcento;
}
