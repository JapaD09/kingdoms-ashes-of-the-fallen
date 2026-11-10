#include "combate.h"
#include "inimigos.h"
#include "aleatorio.h"

using namespace std;

namespace {

const int DANO_TORRE = 6;

// vida, ataque, defesa, de longe, alvos
Lutador criarTropa(int tropa, int vida, int ataque, int defesa, bool distancia, int alvos) {
    Lutador l;
    l.tropa = tropa;
    l.vida = l.vidaMax = vida;
    l.ataque = ataque;
    l.defesa = defesa;
    l.distancia = distancia;
    l.alvos = alvos;
    return l;
}

bool adicionar(Batalha& b, const Lutador& l) {
    if (b.total >= MAX_LUTADORES) return false;
    b.lutadores[b.total++] = l;
    return true;
}

int contarDoLado(const Batalha& b, int lado) {
    int total = 0;
    for (int i = 0; i < b.total; i++) {
        if (b.lutadores[i].lado == lado) total++;
    }
    return total;
}

// bônus que valem pro exército todo (oficina e heróis)
void bonusGerais(const Reino& reino, int& ataque, int& vida) {
    ataque = 0;
    vida = 0;
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        if (no->dados.tipo == OFICINA) ataque += no->dados.nivel;
    }
    for (int h = 0; h < TOTAL_HEROIS; h++) {
        if (!reino.herois.naEquipe[h]) continue;
        int poder = modeloHeroi(h).poder;
        if (poder == AURA_ATAQUE) ataque += 1;
        if (poder == AURA_VIDA) vida += 5;
        if (poder == AURA_REAL) {
            ataque += 2;
            vida += 5;
        }
    }
}

void colocarAliado(Batalha& b, Lutador l, int ataqueExtra, int vidaExtra) {
    if (contarDoLado(b, ALIADO) >= MAX_ALIADOS) return;
    l.lado = ALIADO;
    l.ataque += ataqueExtra;
    l.vidaMax += vidaExtra;
    l.vida = l.vidaMax;
    adicionar(b, l);
}

void montarAliados(Batalha& b, const Reino& reino) {
    int ataque, vida;
    bonusGerais(reino, ataque, vida);

    // heróis vão na frente
    for (int h = 0; h < TOTAL_HEROIS; h++) {
        if (!reino.herois.naEquipe[h]) continue;
        const ModeloHeroi& modelo = modeloHeroi(h);
        Lutador l = criarTropa(-1, vidaDoHeroi(reino.herois, h), ataqueDoHeroi(reino.herois, h),
                               modelo.defesa, modelo.distancia, modelo.alvos > 0 ? modelo.alvos : 1);
        l.heroi = h;
        l.cura = modelo.poder == CURA ? 5 : 0;
        l.vampiro = modelo.poder == VAMPIRO;
        colocarAliado(b, l, 0, 0);
    }
    // tropas dos prédios (voltam inteiras toda batalha)
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        const Edificio& e = no->dados;
        const ModeloEdificio& modelo = modeloEdificio(e.tipo);
        if (modelo.tropa == SEM_TROPA) continue;
        int quantas = modelo.tropasPorNivel * e.nivel + e.bonusTropas;
        for (int i = 0; i < quantas; i++) {
            colocarAliado(b, tropaBase(modelo.tropa), ataque + e.bonusAtaque, vida + e.bonusVida);
        }
    }
    // guardas do castelo
    for (int i = 0; i < reino.soldados; i++) colocarAliado(b, tropaBase(GUARDA), ataque, vida);
}

void montarOnda(Batalha& b, int reino, int turno) {
    const ReinoInimigo& inimigo = reinoInimigo(reino);
    int ciclo = (turno - 1) / 6;                  // a cada volta nos 6 reinos, ficam mais fortes
    int poder = poderDaOnda(turno);
    bool chefe = turnoDeChefe(turno);

    while (poder > 0 && contarDoLado(b, INIMIGO) < MAX_INIMIGOS) {
        int nivel = 0;
        if (chefe) {
            nivel = 2;
        } else if (turno >= 3 && poder >= inimigo.tropas[2].custo && chance(20)) {
            nivel = 2;
        } else if (poder >= inimigo.tropas[1].custo && chance(45)) {
            nivel = 1;
        }
        const TropaInimiga& t = inimigo.tropas[nivel];
        Lutador l = criarTropa(-1, t.vida + t.vida * ciclo / 4, t.ataque + ciclo, t.defesa, t.distancia, 1);
        l.lado = INIMIGO;
        l.nivelInimigo = nivel;
        if (chefe) {
            l.chefe = true;
            l.vidaMax = l.vida = l.vida * 4;
            l.ataque += 3;
            l.alvos = 2;                          // chefe bate em dois de uma vez
            chefe = false;
        }
        adicionar(b, l);
        poder -= t.custo;
    }
}

// alvo vivo aleatório do lado pedido (-1 se não tem)
int sortearAlvo(const Batalha& b, int lado) {
    int vivos[MAX_LUTADORES];
    int total = 0;
    for (int i = 0; i < b.total; i++) {
        if (b.lutadores[i].lado == lado && b.lutadores[i].vida > 0) vivos[total++] = i;
    }
    return total == 0 ? -1 : vivos[sortear(0, total - 1)];
}

void anotarGolpe(Batalha& b, int atacante, int alvo, int dano) {
    if (b.totalGolpes >= MAX_GOLPES) return;
    b.golpes[b.totalGolpes++] = {atacante, alvo, dano};
}

void ferir(Batalha& b, int atacante, int alvo, int ataque) {
    Lutador& vitima = b.lutadores[alvo];
    int dano = calcularDano(ataque, vitima.defesa);
    if (dano > vitima.vida) dano = vitima.vida;
    vitima.vida -= dano;
    anotarGolpe(b, atacante, alvo, dano);

    if (atacante >= 0 && b.lutadores[atacante].vampiro) {
        Lutador& eu = b.lutadores[atacante];
        eu.vida += dano / 2;
        if (eu.vida > eu.vidaMax) eu.vida = eu.vidaMax;
    }
}

// cura quem está mais machucado
void curar(Batalha& b, int curandeiro) {
    int melhor = -1;
    int maiorFalta = 0;
    for (int i = 0; i < b.total; i++) {
        const Lutador& l = b.lutadores[i];
        if (l.lado != ALIADO || l.vida <= 0) continue;
        int falta = l.vidaMax - l.vida;
        if (falta > maiorFalta) {
            maiorFalta = falta;
            melhor = i;
        }
    }
    if (melhor < 0) return;
    int quanto = b.lutadores[curandeiro].cura;
    if (quanto > maiorFalta) quanto = maiorFalta;
    b.lutadores[melhor].vida += quanto;
    anotarGolpe(b, curandeiro, melhor, -quanto);
}

void atacar(Batalha& b, int i, int ladoAlvo) {
    for (int k = 0; k < b.lutadores[i].alvos; k++) {
        int alvo = sortearAlvo(b, ladoAlvo);
        if (alvo < 0) return;
        ferir(b, i, alvo, b.lutadores[i].ataque);
    }
}

void vezDosAliados(Batalha& b) {
    for (int t = 0; t < b.tirosDeTorre; t++) {
        int alvo = sortearAlvo(b, INIMIGO);
        if (alvo < 0) return;
        ferir(b, -1, alvo, DANO_TORRE);
    }
    for (int i = 0; i < b.total; i++) {
        const Lutador& l = b.lutadores[i];
        if (l.lado != ALIADO || l.vida <= 0) continue;
        if (l.cura > 0) curar(b, i);
        atacar(b, i, INIMIGO);
    }
}

// sem defensor, cada inimigo bate uma vez no castelo e a batalha acaba
void vezDosInimigos(Batalha& b) {
    bool semDefensor = vivosDoLado(b, ALIADO) == 0;
    for (int i = 0; i < b.total; i++) {
        const Lutador& l = b.lutadores[i];
        if (l.lado != INIMIGO || l.vida <= 0) continue;
        if (semDefensor) {
            int dano = calcularDano(l.ataque, b.defesaCastelo);
            b.danoNoCastelo += dano;
            anotarGolpe(b, i, -1, dano);
        } else {
            atacar(b, i, ALIADO);
        }
    }
    if (semDefensor) {
        b.acabou = true;
        b.vitoria = false;
    }
}

}

int calcularDano(int ataque, int defesa) {
    int dano = ataque - defesa;
    return dano < 1 ? 1 : dano;
}

Lutador tropaBase(int tropa) {
    switch (tropa) {
        case SOLDADO: return criarTropa(SOLDADO, 12, 3, 1, false, 1);
        case ARQUEIRO: return criarTropa(ARQUEIRO, 7, 4, 0, true, 1);
        case CAVALEIRO: return criarTropa(CAVALEIRO, 18, 4, 2, false, 1);
        case MAGO: return criarTropa(MAGO, 8, 5, 0, true, 2);
        default: return criarTropa(GUARDA, 8, 2, 1, false, 1);
    }
}

void montarBatalha(Batalha& batalha, const Reino& reino, int reinoInimigo, int turno) {
    batalha = Batalha();
    batalha.reino = reinoInimigo;
    batalha.turno = turno;
    batalha.defesaCastelo = defesaDoReino(reino);
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        if (no->dados.tipo == TORRE) batalha.tirosDeTorre += no->dados.nivel;
    }
    montarAliados(batalha, reino);
    montarOnda(batalha, reinoInimigo, turno);
}

int vivosDoLado(const Batalha& batalha, int lado) {
    int total = 0;
    for (int i = 0; i < batalha.total; i++) {
        if (batalha.lutadores[i].lado == lado && batalha.lutadores[i].vida > 0) total++;
    }
    return total;
}

void rodadaDeBatalha(Batalha& b) {
    if (b.acabou) return;
    b.totalGolpes = 0;
    b.rodada++;

    vezDosAliados(b);
    if (vivosDoLado(b, INIMIGO) == 0) {
        b.acabou = true;
        b.vitoria = true;
        return;
    }
    vezDosInimigos(b);
    if (!b.acabou && b.rodada >= MAX_RODADAS) {   // empate longo conta como derrota
        b.acabou = true;
        b.vitoria = false;
    }
}

void recompensaDaBatalha(const Batalha& b, int& ouro, int& cristais) {
    ouro = 15 + 3 * contarDoLado(b, INIMIGO);
    cristais = 2 + b.turno / 4 + (turnoDeChefe(b.turno) ? 3 : 0);
}

string aplicarResultado(const Batalha& b, Reino& reino) {
    // guardas que caíram não voltam
    int guardasVivos = 0;
    for (int i = 0; i < b.total; i++) {
        const Lutador& l = b.lutadores[i];
        if (l.lado == ALIADO && l.tropa == GUARDA && l.heroi < 0 && l.vida > 0) guardasVivos++;
    }
    int guardasNaBatalha = 0;
    for (int i = 0; i < b.total; i++) {
        if (b.lutadores[i].lado == ALIADO && b.lutadores[i].tropa == GUARDA) guardasNaBatalha++;
    }
    int perdidos = guardasNaBatalha - guardasVivos;
    reino.soldados -= perdidos;

    reino.vidaCastelo -= b.danoNoCastelo;
    if (reino.vidaCastelo < 0) reino.vidaCastelo = 0;

    if (b.vitoria) {
        int ouro, cristais;
        recompensaDaBatalha(b, ouro, cristais);
        reino.estoque.ouro += ouro;
        reino.cristais += cristais;
        reino.vidaCastelo += 5;                   // reparos depois da vitória
        if (reino.vidaCastelo > VIDA_CASTELO) reino.vidaCastelo = VIDA_CASTELO;
        string texto = "Vitória! +" + to_string(ouro) + " ouro, +" + to_string(cristais) + " cristais.";
        if (perdidos == 1) texto += " (-1 guarda)";
        if (perdidos > 1) texto += " (-" + to_string(perdidos) + " guardas)";
        return texto;
    }
    int saque = reino.estoque.ouro / 5;
    reino.estoque.ouro -= saque;
    return "Derrota: castelo -" + to_string(b.danoNoCastelo) + ", saquearam " + to_string(saque) + " de ouro.";
}
