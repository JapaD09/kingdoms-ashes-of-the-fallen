#ifndef REINO_H
#define REINO_H

// Recursos guardados com seguranca no reino.
// O saque da expedicao fica separado ate o grupo voltar.
struct Recursos {
    int ouro = 0;
    int madeira = 0;
    int pedra = 0;
    int comida = 0;
    int ferro = 0;
};

struct Reino {
    Recursos estoque;
    int populacao = 0;
    int soldados = 0;
};

// Valores de inicio de partida (ainda vamos balancear isso jogando)
void iniciarReino(Reino& reino);

void mostrarRecursos(const Reino& reino);

#endif
