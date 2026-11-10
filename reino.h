#ifndef REINO_H
#define REINO_H

#include <string>
#include "estruturas.h"
#include "recursos.h"
#include "cartas.h"
#include "herois.h"

enum TipoEdificio {
    FAZENDA, SERRARIA, QUARTEL, MURALHA, OFICINA,
    MINA, MERCADO, CASA, TORRE, ARQUEARIA, ESTABULO, TORRE_ARCANA,
    TOTAL_TIPOS_EDIFICIO
};

// guarda = soldado do castelo (botão Recrutar), o resto sai dos prédios de tropa
enum TipoTropa { GUARDA, SOLDADO, ARQUEIRO, CAVALEIRO, MAGO, TOTAL_TROPAS };

const int SEM_TROPA = -1;

struct ModeloEdificio {
    const char* nome;
    Recursos custo;
    Recursos producao;      // por nível
    int defesa;             // por nível
    int tropa;              // tipo de tropa que sai dele (ou SEM_TROPA)
    int tropasPorNivel;
    int moradores;          // gente que chega a cada nível
};

// grade 5x5, castelo no meio
const int TAMANHO_GRADE = 5;
const int NIVEL_MAXIMO = 3;
const int VIDA_CASTELO = 100;

struct Reino {
    Recursos estoque;
    int populacao = 0;
    int soldados = 0;               // guardas do castelo
    int cristais = 0;               // pra invocar heróis
    int vidaCastelo = VIDA_CASTELO;
    bool liberado[TAMANHO_GRADE][TAMANHO_GRADE] = {};
    ListaEdificios edificios;
    int proximoIdEdificio = 1;
    PilhaAcoes historico;       // o que dá pra desfazer neste turno
    FilaConstrucao ordens;      // construções esperando recurso
    Baralho baralho;
    ColecaoHerois herois;
};

void iniciarReino(Reino& reino);
void liberarReino(Reino& reino);

const ModeloEdificio& modeloEdificio(int tipo);

bool loteLiberado(const Reino& reino, int linha, int coluna);
bool ehCastelo(int linha, int coluna);

// soma das muralhas
int defesaDoReino(const Reino& reino);

// devolve "" se pode construir
std::string validarConstrucao(const Reino& reino, int tipo, int linha, int coluna);
std::string construirEdificio(Reino& reino, int tipo, int linha, int coluna);
std::string demolirEdificio(Reino& reino, int linha, int coluna);

// carta da mão no lote (prédio, tropa, encanto, expansão ou instante)
std::string jogarCarta(Reino& reino, int indiceMao, int linha, int coluna);

// desfaz a última jogada do turno (pilha)
std::string desfazerUltimaAcao(Reino& reino);

// coloca uma construção na fila pra quando tiver recurso
std::string agendarConstrucao(Reino& reino, int tipo, int linha, int coluna);

// tenta construir as ordens da fila, na ordem em que entraram
std::string processarFila(Reino& reino);

// 2 guardas por 20 ouro, precisa de quartel
std::string recrutarGuardas(Reino& reino);

// gasta cristais e tira um herói. heroi = id sorteado (-1 se não deu)
std::string invocar(Reino& reino, int& heroi);

// quanto muda no próximo turno
Recursos previsaoDoTurno(const Reino& reino);

std::string processarTurnoReino(Reino& reino);

#endif
