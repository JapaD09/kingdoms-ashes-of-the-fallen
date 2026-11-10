#include "estruturas.h"

// insere no fim - O(1)
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

// remove pelo id - O(n)
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

// busca pelo lote - O(n)
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

// apaga todos os nós
void liberarEdificios(ListaEdificios& lista) {
    NoEdificio* atual = lista.primeiro;
    while (atual != nullptr) {
        NoEdificio* proximo = atual->proximo;   // guarda antes do delete
        delete atual;
        atual = proximo;
    }
    lista.primeiro = nullptr;
    lista.ultimo = nullptr;
    lista.tamanho = 0;
}
