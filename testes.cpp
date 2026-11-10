// Testes do reino e da lista (sem janela)
// g++ -std=c++17 testes.cpp reino.cpp estruturas.cpp -o testes

#include <iostream>
#include <string>
#include "reino.h"

using namespace std;

static int falhas = 0;

static void conferir(bool condicao, const string& descricao) {
    cout << (condicao ? "  ok    " : "  FALHA ") << descricao << "\n";
    if (!condicao) falhas++;
}

static Edificio edificio(int id, int linha, int coluna) {
    Edificio e;
    e.id = id;
    e.tipo = FAZENDA;
    e.linha = linha;
    e.coluna = coluna;
    return e;
}

static void testarLista() {
    cout << "Lista encadeada\n";
    ListaEdificios lista;
    conferir(!removerEdificio(lista, 1), "remover de lista vazia devolve false");
    conferir(buscarEdificioNoLote(lista, 1, 1) == nullptr, "buscar em lista vazia");

    inserirEdificio(lista, edificio(1, 1, 1));
    inserirEdificio(lista, edificio(2, 1, 2));
    inserirEdificio(lista, edificio(3, 1, 3));
    conferir(lista.tamanho == 3, "tres insercoes");
    conferir(lista.primeiro->dados.id == 1 && lista.ultimo->dados.id == 3, "mantem ordem de construcao");

    conferir(removerEdificio(lista, 1), "remove o primeiro");
    conferir(lista.primeiro->dados.id == 2, "segundo virou o primeiro");
    conferir(removerEdificio(lista, 3), "remove o ultimo");
    conferir(lista.ultimo->dados.id == 2, "ponteiro ultimo foi atualizado");
    conferir(!removerEdificio(lista, 99), "id inexistente devolve false");

    inserirEdificio(lista, edificio(4, 3, 3));
    conferir(lista.ultimo->dados.id == 4, "inserir depois de remover o ultimo");
    liberarEdificios(lista);
    conferir(lista.primeiro == nullptr && lista.tamanho == 0, "liberar zera a lista");
}

static void testarConstrucao() {
    cout << "Construcao\n";
    Reino reino;
    iniciarReino(reino);

    reino.estoque = {30, 20, 0, 0, 0};   // custo exato da fazenda
    construirEdificio(reino, FAZENDA, 1, 1);
    conferir(reino.edificios.tamanho == 1, "constroi com recursos exatos");
    conferir(reino.estoque.ouro == 0 && reino.estoque.madeira == 0, "cobra uma vez so");

    construirEdificio(reino, FAZENDA, 1, 2);
    conferir(reino.edificios.tamanho == 1, "nao constroi sem recursos");

    reino.estoque = {500, 500, 500, 500, 500};
    construirEdificio(reino, SERRARIA, 1, 1);
    conferir(reino.edificios.tamanho == 1, "nao constroi em lote ocupado");
    construirEdificio(reino, SERRARIA, 2, 2);
    conferir(reino.edificios.tamanho == 1, "nao constroi no castelo");
    construirEdificio(reino, SERRARIA, 0, 0);
    conferir(reino.edificios.tamanho == 1, "nao constroi na floresta");

    demolirEdificio(reino, 1, 1);
    conferir(reino.edificios.tamanho == 0 && reino.estoque.ouro == 515, "demolir devolve metade do ouro");
    liberarReino(reino);
}

static void testarTurno() {
    cout << "Turno\n";
    Reino reino;
    iniciarReino(reino);
    reino.estoque.ouro = 200;
    construirEdificio(reino, FAZENDA, 1, 1);
    construirEdificio(reino, SERRARIA, 1, 2);

    Recursos antes = reino.estoque;
    processarTurnoReino(reino);
    // +8 da fazenda - 6 de consumo = +2
    conferir(reino.estoque.comida == antes.comida + 2, "producao e consumo de comida");
    conferir(reino.estoque.madeira == antes.madeira + 6, "serraria produz madeira");
    conferir(reino.estoque.ouro == antes.ouro + 10, "imposto de 1 ouro por 2 habitantes");

    reino.estoque.comida = 0;
    demolirEdificio(reino, 1, 1);
    string msg = processarTurnoReino(reino);
    conferir(reino.estoque.comida == 0, "comida nao fica negativa");
    conferir(reino.populacao < 20, "fome faz gente ir embora");
    liberarReino(reino);
}

int main() {
    testarLista();
    testarConstrucao();
    testarTurno();
    cout << (falhas == 0 ? "\nTudo certo!\n" : "\nTem falha!\n");
    return falhas == 0 ? 0 : 1;
}
