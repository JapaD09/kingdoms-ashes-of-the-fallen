#ifndef EXPEDICAO_H
#define EXPEDICAO_H

#include <string>
#include "reino.h"

// Expedição: os heróis saem do reino e atravessam um mapa de nós até o chefe
// da região. O que acham no caminho é "saque" e só vira recurso do reino se
// o grupo voltar. O combate aqui é por ações (atacar, defender, poção, fugir).

enum TipoNo { NO_ENTRADA, NO_COMBATE, NO_EVENTO, NO_TESOURO, NO_ACAMPAMENTO, NO_ELITE, NO_CHEFE };

const int MAX_CAMINHOS = 3;
const int CAMADAS = 6;              // entrada, 4 camadas no meio e o chefe
const int MAX_NOS = 16;
const int CUSTO_EXPEDICAO = 15;     // comida

// nó do mapa: aponta pra até 3 nós da camada seguinte
struct NoMapa {
    int id = 0;
    int tipo = NO_COMBATE;
    bool visitado = false;
    int camada = 0, posicao = 0;    // onde aparece no desenho
    NoMapa* caminhos[MAX_CAMINHOS] = {};
    int totalCaminhos = 0;
    NoMapa* proximo = nullptr;      // lista com todos os nós, pra liberar depois
};

struct MapaExpedicao {
    NoMapa* entrada = nullptr;
    NoMapa* chefe = nullptr;
    NoMapa* todos = nullptr;
    int total = 0;
};

// cria os nós com new e liga as camadas. Sempre existe caminho até o chefe.
void gerarMapa(MapaExpedicao& mapa);
void liberarMapa(MapaExpedicao& mapa);

// busca em profundidade recursiva. visitados[] (tamanho MAX_NOS) evita repetir nó.
bool existeCaminho(const NoMapa* de, const NoMapa* ate, bool visitados[]);

// todo nó alcança o chefe?
bool mapaValido(const MapaExpedicao& mapa);

struct Explorador {
    int heroi = -1;
    int vida = 0, vidaMax = 0;
    int ataque = 0, defesa = 0;
};

struct InimigoExp {
    int nivel = 0;                  // 0 fraca, 1 média, 2 forte (tropa da região)
    bool chefe = false;
    int vida = 0, vidaMax = 0;
    int ataque = 0, defesa = 0;
};

const int MAX_ITENS_SAQUE = 12;
const int MAX_GRUPO = 3;
const int MAX_INIMIGOS_EXP = 4;

// o que o grupo está carregando (ainda não é do reino)
struct Saque {
    int ouro = 0, madeira = 0, pedra = 0, cristais = 0;
    int itens[MAX_ITENS_SAQUE] = {};    // ids do catálogo
    int totalItens = 0;
};

enum FaseExpedicao { EXP_MAPA, EXP_COMBATE, EXP_EVENTO, EXP_AVISO, EXP_FIM };
enum AcaoCombate { ATACAR, DEFENDER, USAR_POCAO, FUGIR };
enum Desfecho { EM_ANDAMENTO, VOLTOU, DERROTADA, CHEFE_VENCIDO };

struct Expedicao {
    bool ativa = false;
    int regiao = 0;                 // reino inimigo dono da região
    int dificuldade = 0;            // quantos chefes já caíram
    const NoArvore* catalogo = nullptr;
    MapaExpedicao mapa;
    NoMapa* atual = nullptr;
    Explorador grupo[MAX_GRUPO];
    int tamanho = 0;
    InimigoExp inimigos[MAX_INIMIGOS_EXP];
    int totalInimigos = 0;
    bool defendendo = false;
    int forcaExtra = 0;             // bênção do altar
    int pocoes = 0;
    int bonusDoCombate = 0;         // ouro extra se vencer (acampamento inimigo)
    Saque saque;
    int fase = EXP_MAPA;
    int evento = 0;
    int desfecho = EM_ANDAMENTO;
    std::string mensagem;           // o que acabou de acontecer
    int rodada = 0;
    int danoNoHeroi[MAX_GRUPO] = {};        // última rodada, pra tela mostrar
    int danoNoInimigo[MAX_INIMIGOS_EXP] = {};
};

// texto de cada evento e das duas escolhas
const int TOTAL_EVENTOS = 5;
const char* tituloDoEvento(int evento);
const char* textoDoEvento(int evento);
const char* escolhaDoEvento(int evento, int opcao);
const char* nomeDoNo(int tipo);

// "" se pode sair em expedição
std::string podeExplorar(const Reino& reino);

// cobra a comida, monta o grupo com os heróis da equipe e gera o mapa
std::string iniciarExpedicao(Expedicao& exp, Reino& reino, const NoArvore* catalogo, int regiao, int dificuldade);

// anda pra um dos caminhos do nó atual e resolve o que tem lá
std::string avancar(Expedicao& exp, int caminho);

void escolherNoEvento(Expedicao& exp, int opcao);

// uma rodada: a ação do jogador e depois os inimigos que sobraram
void agirNoCombate(Expedicao& exp, int acao, int alvo);

// fecha o aviso e volta pro mapa (ou termina, se a expedição acabou)
void continuarExpedicao(Expedicao& exp);

// desiste de seguir em frente e leva o saque pra casa
void voltarAoReino(Expedicao& exp);

int vivosNoGrupo(const Expedicao& exp);
int inimigosVivos(const Expedicao& exp);

// passa o saque pro reino (ou perde tudo), libera o mapa e devolve o resumo
std::string encerrarExpedicao(Expedicao& exp, Reino& reino);

#endif
