#ifndef REINO_H
#define REINO_H

#include <string>
#include "estruturas.h"

// Recursos guardados com segurança no reino.
// O saque da expedição fica separado até o grupo voltar.
struct Recursos {
    int ouro = 0;
    int madeira = 0;
    int pedra = 0;
    int comida = 0;
    int ferro = 0;
};

enum TipoEdificio { FAZENDA, SERRARIA, QUARTEL, MURALHA, OFICINA, TOTAL_TIPOS_EDIFICIO };

// "Ficha" de cada tipo de edifício: quanto custa e o que faz por turno
struct ModeloEdificio {
    const char* nome;
    Recursos custo;
    Recursos producao;
    int defesa;
};

// O reino é uma grade 5x5 com o castelo no meio.
// No começo só o 3x3 central pode ser usado.
const int TAMANHO_GRADE = 5;

struct Reino {
    Recursos estoque;
    int populacao = 0;
    int soldados = 0;
    int defesa = 0;
    ListaEdificios edificios;
    int proximoIdEdificio = 1;
};

void iniciarReino(Reino& reino);
void liberarReino(Reino& reino);

const ModeloEdificio& modeloEdificio(int tipo);

bool loteLiberado(int linha, int coluna);
bool ehCastelo(int linha, int coluna);
bool temRecursos(const Recursos& estoque, const Recursos& custo);

// Validação separada da aplicação: a primeira só olha, a segunda cobra.
// validarConstrucao devolve "" se pode construir, ou o motivo se não pode.
std::string validarConstrucao(const Reino& reino, int tipo, int linha, int coluna);
std::string construirEdificio(Reino& reino, int tipo, int linha, int coluna);
std::string demolirEdificio(Reino& reino, int linha, int coluna);

// Quanto cada recurso vai mudar no próximo fim de turno (produção - consumo)
Recursos previsaoDoTurno(const Reino& reino);

// Aplica produção, impostos e consumo de comida. Devolve um resumo pra tela.
std::string processarTurnoReino(Reino& reino);

#endif
