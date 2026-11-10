#ifndef CARTAS_H
#define CARTAS_H

#include "estruturas.h"
#include "recursos.h"

// Cartas do jogo, no estilo do 9 Kings: todo turno vem uma mão de 5
// tiradas do monte. Prédios, tropas, encantos e umas cartas especiais.

enum TipoCarta { CARTA_PREDIO, CARTA_TROPA, CARTA_ENCANTO, CARTA_EXPANSAO, CARTA_INSTANTE };

enum Encanto { LAMINAS, ARMADURAS, ESTANDARTE, BENCAO };

enum Instante { CARAVANA };

struct ModeloCarta {
    const char* nome;
    int tipo;            // TipoCarta
    int alvo;            // edifício que ergue, encanto ou instante
    Recursos custo;      // prédio e tropa usam o custo do edifício
    int raridade;        // 1 comum, 2 rara, 3 épica
    const char* efeito;
};

const int TOTAL_CARTAS = 18;
const int TAMANHO_MAO = 5;
const int MAX_USADAS = 12;

const ModeloCarta& modeloCarta(int id);
Recursos custoDaCarta(int id);
const char* nomeTipoCarta(int tipo);

struct Baralho {
    PilhaCartas monte;           // de onde compra
    PilhaCartas descarte;        // volta pro monte quando ele acaba
    int mao[TAMANHO_MAO];
    int naMao = 0;
    int usadas[MAX_USADAS];      // jogadas neste turno (podem voltar com o desfazer)
    int totalUsadas = 0;
};

// 3 cópias de cada comum, 2 de cada rara, 1 de cada épica
void montarBaralho(Baralho& baralho);
void embaralhar(PilhaCartas& pilha);

// completa a mão até 5 (se o monte acabar, embaralha o descarte)
void comprarMao(Baralho& baralho);

// tira da mão e marca como usada. devolve o id da carta (-1 se índice ruim)
int usarCarta(Baralho& baralho, int indice);

// desfazer: a carta usada volta pra mão
bool devolverCarta(Baralho& baralho, int carta);

// fim do turno: mão e usadas vão pro descarte
void descartarTudo(Baralho& baralho);

int totalDeCartas(const Baralho& baralho);
void liberarBaralho(Baralho& baralho);

#endif
