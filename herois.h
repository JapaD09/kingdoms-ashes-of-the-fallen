#ifndef HEROIS_H
#define HEROIS_H

// Heróis tirados na invocação (gacha). Herói repetido ganha estrela.

enum Raridade { COMUM = 1, RARO, EPICO, LENDARIO };

enum PoderHeroi { SEM_PODER, CURA, AURA_ATAQUE, AURA_VIDA, EM_AREA, VAMPIRO, AURA_REAL };

struct ModeloHeroi {
    const char* nome;
    const char* titulo;
    int raridade;
    int vida, ataque, defesa;
    bool distancia;
    int poder;
    const char* descricao;
    int alvos;          // quantos acerta por ataque (0 = 1)
};

const int TOTAL_HEROIS = 9;
const int MAX_ESTRELAS = 5;
const int MAX_EQUIPE = 3;
const int CUSTO_INVOCACAO = 5;     // cristais
const int GARANTIA_EPICO = 10;     // a 10ª invocação sem épico vem épica

struct ColecaoHerois {
    int estrelas[TOTAL_HEROIS] = {};    // 0 = ainda não tem
    bool naEquipe[TOTAL_HEROIS] = {};
    int semEpico = 0;                   // invocações desde o último épico ou lendário
    int invocacoes = 0;
};

const ModeloHeroi& modeloHeroi(int id);
const char* nomeRaridade(int raridade);

// chance em % de cada raridade (comum 60, raro 28, épico 10, lendário 2)
int chanceDaRaridade(int raridade);

// sorteia raridade e herói. Não cobra nada. Devolve o id do herói.
int invocarHeroi(ColecaoHerois& colecao);

int tamanhoDaEquipe(const ColecaoHerois& colecao);

// põe ou tira da equipe. "" se deu certo
const char* alternarEquipe(ColecaoHerois& colecao, int id);

// atributos com o bônus das estrelas (+25% por estrela extra)
int vidaDoHeroi(const ColecaoHerois& colecao, int id);
int ataqueDoHeroi(const ColecaoHerois& colecao, int id);

#endif
