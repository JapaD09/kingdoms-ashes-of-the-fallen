#include "aleatorio.h"

static unsigned estado = 12345;

// Define a semente do sorteio. A mesma semente gera sempre a mesma partida
// (é o que deixa os testes e o simulador repetíveis). Semente 0 vira 1.
void semearAleatorio(unsigned semente) {
    estado = semente == 0 ? 1 : semente;
}

// Gerador congruente linear (mesmas constantes do glibc).
// Retorno: o próximo número da sequência, entre 0 e 32767.
static unsigned proximo() {
    estado = estado * 1103515245u + 12345u;
    return (estado >> 16) & 0x7fff;
}

// Sorteia um inteiro entre minimo e maximo, os dois inclusos.
// Retorno: minimo se o intervalo for inválido (maximo <= minimo).
int sortear(int minimo, int maximo) {
    if (maximo <= minimo) return minimo;
    return minimo + static_cast<int>(proximo() % static_cast<unsigned>(maximo - minimo + 1));
}

// Retorno: true em "porcento" por cento das vezes (ex.: chance(35) acerta 35% das vezes).
bool chance(int porcento) {
    return sortear(1, 100) <= porcento;
}
