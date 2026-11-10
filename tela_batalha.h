#ifndef TELA_BATALHA_H
#define TELA_BATALHA_H

#include <string>
#include "combate.h"

// Tela da batalha do fim do turno. Chama uma rodada da lógica por vez
// e anima os golpes. A lógica em si fica no combate.cpp.

void comecarAnimacaoBatalha(const Batalha& batalha);

// true quando o jogador apertou "Continuar" no fim
bool atualizarTelaBatalha(Batalha& batalha, float dt);

void desenharTelaBatalha(const Batalha& batalha, const Reino& reino, const std::string& resumoDoTurno);

#endif
