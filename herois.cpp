#include "herois.h"
#include "aleatorio.h"

namespace {

const ModeloHeroi HEROIS[TOTAL_HEROIS] = {
    {"Bran", "o Escudeiro", COMUM, 34, 3, 3, false, SEM_PODER, "Aguenta muita\npancada.", 0},
    {"Lia", "a Caçadora", COMUM, 16, 5, 0, true, SEM_PODER, "Atira de longe.", 0},
    {"Tomas", "o Ferreiro", COMUM, 24, 4, 1, false, AURA_ATAQUE, "+1 ataque em\ntodas as tropas.", 0},
    {"Mira", "a Curandeira", RARO, 18, 2, 0, true, CURA, "Cura 5 de vida\npor rodada.", 0},
    {"Kael", "o Lanceiro", RARO, 26, 6, 1, false, EM_AREA, "Acerta 2\ninimigos.", 2},
    {"Sigrid", "a Valquíria", RARO, 30, 5, 2, false, AURA_VIDA, "+5 vida em\ntodas as tropas.", 0},
    {"Oren", "o Arquimago", EPICO, 20, 8, 0, true, EM_AREA, "Bola de fogo:\nacerta 3.", 3},
    {"Draven", "o Cavaleiro Negro", EPICO, 42, 8, 3, false, VAMPIRO, "Cura metade\ndo dano que dá.", 0},
    {"Aurora", "Rainha das Cinzas", LENDARIO, 48, 10, 3, true, AURA_REAL, "+2 ataque e\n+5 vida em todos.", 0},
};

// Sorteia a raridade (60/28/10/2%). Depois de 9 invocações sem épico, a 10ª vem épica.
// semEpico: quantas invocações seguidas vieram sem épico.
int sortearRaridade(int semEpico) {
    int rolagem = sortear(1, 100);
    int raridade = rolagem <= 2 ? LENDARIO : rolagem <= 12 ? EPICO : rolagem <= 40 ? RARO : COMUM;
    if (semEpico >= GARANTIA_EPICO - 1 && raridade < EPICO) raridade = EPICO;
    return raridade;
}

// Retorno: um herói sorteado entre os que têm essa raridade.
int sortearHeroiDaRaridade(int raridade) {
    int candidatos[TOTAL_HEROIS];
    int total = 0;
    for (int i = 0; i < TOTAL_HEROIS; i++) {
        if (HEROIS[i].raridade == raridade) candidatos[total++] = i;
    }
    return candidatos[sortear(0, total - 1)];
}

// Retorno: o valor com +25% por estrela acima da primeira.
int comEstrelas(int valor, int estrelas) {
    if (estrelas <= 1) return valor;
    return valor + valor * (estrelas - 1) / 4;
}

}

// Retorno: os dados fixos do herói (nome, título, raridade, vida, ataque...).
const ModeloHeroi& modeloHeroi(int id) {
    return HEROIS[id];
}

// Retorno: o nome da raridade pra mostrar na tela.
const char* nomeRaridade(int raridade) {
    switch (raridade) {
        case LENDARIO: return "Lendário";
        case EPICO: return "Épico";
        case RARO: return "Raro";
        default: return "Comum";
    }
}

// Retorno: a chance em % de sair essa raridade (as quatro somam 100).
int chanceDaRaridade(int raridade) {
    switch (raridade) {
        case LENDARIO: return 2;
        case EPICO: return 10;
        case RARO: return 28;
        default: return 60;
    }
}

// Uma invocação do gacha: sorteia, soma estrela se o herói já existia
// e coloca na equipe se for novo e tiver vaga. Retorno: o id do herói.
int invocarHeroi(ColecaoHerois& colecao) {
    int raridade = sortearRaridade(colecao.semEpico);
    int id = sortearHeroiDaRaridade(raridade);

    colecao.invocacoes++;
    colecao.semEpico = raridade >= EPICO ? 0 : colecao.semEpico + 1;
    if (colecao.estrelas[id] < MAX_ESTRELAS) colecao.estrelas[id]++;
    // herói novo entra direto se tiver vaga
    if (colecao.estrelas[id] == 1 && tamanhoDaEquipe(colecao) < MAX_EQUIPE) {
        colecao.naEquipe[id] = true;
    }
    return id;
}

// Retorno: quantos heróis estão escalados (no máximo 3).
int tamanhoDaEquipe(const ColecaoHerois& colecao) {
    int total = 0;
    for (int i = 0; i < TOTAL_HEROIS; i++) {
        if (colecao.naEquipe[i]) total++;
    }
    return total;
}

// Escala o herói ou tira da equipe.
// Retorno: texto vazio se deu certo, ou o motivo de não ter dado.
const char* alternarEquipe(ColecaoHerois& colecao, int id) {
    if (id < 0 || id >= TOTAL_HEROIS || colecao.estrelas[id] == 0) return "Você ainda não tem esse herói.";
    if (colecao.naEquipe[id]) {
        colecao.naEquipe[id] = false;
        return "";
    }
    if (tamanhoDaEquipe(colecao) >= MAX_EQUIPE) return "A equipe já tem 3 heróis.";
    colecao.naEquipe[id] = true;
    return "";
}

// Retorno: a vida máxima do herói contando as estrelas.
int vidaDoHeroi(const ColecaoHerois& colecao, int id) {
    return comEstrelas(HEROIS[id].vida, colecao.estrelas[id]);
}

// Retorno: o ataque do herói contando as estrelas.
int ataqueDoHeroi(const ColecaoHerois& colecao, int id) {
    return comEstrelas(HEROIS[id].ataque, colecao.estrelas[id]);
}

// Soma as auras dos heróis que estão na equipe: +1 de ataque, +5 de vida
// ou +2 de ataque e +5 de vida (aura real). ataque e vida já chegam com valor e só aumentam.
void somarAuras(const ColecaoHerois& colecao, int& ataque, int& vida) {
    for (int h = 0; h < TOTAL_HEROIS; h++) {
        if (!colecao.naEquipe[h]) continue;
        int poder = HEROIS[h].poder;
        if (poder == AURA_ATAQUE) ataque += 1;
        if (poder == AURA_REAL) ataque += 2;
        if (poder == AURA_VIDA || poder == AURA_REAL) vida += 5;
    }
}
