#include "algoritmos.h"

// Insertion sort pelo preço: pega cada item e empurra os maiores pra direita até
// achar o lugar dele. Ordena no próprio vetor.
// Complexidade: melhor caso O(n) (já ordenado), médio e pior caso O(n²); memória extra O(1).
// comparacoes volta com o total de comparações feitas.
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

// Busca sequencial: olha item por item, do começo ao fim. O(n).
// Não precisa do vetor ordenado. comparacoes volta com quantos itens foram olhados.
// Retorno: a posição do id no vetor ou -1 se não estiver lá.
int buscaSequencial(const Item itens[], int tamanho, int id, int& comparacoes) {
    comparacoes = 0;
    for (int i = 0; i < tamanho; i++) {
        comparacoes++;
        if (itens[i].id == id) return i;
    }
    return -1;
}

// Busca binária: compara com o item do meio e descarta a metade que não pode ter
// o id. O(log n). O vetor PRECISA estar ordenado por id.
// comparacoes volta com quantas vezes olhou o meio.
// Retorno: a posição do id no vetor ou -1 se não estiver lá.
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
