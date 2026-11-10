#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#include <string>

// Estruturas feitas na mão, com new e delete.

// Lista encadeada de edifícios (R5)

struct Edificio {
    int id = 0;
    int tipo = 0;
    int linha = 0;
    int coluna = 0;
    int nivel = 1;          // sobe jogando a mesma carta em cima (máx 3)
    int bonusAtaque = 0;    // encantos nas tropas desse prédio
    int bonusVida = 0;
    int bonusTropas = 0;
};

struct NoEdificio {
    Edificio dados;
    NoEdificio* proximo = nullptr;
};

// guarda o último pra inserir no fim
struct ListaEdificios {
    NoEdificio* primeiro = nullptr;
    NoEdificio* ultimo = nullptr;
    int tamanho = 0;
};

void inserirEdificio(ListaEdificios& lista, const Edificio& edificio);

// false se não achou
bool removerEdificio(ListaEdificios& lista, int id);

// nullptr se o lote está vazio
const Edificio* buscarEdificioNoLote(const ListaEdificios& lista, int linha, int coluna);

// versão que deixa alterar (nível, encantos)
Edificio* acharEdificio(ListaEdificios& lista, int id);

int contarEdificiosDoTipo(const ListaEdificios& lista, int tipo);

// chamar antes de sair ou reiniciar
void liberarEdificios(ListaEdificios& lista);

// Pilha de ações do turno (R6) - LIFO, serve pro "desfazer"

enum TipoAcao { CONSTRUIU, DEMOLIU, MELHOROU, EXPANDIU, USOU_CARTA };

struct Acao {
    int tipo = CONSTRUIU;
    Edificio edificio;     // construído, demolido ou como era antes de melhorar
    int ouroDevolvido = 0; // só na demolição
    int carta = -1;        // carta gasta (volta pra mão se desfizer)
};

struct NoPilha {
    Acao acao;
    NoPilha* abaixo = nullptr;
};

struct PilhaAcoes {
    NoPilha* topo = nullptr;
    int tamanho = 0;
};

void empilhar(PilhaAcoes& pilha, const Acao& acao);
bool desempilhar(PilhaAcoes& pilha, Acao& saida);   // false se vazia
const Acao* topoDaPilha(const PilhaAcoes& pilha);
bool pilhaVazia(const PilhaAcoes& pilha);
void limparPilha(PilhaAcoes& pilha);

// Fila de ordens de construção (R7) - FIFO

struct OrdemConstrucao {
    int tipoEdificio = 0;
    int linha = 0;
    int coluna = 0;
};

struct NoFila {
    OrdemConstrucao ordem;
    NoFila* proximo = nullptr;
};

struct FilaConstrucao {
    NoFila* inicio = nullptr;
    NoFila* fim = nullptr;
    int tamanho = 0;
};

void enfileirar(FilaConstrucao& fila, const OrdemConstrucao& ordem);
bool desenfileirar(FilaConstrucao& fila, OrdemConstrucao& saida);   // false se vazia
const OrdemConstrucao* frenteDaFila(const FilaConstrucao& fila);
bool filaVazia(const FilaConstrucao& fila);
bool loteNaFila(const FilaConstrucao& fila, int linha, int coluna);
void limparFila(FilaConstrucao& fila);

// Pilha de cartas (R6 de novo): o monte de compra e o descarte.
// Compra sempre do topo, como num baralho de verdade.

struct NoCarta {
    int carta = 0;
    NoCarta* abaixo = nullptr;
};

struct PilhaCartas {
    NoCarta* topo = nullptr;
    int tamanho = 0;
};

void colocarCarta(PilhaCartas& pilha, int carta);
bool tirarCarta(PilhaCartas& pilha, int& carta);   // false se vazia
void limparCartas(PilhaCartas& pilha);

// Árvore binária de busca do catálogo (R8), organizada pelo id
// menor id vai pra esquerda, maior pra direita

struct Item {
    int id = 0;
    std::string nome;
    std::string tipo;
    int valor = 0;       // preço em ouro
    int raridade = 1;    // 1 comum, 2 raro, 3 lendário
};

struct NoArvore {
    Item item;
    NoArvore* esquerda = nullptr;
    NoArvore* direita = nullptr;
};

// false se o id já existe (não aceita repetido)
bool inserirItem(NoArvore*& raiz, const Item& item);

// recursiva; conta quantos nós olhou. nullptr se não achou
const Item* buscarItem(const NoArvore* raiz, int id, int& comparacoes);

// percurso in-ordem recursivo (R3): enche o vetor em ordem crescente de id
void copiarEmOrdem(const NoArvore* raiz, Item vetor[], int& quantidade, int maximo);

int contarItens(const NoArvore* raiz);
int alturaArvore(const NoArvore* raiz);
void liberarArvore(NoArvore*& raiz);

#endif
