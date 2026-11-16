// Simulador de balanceamento: joga centenas de partidas sozinho, com três tipos
// de jogador, pra comparar opções de economia. Não faz parte do jogo.
//
// g++ -std=c++17 -O2 simulador.cpp jogo.cpp reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp
//     expedicao.cpp estruturas.cpp algoritmos.cpp aleatorio.cpp -o simulador

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "aleatorio.h"
#include "economia.h"
#include "jogo.h"

enum Jogador { PARADO, CASUAL, ESPERTO, DEFENSOR };   // defensor = esperto que nunca sai em expedição

const int LIMITE_DE_TURNOS = 60;
const int PARTIDAS = 40;

struct Resultado {
    int vitorias = 0;
    int somaTurnos = 0;         // turno em que a partida acabou
    int cartasJogadas = 0;
    int turnosJogados = 0;
    int chefes = 0;
    int turnosDasVitorias = 0;  // soma do turno final, só das partidas ganhas
    double sobraOuro = 0;       // ouro parado depois de jogar (turnos 4 a 20)
    double naoPagou = 0;        // cartas que ficaram na mão com ouro sobrando mas sem material (turnos 4 a 20)
    int turnosMedidos = 0;
    int defesas = 0, defesasVencidas = 0;   // turnos 6 a 20
    int chefesNoTurno3 = 0;
};

// ---------- expedição ----------

int vidaDoGrupo(const Expedicao& e) {
    int vida = 0, maxima = 0;
    for (int i = 0; i < e.tamanho; i++) {
        vida += e.grupo[i].vida;
        maxima += e.grupo[i].vidaMax;
    }
    return maxima > 0 ? vida * 100 / maxima : 0;
}

// Bot escolhe o caminho no mapa. O esperto dá nota pra cada tipo de nó conforme a vida do grupo;
// o casual vai no primeiro. Retorno: o índice do caminho.
int escolherCaminho(const Expedicao& e, bool esperto) {
    int melhor = 0, melhorNota = -100;
    int vida = vidaDoGrupo(e);
    for (int i = 0; i < e.atual->totalCaminhos; i++) {
        int tipo = e.atual->caminhos[i]->tipo;
        int nota = 0;
        if (esperto) {
            if (tipo == NO_ACAMPAMENTO) nota = vida < 70 ? 10 : 1;
            if (tipo == NO_TESOURO) nota = 6;
            if (tipo == NO_EVENTO) nota = 4;
            if (tipo == NO_COMBATE) nota = vida > 50 ? 3 : 0;
            if (tipo == NO_ELITE) nota = vida > 80 ? 5 : -5;
        }
        if (nota > melhorNota) {
            melhorNota = nota;
            melhor = i;
        }
    }
    return melhor;
}

// Bot luta até o combate acabar. O esperto usa poção e foge quando está mal.
void lutar(Expedicao& e, bool esperto) {
    while (e.fase == EXP_COMBATE) {
        bool alguemMal = false;
        for (int i = 0; i < e.tamanho; i++) {
            if (e.grupo[i].vida > 0 && e.grupo[i].vida * 100 < e.grupo[i].vidaMax * 35) alguemMal = true;
        }
        if (esperto && alguemMal && e.pocoes > 0) {
            agirNoCombate(e, USAR_POCAO, 0);
            continue;
        }
        if (esperto && vidaDoGrupo(e) < 20 && e.atual->tipo != NO_CHEFE) {
            agirNoCombate(e, FUGIR, 0);
            continue;
        }
        int alvo = 0, menor = 99999;                 // bate em quem está mais perto de cair
        for (int i = 0; i < e.totalInimigos; i++) {
            if (e.inimigos[i].vida > 0 && e.inimigos[i].vida < menor) {
                menor = e.inimigos[i].vida;
                alvo = i;
            }
        }
        agirNoCombate(e, ATACAR, esperto ? alvo : 0);
    }
}

// Bot faz uma expedição inteira, do começo até voltar pro reino.
void explorar(Jogo& jogo, bool esperto) {
    if (!partirEmExpedicao(jogo).empty()) return;
    Expedicao& e = jogo.expedicao;
    while (e.fase != EXP_FIM) {
        if (e.fase == EXP_AVISO) continuarExpedicao(e);
        else if (e.fase == EXP_EVENTO) escolherNoEvento(e, (!esperto || vidaDoGrupo(e) > 60) ? 0 : 1);
        else if (e.fase == EXP_COMBATE) lutar(e, esperto);
        else {
            bool chefeAFrente = e.atual->totalCaminhos > 0 && e.atual->caminhos[0]->tipo == NO_CHEFE;
            int vida = vidaDoGrupo(e);
            if (esperto && (vida < 35 || (chefeAFrente && vida < 55))) voltarAoReino(e);
            else avancar(e, escolherCaminho(e, esperto));
        }
    }
    concluirExpedicao(jogo);
}

// ---------- reino ----------

int lotesLivres(const Reino& r) {
    int livres = 0;
    for (int l = 0; l < TAMANHO_GRADE; l++) {
        for (int c = 0; c < TAMANHO_GRADE; c++) {
            if (loteLiberado(r, l, c) && !ehCastelo(l, c) && !buscarEdificioNoLote(r.edificios, l, c)) livres++;
        }
    }
    return livres;
}

// quanto vale construir esse prédio agora (0 = não quero)
int notaDoPredio(const Reino& r, int tipo) {
    Recursos previsao = previsaoDoTurno(r);
    switch (tipo) {
        case QUARTEL: case ARQUEARIA: return 9;
        case ESTABULO: case TORRE_ARCANA: return 10;
        case TORRE: return 7;
        case MERCADO: return 8;
        case CASA: return 6;
        case FAZENDA: return previsao.comida < 3 ? 9 : 0;
        case SERRARIA: return r.estoque.madeira < 50 && contarEdificiosDoTipo(r.edificios, SERRARIA) < 2 ? 7 : 1;
        case MINA: return contarEdificiosDoTipo(r.edificios, MINA) < 1 ? 6 : 1;
        case OFICINA: return 5;
        default: return 3;
    }
}

// tenta a carta i em todos os lotes; true se jogou
bool jogarEmQualquerLote(Reino& r, int i) {
    for (int l = 0; l < TAMANHO_GRADE; l++) {
        for (int c = 0; c < TAMANHO_GRADE; c++) {
            int antes = r.baralho.naMao;
            jogarCarta(r, i, l, c);
            if (r.baralho.naMao < antes) return true;
        }
    }
    return false;
}

// Joga a carta com melhor nota que dá pra pagar. Retorno: false se não tinha nada bom.
bool jogadaEsperta(Reino& r) {
    int melhor = -1, melhorNota = 0;
    for (int i = 0; i < r.baralho.naMao; i++) {
        int carta = r.baralho.mao[i];
        const ModeloCarta& m = modeloCarta(carta);
        if (!temRecursos(r.estoque, custoDaCarta(carta))) continue;      // não enfileira
        int nota = 0;
        if (m.tipo == CARTA_INSTANTE) nota = 20;
        if (m.tipo == CARTA_PREDIO || m.tipo == CARTA_TROPA) nota = notaDoPredio(r, m.alvo);
        if (m.tipo == CARTA_ENCANTO) nota = 6;
        if (m.tipo == CARTA_EXPANSAO) nota = lotesLivres(r) == 0 ? 12 : 0;
        if (nota > melhorNota) {
            melhorNota = nota;
            melhor = i;
        }
    }
    return melhor >= 0 && jogarEmQualquerLote(r, melhor);
}

// Joga as cartas de um turno conforme o tipo de jogador. Retorno: quantas cartas jogou.
int jogarTurno(Jogo& jogo, int jogador) {
    Reino& r = jogo.reino;
    int jogadas = 0;
    if (jogador == CASUAL) {
        for (int i = r.baralho.naMao - 1; i >= 0; i--) {
            if (temRecursos(r.estoque, custoDaCarta(r.baralho.mao[i])) && jogarEmQualquerLote(r, i)) jogadas++;
        }
    }
    if (jogador == ESPERTO || jogador == DEFENSOR) {
        while (jogadaEsperta(r)) jogadas++;
        // sobrou ouro e nada pra jogar: troca a mão (até 2 vezes) e tenta de novo
        while (r.trocasDeMao < 2 && r.estoque.ouro >= precoDaTroca(r) + 50) {
            trocarMao(r);
            while (jogadaEsperta(r)) jogadas++;
        }
        if (r.estoque.ouro > 90) recrutarGuardas(r);
    }
    if (jogador == CASUAL && jogadas == 0 && r.estoque.ouro >= precoDaTroca(r) + 40) {
        trocarMao(r);
        for (int i = r.baralho.naMao - 1; i >= 0; i--) {
            if (temRecursos(r.estoque, custoDaCarta(r.baralho.mao[i])) && jogarEmQualquerLote(r, i)) jogadas++;
        }
    }
    if (jogador != PARADO) {
        int heroi;
        while (r.cristais >= CUSTO_INVOCACAO) invocar(r, heroi);
        int minimo = jogador == ESPERTO ? 2 : 1;
        if (jogador != DEFENSOR && tamanhoDaEquipe(r.herois) >= minimo) explorar(jogo, jogador == ESPERTO);
    }
    return jogadas;
}

// Entre os turnos 4 e 20: quanto ouro sobra e quantas cartas ficaram na mão
// porque, mesmo com ouro, faltou madeira, pedra ou ferro.
static void medirSobras(Resultado& res, const Jogo& jogo) {
    res.turnosMedidos++;
    res.sobraOuro += jogo.reino.estoque.ouro;
    const Recursos& tem = jogo.reino.estoque;
    for (int i = 0; i < jogo.reino.baralho.naMao; i++) {
        Recursos c = custoDaCarta(jogo.reino.baralho.mao[i]);
        bool faltaMaterial = tem.madeira < c.madeira || tem.pedra < c.pedra || tem.ferro < c.ferro;
        if (tem.ouro >= c.ouro && faltaMaterial) res.naoPagou += 1;
    }
}

// Joga PARTIDAS partidas inteiras com o estilo de jogador pedido.
// Retorno: as somas de tudo que foi medido (vitórias, turnos, defesas, chefes...).
Resultado simular(int jogador) {
    Resultado res;
    for (int semente = 1; semente <= PARTIDAS; semente++) {
        Jogo jogo;
        iniciarJogo(jogo, semente * 31 + jogador);
        while (!jogo.derrota && !jogo.vitoria && jogo.turno < LIMITE_DE_TURNOS) {
            int turno = jogo.turno;
            res.cartasJogadas += jogarTurno(jogo, jogador);
            res.turnosJogados++;
            if (turno >= 4 && turno <= 20) medirSobras(res, jogo);
            if (turno == 3) res.chefesNoTurno3 += chefesDerrotados(jogo);
            if (jogo.vitoria) break;
            encerrarTurno(jogo);
            concluirBatalha(jogo);
            if (turno >= 6 && turno <= 20) {
                res.defesas++;
                res.defesasVencidas += jogo.batalha.vitoria;
            }
            if (jogo.escolhendoCarta) escolherRecompensa(jogo, jogador == PARADO ? -1 : 0);
        }
        res.vitorias += jogo.vitoria;
        if (jogo.vitoria) res.turnosDasVitorias += jogo.turno;
        res.somaTurnos += jogo.turno;
        res.chefes += chefesDerrotados(jogo);
        encerrarJogo(jogo);
    }
    return res;
}

// Nota de uma economia. O que eu quero ver numa partida:
//   - quem joga bem ganha na maioria das vezes, mas não sempre (meta: 65%)
//   - quem joga de qualquer jeito ganha pouco (meta: 22%)
//   - a vitória sai lá pelo turno 24, e o primeiro chefe não cai nos 3 primeiros turnos
//   - dá pra jogar umas 2 cartas por turno e quase nunca a mão trava por falta de material
//   - o ouro não fica sobrando: se sobra, é porque não tem onde gastar
//   - a defesa do castelo é vencida quase sempre (meta: 88%), mas não sempre
double distancia(double valor, double meta) {
    return valor > meta ? valor - meta : meta - valor;
}

// Retorno: quanto o valor passou do limite (0 se não passou).
double acima(double valor, double limite) {
    return valor > limite ? valor - limite : 0;
}

// Dá uma nota pra economia testada: perde pontos quanto mais longe dos alvos de vitória,
// duração da partida e uso de cartas. Retorno: a nota (100 é perfeito).
double nota(const Resultado r[]) {
    const Resultado& e = r[ESPERTO];
    double vEsperto = e.vitorias * 100.0 / PARTIDAS, vCasual = r[CASUAL].vitorias * 100.0 / PARTIDAS;
    double cartas = e.cartasJogadas * 1.0 / e.turnosJogados;
    double turnoVitoria = e.vitorias > 0 ? e.turnosDasVitorias * 1.0 / e.vitorias : 60;
    double medidos = e.turnosMedidos > 0 ? e.turnosMedidos : 1;
    double defesa = e.defesas > 0 ? e.defesasVencidas * 100.0 / e.defesas : 0;
    double n = 100;
    n -= distancia(vEsperto, 65) * 1.0;
    n -= distancia(vCasual, 22) * 0.6;
    n -= distancia(turnoVitoria, 24) * 1.5;
    n -= distancia(r[DEFENSOR].somaTurnos * 1.0 / PARTIDAS, 28) * 0.5;
    n -= acima(2.0, cartas) * 20;
    n -= acima(e.sobraOuro / medidos, 100) / 10;
    n -= acima(e.naoPagou / medidos, 0.3) * 20;
    n -= acima(e.chefesNoTurno3 * 1.0 / PARTIDAS, 0.4) * 30;
    n -= distancia(defesa, 88) * 0.5;
    return n;
}

// Roda os 4 jogadores com a economia e imprime uma linha com os números e a nota.
void mostrar(const char* nome, const Economia& e) {
    economia() = e;
    aplicarEconomia();
    Resultado r[4];
    for (int j = 0; j < 4; j++) r[j] = simular(j);
    const Resultado& x = r[ESPERTO];
    double medidos = x.turnosMedidos > 0 ? x.turnosMedidos : 1;
    printf("%-6s custo %3d prod %3d renda %d onda %d/%2d chefe %2d base %d saque %2d porInim %d nivel %3d | defensor T%4.1f | casual %3d%% | esperto %3d%% vitoria T%4.1f chefes %.1f t3 %.2f | cartas %.2f travou %.2f sobra %4.0f defesa %2.0f%% | nota %5.1f\n",
           nome, e.custoPct, e.producaoPct, e.rendaMadeira, e.ondaPorTurno, e.ondaDivisor, e.forcaPorChefe, e.forcaBase, e.saquePct,
           e.ouroPorInimigo, e.nivelPct, r[DEFENSOR].somaTurnos * 1.0 / PARTIDAS, r[CASUAL].vitorias * 100 / PARTIDAS,
           x.vitorias * 100 / PARTIDAS, x.vitorias ? x.turnosDasVitorias * 1.0 / x.vitorias : 0.0, x.chefes * 1.0 / PARTIDAS,
           x.chefesNoTurno3 * 1.0 / PARTIDAS, x.cartasJogadas * 1.0 / x.turnosJogados, x.naoPagou / medidos, x.sobraOuro / medidos,
           x.defesas ? x.defesasVencidas * 100.0 / x.defesas : 0.0, nota(r));
}

// --- Rastro: média do que acontece em cada turno (pra ver ONDE a economia aperta) ---

const int RASTRO_PARTIDAS = 200, RASTRO_TURNOS = 41, RASTRO_COLUNAS = 16;
static double soma[RASTRO_TURNOS][RASTRO_COLUNAS];
static int vivos[RASTRO_TURNOS];
static int faltou[5];        // ouro, madeira, pedra, comida, ferro

// Estoque e previsão no começo do turno (colunas 0 a 6).
static void anotarInicio(double* s, const Reino& r) {
    Recursos p = previsaoDoTurno(r);
    s[0] += r.estoque.ouro; s[1] += r.estoque.madeira; s[2] += r.estoque.pedra;
    s[3] += r.estoque.comida; s[4] += r.estoque.ferro; s[5] += p.ouro; s[6] += p.comida;
}

// Cartas que sobraram na mão sem dar pra pagar, e qual recurso faltou (coluna 8).
static void anotarFaltas(double* s, const Reino& r) {
    for (int i = 0; i < r.baralho.naMao; i++) {
        Recursos c = custoDaCarta(r.baralho.mao[i]);
        if (temRecursos(r.estoque, c)) continue;
        s[8] += 1;
        if (r.estoque.ouro < c.ouro) faltou[0]++;
        if (r.estoque.madeira < c.madeira) faltou[1]++;
        if (r.estoque.pedra < c.pedra) faltou[2]++;
        if (r.estoque.comida < c.comida) faltou[3]++;
        if (r.estoque.ferro < c.ferro) faltou[4]++;
    }
}

// Fecha o turno e anota a batalha e o estado do reino (colunas 10 a 15).
static void anotarFimDoTurno(double* s, Jogo& jogo) {
    encerrarTurno(jogo);
    s[10] += vivosDoLado(jogo.batalha, ALIADO);
    s[11] += vivosDoLado(jogo.batalha, INIMIGO);
    concluirBatalha(jogo);
    s[12] += jogo.batalha.vitoria;
    s[13] += jogo.reino.vidaCastelo;
    s[14] += jogo.reino.populacao;
    s[15] += chefesDerrotados(jogo);
    if (jogo.escolhendoCarta) escolherRecompensa(jogo, 0);
}

// Uma tabela com a média de cada coluna, turno a turno.
static void imprimirRastro() {
    printf("turno vivos | ouro mad pedra comida ferro | +ouro +comida | jogou naoPagou sobraOuro | tropas inimigos vence%% castelo pop chefes\n");
    for (int t = 1; t < RASTRO_TURNOS; t += (t < 12 ? 1 : 3)) {
        if (vivos[t] == 0) break;
        double v = vivos[t];
        double* s = soma[t];
        printf("%3d %5d | %4.0f %4.0f %4.0f %4.0f %4.0f | %4.0f %4.0f | %4.1f %4.1f %5.0f | %4.1f %4.1f %4.0f %5.0f %4.0f %4.1f\n", t, vivos[t],
               s[0]/v, s[1]/v, s[2]/v, s[3]/v, s[4]/v, s[5]/v, s[6]/v, s[7]/v, s[8]/v, s[9]/v, s[10]/v, s[11]/v, s[12]*100/v, s[13]/v, s[14]/v, s[15]/v);
    }
    printf("faltou pra pagar carta: ouro %d  madeira %d  pedra %d  comida %d  ferro %d\n", faltou[0], faltou[1], faltou[2], faltou[3], faltou[4]);
}

// Joga RASTRO_PARTIDAS partidas com o estilo pedido e imprime a média turno a turno.
void rastro(int jogador) {
    for (int semente = 1; semente <= RASTRO_PARTIDAS; semente++) {
        Jogo jogo;
        iniciarJogo(jogo, semente * 31 + jogador);
        while (!jogo.derrota && !jogo.vitoria && jogo.turno < RASTRO_TURNOS) {
            double* s = soma[jogo.turno];
            vivos[jogo.turno]++;
            anotarInicio(s, jogo.reino);
            s[7] += jogarTurno(jogo, jogador);
            anotarFaltas(s, jogo.reino);
            s[9] += jogo.reino.estoque.ouro;                      // ouro parado depois de jogar
            if (jogo.vitoria) break;
            anotarFimDoTurno(s, jogo);
        }
        encerrarJogo(jogo);
    }
    imprimirRastro();
}
// "rastro <jogador> [custo mat prod ondaT ondaD chefe base renda saque venda porInimigo nivel]":
// troca os números da economia pelos da linha de comando e imprime o rastro turno a turno.
static void modoRastro(int argc, char** argv) {
    Economia& e = economia();
    int* campos[] = {&e.custoPct, &e.materiaisPct, &e.producaoPct, &e.ondaPorTurno, &e.ondaDivisor, &e.forcaPorChefe,
                     &e.forcaBase, &e.rendaMadeira, &e.saquePct, &e.vendaPct, &e.ouroPorInimigo, &e.nivelPct};
    for (int i = 3; i < argc && i - 3 < 12; i++) *campos[i - 3] = atoi(argv[i]);
    aplicarEconomia();
    rastro(atoi(argv[2]));
}

// Compara a economia de antes da reestruturação com a atual, lado a lado.
static void compararEconomias() {
    Economia antiga;
    antiga.custoPct = 70;
    antiga.forcaPorChefe = 45;
    antiga.forcaBase = 0;
    antiga.rendaMadeira = 0;
    antiga.saquePct = 100;
    antiga.vendaPct = 100;
    antiga.ouroPorInimigo = 3;
    antiga.nivelPct = 0;
    for (int& v : antiga.chefeVidaPct) v = 100;
    mostrar("antiga", antiga);
    mostrar("atual", Economia());
}

// Uma economia candidata da varredura (os campos que não variam ficam fixos).
static Economia candidata(int custo, int renda, const int onda[2], int chefe, int base, int saque, int inim, int nivel) {
    Economia e;
    e.custoPct = custo;
    e.producaoPct = 130;
    e.materiaisPct = 150;
    e.rendaMadeira = renda;
    e.ondaPorTurno = onda[0];
    e.ondaDivisor = onda[1];
    e.forcaPorChefe = chefe;
    e.forcaBase = base;
    e.saquePct = saque;
    e.vendaPct = 20;
    e.ouroPorInimigo = inim;
    e.nivelPct = nivel;
    return e;
}

// "varrer [preço]": testa todas as combinações. O preço (0, 1 ou 2) permite rodar 3 processos juntos.
static void varrerEconomias(int so) {
    const int custos[] = {70, 85, 100}, rendas[] = {4, 6, 8}, chefes[] = {25, 30}, bases[] = {3, 4};
    const int saques[] = {25, 40}, porInimigo[] = {1, 2}, niveis[] = {100, 150};
    const int ondas[][2] = {{3, 10}, {3, 14}, {2, 12}};
    for (int c = 0; c < 3; c++) {
        if (so >= 0 && so != c) continue;
        for (int renda : rendas)
            for (const auto& onda : ondas)
                for (int chefe : chefes)
                    for (int base : bases)
                        for (int saque : saques)
                            for (int inim : porInimigo)
                                for (int nivel : niveis)
                                    mostrar("", candidata(custos[c], renda, onda, chefe, base, saque, inim, nivel));
    }
}

// Sem argumento compara a economia antiga com a atual; "varrer" e "rastro" são os modos de calibrar.
int main(int argc, char** argv) {
    if (argc >= 3 && strcmp(argv[1], "rastro") == 0) modoRastro(argc, argv);
    else if (argc >= 2 && strcmp(argv[1], "varrer") == 0) varrerEconomias(argc >= 3 ? atoi(argv[2]) : -1);
    else compararEconomias();
    return 0;
}
