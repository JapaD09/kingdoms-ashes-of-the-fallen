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

#endif
