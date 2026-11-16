#include "estruturas.h"

// ---------------- Lista encadeada de edifícios (R5) ----------------

// Coloca um edifício no fim da lista. Como guardamos o último nó, não precisa
// andar a lista: O(1).
// lista: onde inserir. edificio: os dados copiados pro nó novo.
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

// Tira da lista o edifício com esse id e apaga o nó. O(n).
// Trata os casos de lista vazia, primeiro, meio e último nó.
// Retorno: true se achou e removeu, false se o id não estava na lista.
bool removerEdificio(ListaEdificios& lista, int id) {
    NoEdificio* anterior = nullptr;
    NoEdificio* atual = lista.primeiro;

    while (atual != nullptr && atual->dados.id != id) {
        anterior = atual;
        atual = atual->proximo;
    }
    if (atual == nullptr) return false;

    if (anterior == nullptr) {
        lista.primeiro = atual->proximo;             // era o primeiro
    } else {
        anterior->proximo = atual->proximo;          // pula o nó removido
    }
    if (atual == lista.ultimo) lista.ultimo = anterior;

    delete atual;
    atual = nullptr;
    lista.tamanho--;
    return true;
}

// Procura o edifício que está no lote (linha, coluna). O(n).
// Retorno: ponteiro pros dados (só leitura) ou nullptr se o lote está vazio.
const Edificio* buscarEdificioNoLote(const ListaEdificios& lista, int linha, int coluna) {
    for (NoEdificio* no = lista.primeiro; no != nullptr; no = no->proximo) {
        if (no->dados.linha == linha && no->dados.coluna == coluna) {
            return &no->dados;
        }
    }
    return nullptr;
}

// Procura pelo id e devolve um ponteiro que permite alterar o edifício
// (usado pra subir de nível e pra desfazer). Retorno: nullptr se não achou.
Edificio* acharEdificio(ListaEdificios& lista, int id) {
    for (NoEdificio* no = lista.primeiro; no != nullptr; no = no->proximo) {
        if (no->dados.id == id) return &no->dados;
    }
    return nullptr;
}

// Percorre a lista contando quantos edifícios são do tipo pedido.
// Retorno: a quantidade (0 se não tiver nenhum).
int contarEdificiosDoTipo(const ListaEdificios& lista, int tipo) {
    int total = 0;
    for (NoEdificio* no = lista.primeiro; no != nullptr; no = no->proximo) {
        if (no->dados.tipo == tipo) total++;
    }
    return total;
}

// Apaga todos os nós e deixa a lista vazia, pronta pra usar de novo.
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

// ---------------- Pilha de ações do Desfazer (R6) ----------------

// Coloca a ação no topo. O novo nó aponta pro antigo topo: O(1).
void empilhar(PilhaAcoes& pilha, const Acao& acao) {
    NoPilha* novo = new NoPilha;
    novo->acao = acao;
    novo->abaixo = pilha.topo;
    pilha.topo = novo;
    pilha.tamanho++;
}

// Tira a ação do topo (a última que entrou) e copia em "saida". O(1).
// Retorno: false se a pilha estava vazia (aí "saida" não muda).
bool desempilhar(PilhaAcoes& pilha, Acao& saida) {
    if (pilha.topo == nullptr) return false;
    NoPilha* velho = pilha.topo;
    saida = velho->acao;
    pilha.topo = velho->abaixo;
    delete velho;
    velho = nullptr;
    pilha.tamanho--;
    return true;
}

// Olha o topo sem tirar. Retorno: a ação do topo ou nullptr se a pilha está vazia.
const Acao* topoDaPilha(const PilhaAcoes& pilha) {
    return pilha.topo ? &pilha.topo->acao : nullptr;
}

// Retorno: true se não tem nada pra desfazer.
bool pilhaVazia(const PilhaAcoes& pilha) {
    return pilha.topo == nullptr;
}

// Desempilha (e apaga) tudo. Usado na virada do turno e no fim da partida.
void limparPilha(PilhaAcoes& pilha) {
    Acao lixo;
    while (desempilhar(pilha, lixo)) {
    }
}

// ---------------- Pilha de cartas: monte e descarte ----------------

// Põe a carta no topo da pilha. O(1).
void colocarCarta(PilhaCartas& pilha, int carta) {
    NoCarta* novo = new NoCarta;
    novo->carta = carta;
    novo->abaixo = pilha.topo;
    pilha.topo = novo;
    pilha.tamanho++;
}

// Compra a carta do topo e devolve em "carta". O(1).
// Retorno: false se a pilha estava vazia.
bool tirarCarta(PilhaCartas& pilha, int& carta) {
    if (pilha.topo == nullptr) return false;
    NoCarta* velho = pilha.topo;
    carta = velho->carta;
    pilha.topo = velho->abaixo;
    delete velho;
    velho = nullptr;
    pilha.tamanho--;
    return true;
}

// Tira e apaga todas as cartas da pilha.
void limparCartas(PilhaCartas& pilha) {
    int lixo;
    while (tirarCarta(pilha, lixo)) {
    }
}

// ---------------- Fila de ordens de construção (R7) ----------------

// A ordem entra no fim da fila. Guardamos o fim, então é O(1).
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

// Tira a ordem da frente (a mais antiga) e copia em "saida". O(1).
// Retorno: false se a fila estava vazia.
bool desenfileirar(FilaConstrucao& fila, OrdemConstrucao& saida) {
    if (fila.inicio == nullptr) return false;
    NoFila* velho = fila.inicio;
    saida = velho->ordem;
    fila.inicio = velho->proximo;
    if (fila.inicio == nullptr) fila.fim = nullptr;     // esvaziou: o fim também some
    delete velho;
    velho = nullptr;
    fila.tamanho--;
    return true;
}

// Olha a primeira ordem sem tirar. Retorno: nullptr se a fila está vazia.
const OrdemConstrucao* frenteDaFila(const FilaConstrucao& fila) {
    return fila.inicio ? &fila.inicio->ordem : nullptr;
}

// Retorno: true se não tem ordem esperando.
bool filaVazia(const FilaConstrucao& fila) {
    return fila.inicio == nullptr;
}

// Percorre a fila pra saber se já tem ordem pra esse lote (evita duas no mesmo lugar).
// Retorno: true se o lote já está reservado.
bool loteNaFila(const FilaConstrucao& fila, int linha, int coluna) {
    for (NoFila* no = fila.inicio; no != nullptr; no = no->proximo) {
        if (no->ordem.linha == linha && no->ordem.coluna == coluna) return true;
    }
    return false;
}

// Tira e apaga todas as ordens.
void limparFila(FilaConstrucao& fila) {
    OrdemConstrucao lixo;
    while (desenfileirar(fila, lixo)) {
    }
}

// ---------------- Árvore binária de busca do catálogo (R8, R3) ----------------

// Recursiva: desce pra esquerda (id menor) ou direita (id maior) até achar
// um lugar vazio e cria o nó ali. O(h), h = altura da árvore.
// raiz é passada por referência pra função poder trocar o próprio ponteiro do pai.
// Retorno: false se o id já existia (não insere repetido).
bool inserirItem(NoArvore*& raiz, const Item& item) {
    if (raiz == nullptr) {                          // caso-base: achou o lugar
        raiz = new NoArvore;
        raiz->item = item;
        return true;
    }
    if (item.id == raiz->item.id) return false;
    if (item.id < raiz->item.id) return inserirItem(raiz->esquerda, item);
    return inserirItem(raiz->direita, item);
}

// Recursiva: procura o id descendo um galho por vez. O(h).
// comparacoes: soma 1 a cada nó olhado, pra mostrar na tela.
// Retorno: o item achado ou nullptr (caso-base: chegou num nó vazio).
const Item* buscarItem(const NoArvore* raiz, int id, int& comparacoes) {
    if (raiz == nullptr) return nullptr;
    comparacoes++;
    if (id == raiz->item.id) return &raiz->item;
    if (id < raiz->item.id) return buscarItem(raiz->esquerda, id, comparacoes);
    return buscarItem(raiz->direita, id, comparacoes);
}

// Percurso in-ordem recursivo (esquerda, nó, direita): copia os itens pro vetor
// em ordem crescente de id. O(n).
// quantidade: quantos já foram copiados (cresce a cada item). maximo: tamanho do vetor.
void copiarEmOrdem(const NoArvore* raiz, Item vetor[], int& quantidade, int maximo) {
    if (raiz == nullptr || quantidade >= maximo) return;
    copiarEmOrdem(raiz->esquerda, vetor, quantidade, maximo);
    if (quantidade < maximo) vetor[quantidade++] = raiz->item;
    copiarEmOrdem(raiz->direita, vetor, quantidade, maximo);
}

// Recursiva: tamanho da árvore = 1 (este nó) + os dois lados. O(n).
// Retorno: quantos itens tem no catálogo.
int contarItens(const NoArvore* raiz) {
    if (raiz == nullptr) return 0;
    return 1 + contarItens(raiz->esquerda) + contarItens(raiz->direita);
}

// Recursiva: altura = 1 + o lado mais alto. Árvore vazia tem altura 0. O(n).
int alturaArvore(const NoArvore* raiz) {
    if (raiz == nullptr) return 0;
    int esquerda = alturaArvore(raiz->esquerda);
    int direita = alturaArvore(raiz->direita);
    return 1 + (esquerda > direita ? esquerda : direita);
}

// Apaga a árvore em pós-ordem: primeiro os filhos, depois o pai. Se apagasse
// o pai antes, perderia os ponteiros dos filhos e eles vazariam.
// No fim a raiz vira nullptr.
void liberarArvore(NoArvore*& raiz) {
    if (raiz == nullptr) return;
    liberarArvore(raiz->esquerda);
    liberarArvore(raiz->direita);
    delete raiz;
    raiz = nullptr;
}
