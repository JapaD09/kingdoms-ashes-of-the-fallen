#include "reino.h"
#include <iostream>

using namespace std;

void iniciarReino(Reino& reino) {
    reino.estoque.ouro = 100;
    reino.estoque.madeira = 60;
    reino.estoque.pedra = 30;
    reino.estoque.comida = 80;
    reino.estoque.ferro = 10;

    reino.populacao = 20;
    reino.soldados = 6;
}

void mostrarRecursos(const Reino& reino) {
    const Recursos& r = reino.estoque;

    cout << "  Ouro: " << r.ouro << "\tMadeira: " << r.madeira << "\n";
    cout << "  Pedra: " << r.pedra << "\tComida: " << r.comida << "\n";
    cout << "  Ferro: " << r.ferro << "\n";
    cout << "  Populacao: " << reino.populacao
         << "\tSoldados: " << reino.soldados << "\n";
}
