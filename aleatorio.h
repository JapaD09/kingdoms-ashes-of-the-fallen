#ifndef ALEATORIO_H
#define ALEATORIO_H

// Sorteio próprio (sem raylib) pra lógica do jogo poder ser testada.
// Com a mesma semente, sai sempre a mesma sequência.

void semearAleatorio(unsigned semente);
int sortear(int minimo, int maximo);   // inclui os dois
bool chance(int porcento);

#endif
