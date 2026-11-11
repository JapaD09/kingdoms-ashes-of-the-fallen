#ifndef TELA_EXPEDICAO_H
#define TELA_EXPEDICAO_H

#include "jogo.h"

// Tela da expedição: mapa de nós, eventos e o combate por ações.
// As regras ficam em expedicao.cpp; aqui é só clique e desenho.

void comecarTelaExpedicao();

// true quando o jogador fechou o resultado final (hora de voltar pro reino)
bool atualizarTelaExpedicao(Jogo& jogo);

void desenharTelaExpedicao(const Jogo& jogo);

#endif
