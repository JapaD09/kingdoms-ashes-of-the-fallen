#ifndef JOGO_H
#define JOGO_H

#include <string>
#include "reino.h"
#include "combate.h"
#include "inimigos.h"

struct Jogo {
    std::string nomeReino = "Ashenfall";
    int turno = 1;
    bool rodando = true;
    bool derrota = false;           // castelo caiu
    Reino reino;
    NoArvore* catalogo = nullptr;   // itens do jogo, por id
    int ordemInimigos[TOTAL_REINOS];
    bool emBatalha = false;
    Batalha batalha;
};

void iniciarJogo(Jogo& jogo, unsigned semente = 7);

// coloca os itens no catálogo (árvore binária)
void montarCatalogo(NoArvore*& catalogo);

// libera a memória da partida
void encerrarJogo(Jogo& jogo);

// quem ataca no fim deste turno
int reinoDaVez(const Jogo& jogo);

// produção, fila e prepara a batalha. devolve o resumo da economia
std::string encerrarTurno(Jogo& jogo);

// depois da batalha: aplica o resultado, compra mão nova e passa o turno
std::string concluirBatalha(Jogo& jogo);

#endif
