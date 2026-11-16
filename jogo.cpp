#include "jogo.h"
#include "aleatorio.h"

using namespace std;

// Começa uma partida nova. Antes de criar o reino, o mapa e o catálogo,
// libera os da partida anterior (é o "Novo jogo" depois da derrota).
// semente: base do sorteio; a mesma semente repete a mesma partida.
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

// Libera toda a memória da partida: lista, pilhas, fila, mapa e árvore.
void encerrarJogo(Jogo& jogo) {
    liberarReino(jogo.reino);
    liberarMapa(jogo.expedicao.mapa);
    liberarArvore(jogo.catalogo);
}

// Retorno: quantos dos 6 chefes já foram derrotados.
int chefesDerrotados(const Jogo& jogo) {
    int total = 0;
    for (bool c : jogo.chefeDerrotado) total += c;
    return total;
}

// As regiões vão na ordem dos reinos: goblins primeiro, o dragão por último.
// Retorno: o índice do primeiro reino com chefe vivo, ou -1 se não sobrou nenhum.
int proximaRegiao(const Jogo& jogo) {
    for (int r = 0; r < TOTAL_REINOS; r++) {
        if (!jogo.chefeDerrotado[r]) return r;
    }
    return -1;
}

// Tenta mandar a equipe de heróis pra região do próximo chefe.
// Retorno: texto vazio se partiu, ou o motivo de não ter partido.
string partirEmExpedicao(Jogo& jogo) {
    if (jogo.emBatalha || jogo.derrota || jogo.vitoria) return "Agora não.";
    int regiao = proximaRegiao(jogo);
    if (regiao < 0) return "Não sobrou nenhum reino inimigo.";
    return iniciarExpedicao(jogo.expedicao, jogo.reino, jogo.catalogo, regiao, chefesDerrotados(jogo));
}

// Fecha a expedição: marca o chefe se ele caiu, leva o saque pro reino (se voltou)
// e confere a vitória da campanha. Retorno: o resumo pra mostrar na tela.
string concluirExpedicao(Jogo& jogo) {
    Expedicao& exp = jogo.expedicao;
    if (!exp.ativa) return "";
    if (exp.desfecho == CHEFE_VENCIDO) jogo.chefeDerrotado[exp.regiao] = true;
    jogo.expedicoesFeitas++;
    string resumo = encerrarExpedicao(exp, jogo.reino);
    if (proximaRegiao(jogo) < 0) jogo.vitoria = true;
    return resumo;
}

// Coloca no baralho a carta escolhida depois da vitória (opcao 0 a 2).
// Qualquer outro valor é "nenhuma". Retorno: a mensagem pro jogador.
string escolherRecompensa(Jogo& jogo, int opcao) {
    if (!jogo.escolhendoCarta) return "";
    jogo.escolhendoCarta = false;
    if (opcao < 0 || opcao > 2) return "Você ficou sem a carta.";
    int carta = jogo.recompensa[opcao];
    ganharCarta(jogo.reino.baralho, carta);
    return string(modeloCarta(carta).nome) + " entrou no baralho. Vem na próxima mão.";
}

// Pontos do resumo final: chefes, vida do castelo, ouro, prédios e bônus de rapidez.
int pontuacao(const Jogo& jogo) {
    int pontos = chefesDerrotados(jogo) * 500 + jogo.reino.vidaCastelo * 5 + jogo.reino.estoque.ouro;
    pontos += jogo.reino.edificios.tamanho * 20;
    if (jogo.vitoria && jogo.turno < 40) pontos += (40 - jogo.turno) * 25;     // quanto mais rápido, melhor
    return pontos;
}

// Insere os 14 itens do jogo na árvore do catálogo. Os ids vão embaralhados de
// propósito pra árvore não virar uma "lista torta" (fica com altura 4).
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

// Retorno: qual dos 6 reinos ataca neste turno (a ordem é sorteada a cada volta).
int reinoDaVez(const Jogo& jogo) {
    return jogo.ordemInimigos[(jogo.turno - 1) % TOTAL_REINOS];
}

// Fim do turno: produção e consumo (uma vez só), fila de construção e montagem
// da batalha contra o reino da vez. Retorno: resumo da produção e da fila.
string encerrarTurno(Jogo& jogo) {
    if (jogo.emBatalha || jogo.derrota) return "";
    string resumo = processarTurnoReino(jogo.reino);
    string fila = processarFila(jogo.reino);
    montarBatalha(jogo.batalha, jogo.reino, reinoDaVez(jogo), jogo.turno);
    jogo.emBatalha = true;
    return fila.empty() ? resumo : resumo + "  " + fila;
}

// Termina a batalha, aplica o resultado e prepara o próximo turno (cartas novas,
// heróis se recuperando, recompensa se venceu). Castelo a 0 = derrota.
// Retorno: o resumo da batalha.
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
