// Simulador de balanceamento: joga centenas de partidas sozinho, com três tipos
// de jogador, pra comparar opções de economia. Não faz parte do jogo.
//
// g++ -std=c++17 -O2 simulador.cpp jogo.cpp reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp
//     expedicao.cpp estruturas.cpp algoritmos.cpp aleatorio.cpp -o simulador

#include <cstdio>
#include <cstring>
#include "aleatorio.h"
#include "economia.h"
#include "jogo.h"

enum Jogador { PARADO, CASUAL, ESPERTO, DEFENSOR };   // defensor = esperto que nunca sai em expedição

const int LIMITE_DE_TURNOS = 60;
const int PARTIDAS = 60;

struct Resultado {
    int vitorias = 0;
    int somaTurnos = 0;         // turno em que a partida acabou
    int cartasJogadas = 0;
    int turnosJogados = 0;
    int chefes = 0;
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
        if (r.estoque.ouro > 90) recrutarGuardas(r);
    }
    if (jogador != PARADO) {
        int heroi;
        while (r.cristais >= CUSTO_INVOCACAO) invocar(r, heroi);
        int minimo = jogador == ESPERTO ? 2 : 1;
        if (jogador != DEFENSOR && tamanhoDaEquipe(r.herois) >= minimo) explorar(jogo, jogador == ESPERTO);
    }
    return jogadas;
}

Resultado simular(int jogador) {
    Resultado res;
    for (int semente = 1; semente <= PARTIDAS; semente++) {
        Jogo jogo;
        iniciarJogo(jogo, semente * 31 + jogador);
        while (!jogo.derrota && !jogo.vitoria && jogo.turno < LIMITE_DE_TURNOS) {
            res.cartasJogadas += jogarTurno(jogo, jogador);
            res.turnosJogados++;
            if (jogo.vitoria) break;
            encerrarTurno(jogo);
            concluirBatalha(jogo);
            if (jogo.escolhendoCarta) escolherRecompensa(jogo, jogador == PARADO ? -1 : 0);
        }
        res.vitorias += jogo.vitoria;
        res.somaTurnos += jogo.turno;
        res.chefes += chefesDerrotados(jogo);
        encerrarJogo(jogo);
    }
    return res;
}

// Nota de uma economia. O que eu quero:
//   - quem joga bem ganha na maioria das vezes, mas não sempre (meta: 65%)
//   - quem joga de qualquer jeito ganha pouco (meta: 20%)
//   - a partida de quem ganha dura uns 24 turnos
//   - só se defendendo dá pra aguentar bastante (meta: turno 30), mas não pra sempre
//   - dá pra jogar umas 2 cartas por turno
double distancia(double valor, double meta) {
    return valor > meta ? valor - meta : meta - valor;
}

double nota(const Resultado r[]) {
    double vEsperto = r[ESPERTO].vitorias * 100.0 / PARTIDAS, vCasual = r[CASUAL].vitorias * 100.0 / PARTIDAS;
    double cartas = r[ESPERTO].cartasJogadas * 1.0 / r[ESPERTO].turnosJogados;
    double n = 100;
    n -= distancia(vEsperto, 65) * 1.0;
    n -= distancia(vCasual, 20) * 0.8;
    n -= distancia(r[ESPERTO].somaTurnos * 1.0 / PARTIDAS, 24) * 1.5;
    n -= distancia(r[DEFENSOR].somaTurnos * 1.0 / PARTIDAS, 30) * 1.0;
    n -= distancia(cartas, 2.0) * 15;
    return n;
}

void mostrar(const char* nome, const Economia& e) {
    economia() = e;
    Resultado r[4];
    for (int j = 0; j < 4; j++) r[j] = simular(j);
    printf("%-6s imp %2d custo %3d mat %3d prod %3d onda %d/%2d chefe %2d | parado T%4.1f | defensor T%4.1f | casual %3d%% T%4.1f | esperto %3d%% T%4.1f ch %.1f cartas %.2f | nota %5.1f\n",
           nome, e.impostoPorDez, e.custoPct, e.materiaisPct, e.producaoPct, e.ondaPorTurno, e.ondaDivisor, e.forcaPorChefe,
           r[0].somaTurnos * 1.0 / PARTIDAS, r[3].somaTurnos * 1.0 / PARTIDAS,
           r[1].vitorias * 100 / PARTIDAS, r[1].somaTurnos * 1.0 / PARTIDAS,
           r[2].vitorias * 100 / PARTIDAS, r[2].somaTurnos * 1.0 / PARTIDAS, r[2].chefes * 1.0 / PARTIDAS,
           r[2].cartasJogadas * 1.0 / r[2].turnosJogados, nota(r));
}

int main(int argc, char** argv) {
    Economia atual;
    if (argc < 2 || strcmp(argv[1], "varrer") != 0) {
        Economia antiga;                 // como era antes do ajuste, pra comparar
        antiga.custoPct = 100;
        antiga.ondaPorTurno = 3;
        antiga.ondaDivisor = 10;
        antiga.forcaPorChefe = 15;
        antiga.materiaisPct = 100;
        antiga.producaoPct = 100;
        mostrar("antiga", antiga);
        mostrar("atual", atual);
        return 0;
    }
    // todas as combinações
    const int impostos[] = {5, 8};
    const int custos[] = {100, 85, 70};
    const int ondas[][2] = {{3, 10}, {2, 16}, {2, 12}};
    const int chefes[] = {30, 45, 55};
    const int materiais[] = {100, 150, 200};
    const int producoes[] = {100, 130, 160};
    for (int imposto : impostos)
        for (int custo : custos)
            for (const auto& onda : ondas)
                for (int chefe : chefes)
                    for (int material : materiais)
                        for (int producao : producoes) {
                            Economia e;
                            e.impostoPorDez = imposto;
                            e.custoPct = custo;
                            e.ondaPorTurno = onda[0];
                            e.ondaDivisor = onda[1];
                            e.forcaPorChefe = chefe;
                            e.materiaisPct = material;
                            e.producaoPct = producao;
                            mostrar("", e);
                        }
    return 0;
}
