#ifndef JOGO_H
#define JOGO_H

#include <string>
#include "reino.h"
#include "combate.h"
#include "inimigos.h"
#include "expedicao.h"

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
    Expedicao expedicao;
    bool chefeDerrotado[TOTAL_REINOS] = {};
    int expedicoesFeitas = 0;
    bool vitoria = false;           // os 6 chefes caíram
    bool escolhendoCarta = false;   // venceu a defesa: escolhe 1 de 3 cartas
    int recompensa[3] = {};
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

// região da próxima expedição (-1 se todos os chefes já caíram)
int proximaRegiao(const Jogo& jogo);
int chefesDerrotados(const Jogo& jogo);

// manda os heróis da equipe pra região da vez
std::string partirEmExpedicao(Jogo& jogo);

// depois que a expedição acaba: saque pro reino, marca chefe, confere a vitória
std::string concluirExpedicao(Jogo& jogo);

// pontos da partida, pra tela de resumo
int pontuacao(const Jogo& jogo);

// pega uma das 3 cartas da recompensa (opção fora de 0..2 = não quero nenhuma)
std::string escolherRecompensa(Jogo& jogo, int opcao);

// depois da batalha: aplica o resultado, compra mão nova e passa o turno
std::string concluirBatalha(Jogo& jogo);

#endif
