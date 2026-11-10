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

// pilha

// coloca no topo - O(1)
void empilhar(PilhaAcoes& pilha, const Acao& acao) {
    NoPilha* novo = new NoPilha;
    novo->acao = acao;
    novo->abaixo = pilha.topo;
    pilha.topo = novo;
    pilha.tamanho++;
}

// tira do topo - O(1)
bool desempilhar(PilhaAcoes& pilha, Acao& saida) {
    if (pilha.topo == nullptr) return false;
    NoPilha* velho = pilha.topo;
    saida = velho->acao;
    pilha.topo = velho->abaixo;
    delete velho;
    pilha.tamanho--;
    return true;
}

const Acao* topoDaPilha(const PilhaAcoes& pilha) {
    return pilha.topo ? &pilha.topo->acao : nullptr;
}

bool pilhaVazia(const PilhaAcoes& pilha) {
    return pilha.topo == nullptr;
}

void limparPilha(PilhaAcoes& pilha) {
    Acao lixo;
    while (desempilhar(pilha, lixo)) {
    }
}

// fila

// entra no fim - O(1)
void enfileirar(FilaConstrucao& fila, const OrdemConstrucao& ordem) {
    NoFila* novo = new NoFila;
    novo->ordem = ordem;
    novo->proximo = nullptr;
    if (fila.fim == nullptr) {
        fila.inicio = novo;
    } else {
        fila.fim->proximo = novo;
    }
    fila.fim = novo;
    fila.tamanho++;
}

// sai do começo - O(1)
bool desenfileirar(FilaConstrucao& fila, OrdemConstrucao& saida) {
    if (fila.inicio == nullptr) return false;
    NoFila* velho = fila.inicio;
    saida = velho->ordem;
    fila.inicio = velho->proximo;
    if (fila.inicio == nullptr) fila.fim = nullptr;
    delete velho;
    fila.tamanho--;
    return true;
}

const OrdemConstrucao* frenteDaFila(const FilaConstrucao& fila) {
    return fila.inicio ? &fila.inicio->ordem : nullptr;
}

bool filaVazia(const FilaConstrucao& fila) {
    return fila.inicio == nullptr;
}

bool loteNaFila(const FilaConstrucao& fila, int linha, int coluna) {
    for (NoFila* no = fila.inicio; no != nullptr; no = no->proximo) {
        if (no->ordem.linha == linha && no->ordem.coluna == coluna) return true;
    }
    return false;
}

void limparFila(FilaConstrucao& fila) {
    OrdemConstrucao lixo;
    while (desenfileirar(fila, lixo)) {
    }
}
