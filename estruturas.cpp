#include "estruturas.h"

// ---------------- lista encadeada de edifícios ----------------

/*
 * Coloca o edifício no fim da lista.
 * Como temos o ponteiro "ultimo", não precisa percorrer nada: O(1).
 */
void inserirEdificio(ListaEdificios& lista, const Edificio& edificio) {
    NoEdificio* novo = new NoEdificio;
    novo->dados = edificio;
    novo->proximo = nullptr;

    if (lista.primeiro == nullptr) {
        lista.primeiro = novo;
    } else {
        lista.ultimo->proximo = novo;
    }
    lista.ultimo = novo;
    lista.tamanho++;
}

/*
 * Remove o nó com esse id. Casos tratados:
 *  - lista vazia            -> false
 *  - o nó é o primeiro      -> primeiro passa a ser o segundo
 *  - o nó está no meio/fim  -> o anterior "pula" ele
 * Se o removido era o último, o ponteiro "ultimo" volta pro anterior.
 */
bool removerEdificio(ListaEdificios& lista, int id) {
    NoEdificio* anterior = nullptr;
    NoEdificio* atual = lista.primeiro;

    while (atual != nullptr && atual->dados.id != id) {
        anterior = atual;
        atual = atual->proximo;
    }
    if (atual == nullptr) return false;

    if (anterior == nullptr) {
        lista.primeiro = atual->proximo;
    } else {
        anterior->proximo = atual->proximo;
    }
    if (atual == lista.ultimo) lista.ultimo = anterior;

    delete atual;
    lista.tamanho--;
    return true;
}

// Busca sequencial na lista pelo lote (linha, coluna). O(n).
const Edificio* buscarEdificioNoLote(const ListaEdificios& lista, int linha, int coluna) {
    for (NoEdificio* no = lista.primeiro; no != nullptr; no = no->proximo) {
        if (no->dados.linha == linha && no->dados.coluna == coluna) {
            return &no->dados;
        }
    }
    return nullptr;
}

int contarEdificiosDoTipo(const ListaEdificios& lista, int tipo) {
    int total = 0;
    for (NoEdificio* no = lista.primeiro; no != nullptr; no = no->proximo) {
        if (no->dados.tipo == tipo) total++;
    }
    return total;
}

/*
 * Libera todos os nós. Guarda o próximo ANTES do delete,
 * porque depois do delete não pode mais mexer em atual->proximo.
 */
void liberarEdificios(ListaEdificios& lista) {
    NoEdificio* atual = lista.primeiro;
    while (atual != nullptr) {
        NoEdificio* proximo = atual->proximo;
        delete atual;
        atual = proximo;
    }
    lista.primeiro = nullptr;
    lista.ultimo = nullptr;
    lista.tamanho = 0;
}
