#include "jogo.h"
#include "aleatorio.h"

using namespace std;

void iniciarJogo(Jogo& jogo, unsigned semente) {
    semearAleatorio(semente);
    jogo.turno = 1;
    jogo.rodando = true;
    jogo.derrota = false;
    jogo.emBatalha = false;
    jogo.vitoria = false;
    jogo.escolhendoCarta = false;
    jogo.expedicoesFeitas = 0;
    for (bool& c : jogo.chefeDerrotado) c = false;
    liberarMapa(jogo.expedicao.mapa);
    jogo.expedicao = Expedicao();
    iniciarReino(jogo.reino);
    sortearOrdemDosReinos(jogo.ordemInimigos);
    liberarArvore(jogo.catalogo);
    montarCatalogo(jogo.catalogo);
}

void encerrarJogo(Jogo& jogo) {
    liberarReino(jogo.reino);
    liberarMapa(jogo.expedicao.mapa);
    liberarArvore(jogo.catalogo);
}

int chefesDerrotados(const Jogo& jogo) {
    int total = 0;
    for (bool c : jogo.chefeDerrotado) total += c;
    return total;
}

// as regiões vão na ordem dos reinos: goblins primeiro, o dragão por último
int proximaRegiao(const Jogo& jogo) {
    for (int r = 0; r < TOTAL_REINOS; r++) {
        if (!jogo.chefeDerrotado[r]) return r;
    }
    return -1;
}

string partirEmExpedicao(Jogo& jogo) {
    if (jogo.emBatalha || jogo.derrota || jogo.vitoria) return "Agora não.";
    int regiao = proximaRegiao(jogo);
    if (regiao < 0) return "Não sobrou nenhum reino inimigo.";
    return iniciarExpedicao(jogo.expedicao, jogo.reino, jogo.catalogo, regiao, chefesDerrotados(jogo));
}

string concluirExpedicao(Jogo& jogo) {
    Expedicao& exp = jogo.expedicao;
    if (!exp.ativa) return "";
    if (exp.desfecho == CHEFE_VENCIDO) jogo.chefeDerrotado[exp.regiao] = true;
    jogo.expedicoesFeitas++;
    string resumo = encerrarExpedicao(exp, jogo.reino);
    if (proximaRegiao(jogo) < 0) jogo.vitoria = true;
    return resumo;
}

string escolherRecompensa(Jogo& jogo, int opcao) {
    if (!jogo.escolhendoCarta) return "";
    jogo.escolhendoCarta = false;
    if (opcao < 0 || opcao > 2) return "Você ficou sem a carta.";
    int carta = jogo.recompensa[opcao];
    ganharCarta(jogo.reino.baralho, carta);
    return string(modeloCarta(carta).nome) + " entrou no baralho. Vem na próxima mão.";
}

int pontuacao(const Jogo& jogo) {
    int pontos = chefesDerrotados(jogo) * 500 + jogo.reino.vidaCastelo * 5 + jogo.reino.estoque.ouro;
    pontos += jogo.reino.edificios.tamanho * 20;
    if (jogo.vitoria && jogo.turno < 40) pontos += (40 - jogo.turno) * 25;     // quanto mais rápido, melhor
    return pontos;
}

// ids embaralhados de propósito pra árvore não virar uma "lista torta"
void montarCatalogo(NoArvore*& catalogo) {
    const Item itens[] = {
        {50, "Espada longa", "arma", 120, 2},     {25, "Arco curto", "arma", 70, 1},
        {75, "Elmo de ferro", "armadura", 90, 2}, {12, "Poção de vida", "poção", 25, 1},
        {37, "Machado", "arma", 80, 1},           {62, "Escudo", "armadura", 60, 1},
        {87, "Cajado rúnico", "arma", 200, 3},    {6, "Pão de viagem", "comida", 8, 1},
        {18, "Poção de força", "poção", 45, 2},   {31, "Lança", "arma", 65, 1},
        {43, "Cota de malha", "armadura", 150, 3}, {56, "Tocha", "ferramenta", 10, 1},
        {68, "Mapa antigo", "ferramenta", 35, 2}, {93, "Coroa das cinzas", "tesouro", 500, 3},
    };
    for (const Item& item : itens) inserirItem(catalogo, item);
}

int reinoDaVez(const Jogo& jogo) {
    return jogo.ordemInimigos[(jogo.turno - 1) % TOTAL_REINOS];
}

string encerrarTurno(Jogo& jogo) {
    if (jogo.emBatalha || jogo.derrota) return "";
    string resumo = processarTurnoReino(jogo.reino);
    string fila = processarFila(jogo.reino);
    montarBatalha(jogo.batalha, jogo.reino, reinoDaVez(jogo), jogo.turno);
    jogo.emBatalha = true;
    return fila.empty() ? resumo : resumo + "  " + fila;
}

string concluirBatalha(Jogo& jogo) {
    if (!jogo.emBatalha) return "";
    while (!jogo.batalha.acabou) rodadaDeBatalha(jogo.batalha);   // se pulou a animação
    string resultado = aplicarResultado(jogo.batalha, jogo.reino);
    jogo.emBatalha = false;

    if (jogo.reino.vidaCastelo <= 0) {
        jogo.derrota = true;
        return resultado;
    }
    if (jogo.batalha.vitoria) {
        sortearRecompensa(jogo.recompensa);
        jogo.escolhendoCarta = true;
    }
    jogo.reino.explorouNoTurno = false;
    if (jogo.reino.feridos > 0) jogo.reino.feridos--;
    descartarTudo(jogo.reino.baralho);
    comprarMao(jogo.reino.baralho);
    jogo.turno++;
    // fechou a volta nos 6 reinos: sorteia outra ordem
    if ((jogo.turno - 1) % TOTAL_REINOS == 0) sortearOrdemDosReinos(jogo.ordemInimigos);
    return resultado;
}
