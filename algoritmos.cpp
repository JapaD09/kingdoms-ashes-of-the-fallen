#include "algoritmos.h"

// pega cada item e empurra pra esquerda até achar o lugar dele
void ordenarPorValor(Item itens[], int tamanho, int& comparacoes) {
    comparacoes = 0;
    for (int i = 1; i < tamanho; i++) {
        Item atual = itens[i];
        int j = i - 1;
        while (j >= 0) {
            comparacoes++;
            if (itens[j].valor <= atual.valor) break;
            itens[j + 1] = itens[j];
            j--;
        }
        itens[j + 1] = atual;
    }
}

int buscaSequencial(const Item itens[], int tamanho, int id, int& comparacoes) {
    comparacoes = 0;
    for (int i = 0; i < tamanho; i++) {
        comparacoes++;
        if (itens[i].id == id) return i;
    }
    return -1;
}

int buscaBinaria(const Item itens[], int tamanho, int id, int& comparacoes) {
    comparacoes = 0;
    int inicio = 0;
    int fim = tamanho - 1;
    while (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2;   // evita estourar int
        comparacoes++;
        if (itens[meio].id == id) return meio;
        if (itens[meio].id < id) {
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
    }
    return -1;
}
