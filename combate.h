#ifndef COMBATE_H
#define COMBATE_H

#include <string>
#include "reino.h"

// Batalha do fim do turno: o exército do reino contra a onda inimiga.
// A lógica fica aqui; a tela só chama uma rodada de cada vez e desenha.

const int MAX_ALIADOS = 30;
const int MAX_INIMIGOS = 24;
const int MAX_LUTADORES = MAX_ALIADOS + MAX_INIMIGOS;
const int MAX_GOLPES = 120;
const int MAX_RODADAS = 40;

enum Lado { ALIADO, INIMIGO };

struct Lutador {
    int lado = ALIADO;
    int tropa = -1;        // TipoTropa (aliado comum)
    int heroi = -1;        // id do herói
    int nivelInimigo = 0;  // 0 fraca, 1 média, 2 forte (inimigo)
    bool chefe = false;
    int vida = 0, vidaMax = 0;
    int ataque = 0, defesa = 0;
    bool distancia = false;
    int alvos = 1;         // quantos acerta por vez
    int cura = 0;          // cura um aliado por rodada
    bool vampiro = false;
};

// um ataque (ou cura) da rodada, pra tela animar
struct Golpe {
    int atacante;          // índice do lutador, ou -1 = torre
    int alvo;              // índice do lutador, ou -1 = castelo
    int dano;              // negativo = cura
};

struct Batalha {
    int reino = 0;                  // qual reino inimigo
    int turno = 1;
    Lutador lutadores[MAX_LUTADORES];
    int total = 0;
    int tirosDeTorre = 0;           // tiros por rodada (soma dos níveis das torres)
    int defesaCastelo = 0;
    Golpe golpes[MAX_GOLPES];       // só da última rodada
    int totalGolpes = 0;
    int rodada = 0;
    bool acabou = false;
    bool vitoria = false;
    int danoNoCastelo = 0;
};

// dano = ataque - defesa, no mínimo 1 (fórmula do guia)
int calcularDano(int ataque, int defesa);

// atributos base de cada tropa do reino
Lutador tropaBase(int tropa);

void montarBatalha(Batalha& batalha, const Reino& reino, int reinoInimigo, int turno);

int vivosDoLado(const Batalha& batalha, int lado);

// uma rodada: torres, aliados, confere, inimigos, confere
void rodadaDeBatalha(Batalha& batalha);

// quanto a vitória rende
void recompensaDaBatalha(const Batalha& batalha, int& ouro, int& cristais);

// aplica perdas e recompensas no reino. devolve o resumo
std::string aplicarResultado(const Batalha& batalha, Reino& reino);

#endif
