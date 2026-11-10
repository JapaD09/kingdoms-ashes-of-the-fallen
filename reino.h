#ifndef REINO_H
#define REINO_H

#include <string>
#include "estruturas.h"

struct Recursos {
    int ouro = 0;
    int madeira = 0;
    int pedra = 0;
    int comida = 0;
    int ferro = 0;
};

enum TipoEdificio { FAZENDA, SERRARIA, QUARTEL, MURALHA, OFICINA, TOTAL_TIPOS_EDIFICIO };

struct ModeloEdificio {
    const char* nome;
    Recursos custo;
    Recursos producao;
    int defesa;
};

// grade 5x5, castelo no meio
const int TAMANHO_GRADE = 5;

struct Reino {
    Recursos estoque;
    int populacao = 0;
    int soldados = 0;
    int defesa = 0;
    ListaEdificios edificios;
    int proximoIdEdificio = 1;
    PilhaAcoes historico;       // o que dá pra desfazer neste turno
    FilaConstrucao ordens;      // construções esperando recurso
};

void iniciarReino(Reino& reino);
void liberarReino(Reino& reino);

const ModeloEdificio& modeloEdificio(int tipo);

bool loteLiberado(int linha, int coluna);
bool ehCastelo(int linha, int coluna);
bool temRecursos(const Recursos& estoque, const Recursos& custo);

// devolve "" se pode construir
std::string validarConstrucao(const Reino& reino, int tipo, int linha, int coluna);
std::string construirEdificio(Reino& reino, int tipo, int linha, int coluna);
std::string demolirEdificio(Reino& reino, int linha, int coluna);

// desfaz a última construção/demolição do turno (pilha)
std::string desfazerUltimaAcao(Reino& reino);

// coloca uma construção na fila pra quando tiver recurso
std::string agendarConstrucao(Reino& reino, int tipo, int linha, int coluna);

// tenta construir as ordens da fila, na ordem em que entraram
std::string processarFila(Reino& reino);

// quanto muda no próximo turno
Recursos previsaoDoTurno(const Reino& reino);

std::string processarTurnoReino(Reino& reino);

#endif
