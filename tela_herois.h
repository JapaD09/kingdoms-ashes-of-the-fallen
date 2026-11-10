#ifndef TELA_HEROIS_H
#define TELA_HEROIS_H

#include <string>
#include "reino.h"

// Janela dos heróis: coleção, equipe e invocação.

void abrirHerois();
bool heroisAbertos();

// cliques e teclas. devolve mensagem pra mostrar ("" = nada)
std::string atualizarHerois(Reino& reino);

void desenharHerois(const Reino& reino);

#endif
