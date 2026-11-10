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

int sortearRaridade(int semEpico) {
    int rolagem = sortear(1, 100);
    int raridade = rolagem <= 2 ? LENDARIO : rolagem <= 12 ? EPICO : rolagem <= 40 ? RARO : COMUM;
    if (semEpico >= GARANTIA_EPICO - 1 && raridade < EPICO) raridade = EPICO;
    return raridade;
}

int sortearHeroiDaRaridade(int raridade) {
    int candidatos[TOTAL_HEROIS];
    int total = 0;
    for (int i = 0; i < TOTAL_HEROIS; i++) {
        if (HEROIS[i].raridade == raridade) candidatos[total++] = i;
    }
    return candidatos[sortear(0, total - 1)];
}

int comEstrelas(int valor, int estrelas) {
    if (estrelas <= 1) return valor;
    return valor + valor * (estrelas - 1) / 4;
}

}

const ModeloHeroi& modeloHeroi(int id) {
    return HEROIS[id];
}

const char* nomeRaridade(int raridade) {
    switch (raridade) {
        case LENDARIO: return "Lendário";
        case EPICO: return "Épico";
        case RARO: return "Raro";
        default: return "Comum";
    }
}

int chanceDaRaridade(int raridade) {
    switch (raridade) {
        case LENDARIO: return 2;
        case EPICO: return 10;
        case RARO: return 28;
        default: return 60;
    }
}

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

int tamanhoDaEquipe(const ColecaoHerois& colecao) {
    int total = 0;
    for (int i = 0; i < TOTAL_HEROIS; i++) {
        if (colecao.naEquipe[i]) total++;
    }
    return total;
}

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

int vidaDoHeroi(const ColecaoHerois& colecao, int id) {
    return comEstrelas(HEROIS[id].vida, colecao.estrelas[id]);
}

int ataqueDoHeroi(const ColecaoHerois& colecao, int id) {
    return comEstrelas(HEROIS[id].ataque, colecao.estrelas[id]);
}
