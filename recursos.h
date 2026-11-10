#ifndef RECURSOS_H
#define RECURSOS_H

struct Recursos {
    int ouro = 0;
    int madeira = 0;
    int pedra = 0;
    int comida = 0;
    int ferro = 0;
};

// destino += valor * vezes (vezes = -1 pra cobrar)
void somar(Recursos& destino, const Recursos& valor, int vezes = 1);
bool temRecursos(const Recursos& estoque, const Recursos& custo);

#endif
