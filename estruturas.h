#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

// Estruturas de dados feitas na mão (R5 a R8), com new e delete.
// Nada de std::list / std::stack / std::queue aqui, a disciplina pede manual.

// ---------------- R5: lista encadeada de edifícios ----------------

// Um edifício construído no reino. O "tipo" aponta pro catálogo em reino.cpp.
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

// Guardamos o último nó também, assim inserir no fim é O(1)
// e a lista fica na ordem em que as coisas foram construídas.
struct ListaEdificios {
    NoEdificio* primeiro = nullptr;
    NoEdificio* ultimo = nullptr;
    int tamanho = 0;
};

void inserirEdificio(ListaEdificios& lista, const Edificio& edificio);

// Remove pelo id. Devolve false se não achou (lista vazia ou id inexistente).
bool removerEdificio(ListaEdificios& lista, int id);

// Percorre a lista procurando quem está naquele lote. nullptr se o lote está vazio.
const Edificio* buscarEdificioNoLote(const ListaEdificios& lista, int linha, int coluna);

int contarEdificiosDoTipo(const ListaEdificios& lista, int tipo);

// Apaga todos os nós. Tem que ser chamada antes de fechar ou reiniciar o jogo.
void liberarEdificios(ListaEdificios& lista);

#endif
