#ifndef ALGORITMOS_H
#define ALGORITMOS_H

#include "estruturas.h"

// Ordenação e buscas no catálogo (R9 e R10).
// "comparacoes" volta com quantas comparações foram feitas, pra mostrar na tela.

// Insertion sort pelo preço (menor primeiro).
// Melhor caso (já ordenado): O(n). Médio e pior (ordem inversa): O(n²).
// Memória extra: O(1), ordena no próprio vetor.
void ordenarPorValor(Item itens[], int tamanho, int& comparacoes);

// Olha um por um. Não precisa estar ordenado. O(n).
// Devolve a posição ou -1.
int buscaSequencial(const Item itens[], int tamanho, int id, int& comparacoes);

// Corta o intervalo pela metade a cada passo. O(log n).
// O vetor PRECISA estar ordenado por id. Devolve a posição ou -1.
int buscaBinaria(const Item itens[], int tamanho, int id, int& comparacoes);

#endif
