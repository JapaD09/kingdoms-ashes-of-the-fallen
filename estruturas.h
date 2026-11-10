#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

// Estruturas feitas na mão, com new e delete.

// Lista encadeada de edifícios (R5)

struct Edificio {
    int id = 0;
    int tipo = 0;
    int linha = 0;
    int coluna = 0;
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

int contarEdificiosDoTipo(const ListaEdificios& lista, int tipo);

// chamar antes de sair ou reiniciar
void liberarEdificios(ListaEdificios& lista);

// Pilha de ações do turno (R6) - LIFO, serve pro "desfazer"

enum TipoAcao { CONSTRUIU, DEMOLIU };

struct Acao {
    int tipo = CONSTRUIU;
    Edificio edificio;     // o que foi construído ou demolido
    int ouroDevolvido = 0; // só na demolição
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

#endif
