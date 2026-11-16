#include "expedicao.h"
#include "aleatorio.h"
#include "combate.h"
#include "economia.h"
#include "inimigos.h"

using namespace std;

namespace {

struct TextoEvento {
    const char* titulo;
    const char* texto;
    const char* arriscar;
    const char* seguro;
};

enum Evento { CARAVANA_ABANDONADA, VIAJANTE_FERIDO, PONTE_DESTRUIDA, ALTAR_ANTIGO, ACAMPAMENTO_INIMIGO };

const TextoEvento EVENTOS[TOTAL_EVENTOS] = {
    {"Caravana abandonada", "Uma carroça virada na estrada.\nPode ter ouro. Ou uma emboscada.", "Vasculhar", "Seguir em frente"},
    {"Viajante ferido", "Um viajante pede ajuda. Dividir\nos mantimentos enfraquece o grupo.", "Ajudar", "Ignorar"},
    {"Ponte destruída", "Dá pra aproveitar as tábuas e pedras,\nmas a travessia machuca.", "Atravessar", "Buscar um desvio"},
    {"Altar antigo", "Um altar pede uma oferenda de\n20 de ouro do saque.", "Oferecer", "Ignorar"},
    {"Acampamento inimigo", "Batedores dormem ao lado de um baú.", "Atacar", "Evitar"},
};

NoMapa* criarNo(MapaExpedicao& mapa, int tipo, int camada, int posicao) {
    NoMapa* no = new NoMapa;
    no->id = mapa.total++;
    no->tipo = tipo;
    no->camada = camada;
    no->posicao = posicao;
    no->proximo = mapa.todos;       // entra no começo da lista de todos
    mapa.todos = no;
    return no;
}

bool jaLigado(const NoMapa* de, const NoMapa* ate) {
    for (int i = 0; i < de->totalCaminhos; i++) {
        if (de->caminhos[i] == ate) return true;
    }
    return false;
}

void ligar(NoMapa* de, NoMapa* ate) {
    if (de->totalCaminhos >= MAX_CAMINHOS || jaLigado(de, ate)) return;
    de->caminhos[de->totalCaminhos++] = ate;
}

// o que tem em cada nó do meio do mapa
int sortearTipo(int camada) {
    int rolagem = sortear(1, 100);
    if (camada == 1) return rolagem <= 60 ? NO_COMBATE : rolagem <= 85 ? NO_EVENTO : NO_TESOURO;
    if (camada == CAMADAS - 2) return rolagem <= 45 ? NO_ACAMPAMENTO : rolagem <= 75 ? NO_COMBATE : NO_ELITE;
    return rolagem <= 40 ? NO_COMBATE : rolagem <= 65 ? NO_EVENTO : rolagem <= 82 ? NO_TESOURO : NO_ELITE;
}

InimigoExp criarInimigo(int regiao, int nivel, int dificuldade) {
    const TropaInimiga& t = reinoInimigo(regiao).tropas[nivel];
    dificuldade += economia().forcaBase;
    InimigoExp i;
    i.nivel = nivel;
    i.vidaMax = i.vida = t.vida * (100 + economia().forcaPorChefe * dificuldade) / 100;
    i.ataque = t.ataque + dificuldade * economia().forcaPorChefe / 30;
    i.defesa = t.defesa;
    return i;
}

void zerarDanos(Expedicao& exp) {
    for (int& d : exp.danoNoHeroi) d = 0;
    for (int& d : exp.danoNoInimigo) d = 0;
}

void comecarCombate(Expedicao& exp, int tipoNo) {
    exp.totalInimigos = 0;
    exp.defendendo = false;
    exp.rodada = 0;
    zerarDanos(exp);
    auto por = [&exp](int nivel) {
        if (exp.totalInimigos < MAX_INIMIGOS_EXP) {
            exp.inimigos[exp.totalInimigos++] = criarInimigo(exp.regiao, nivel, exp.dificuldade);
        }
    };
    if (tipoNo == NO_CHEFE) {
        por(2);
        InimigoExp& chefe = exp.inimigos[0];
        chefe.chefe = true;
        chefe.vidaMax = chefe.vida = chefe.vida * 2 * economia().chefeVidaPct[exp.regiao] / 100;
        chefe.ataque += 1;
        por(0);
        if (exp.dificuldade >= 3) por(1);
    } else if (tipoNo == NO_ELITE) {
        por(2);
        por(0);
    } else {
        int quantos = 2 + (exp.dificuldade >= 2 ? 1 : 0);
        for (int i = 0; i < quantos; i++) por(chance(40) ? 1 : 0);
    }
    exp.fase = EXP_COMBATE;
    exp.mensagem = tipoNo == NO_CHEFE ? string(reinoInimigo(exp.regiao).rei) + " barra o caminho!"
                                      : "Inimigos! Escolha uma ação.";
}

// item aleatório do catálogo até uma raridade (-1 se não achar)
int sortearItem(const NoArvore* catalogo, int raridadeMaxima, int raridadeMinima = 1) {
    Item itens[20];
    int total = 0;
    copiarEmOrdem(catalogo, itens, total, 20);
    int candidatos[20];
    int quantos = 0;
    for (int i = 0; i < total; i++) {
        if (itens[i].raridade <= raridadeMaxima && itens[i].raridade >= raridadeMinima) candidatos[quantos++] = itens[i].id;
    }
    return quantos == 0 ? -1 : candidatos[sortear(0, quantos - 1)];
}

string nomeDoItem(const NoArvore* catalogo, int id) {
    int comparacoes = 0;
    const Item* item = buscarItem(catalogo, id, comparacoes);
    return item ? item->nome : "item";
}

// guarda um item no saque e devolve o nome ("" se não coube)
string acharItem(Expedicao& exp, int raridadeMaxima, int raridadeMinima = 1) {
    int id = sortearItem(exp.catalogo, raridadeMaxima, raridadeMinima);
    if (id < 0 || exp.saque.totalItens >= MAX_ITENS_SAQUE) return "";
    exp.saque.itens[exp.saque.totalItens++] = id;
    return nomeDoItem(exp.catalogo, id);
}

void avisar(Expedicao& exp, const string& texto) {
    exp.mensagem = texto;
    exp.fase = EXP_AVISO;
}

// ouro do saque já com o ajuste da economia
int ouroDoSaque(int valor) {
    return valor * economia().saquePct / 100;
}

void abrirTesouro(Expedicao& exp) {
    int ouro = ouroDoSaque(sortear(30, 50) + 5 * exp.dificuldade);
    int madeira = sortear(10, 18), pedra = sortear(6, 12);
    exp.saque.ouro += ouro;
    exp.saque.madeira += madeira;
    exp.saque.pedra += pedra;
    string texto = "Tesouro: +" + to_string(ouro) + " ouro, +" + to_string(madeira) + " madeira, +" + to_string(pedra) + " pedra";
    string item = acharItem(exp, 2);
    if (!item.empty()) texto += ", " + item;
    if (chance(30)) {
        exp.saque.cristais += 2;
        texto += ", +2 cristais";
    }
    avisar(exp, texto + ".");
}

void descansar(Expedicao& exp) {
    for (int i = 0; i < exp.tamanho; i++) {
        Explorador& e = exp.grupo[i];
        if (e.vida <= 0) continue;
        e.vida += e.vidaMax * 4 / 10;
        if (e.vida > e.vidaMax) e.vida = e.vidaMax;
    }
    avisar(exp, "Acampamento: o grupo descansou e recuperou as forças.");
}

void machucarTodos(Expedicao& exp, int dano) {
    for (int i = 0; i < exp.tamanho; i++) {
        Explorador& e = exp.grupo[i];
        if (e.vida > dano) e.vida -= dano;      // evento não mata ninguém
        else if (e.vida > 0) e.vida = 1;
    }
}

void recompensar(Expedicao& exp) {
    int tipo = exp.atual->tipo;
    int ouro = ouroDoSaque(sortear(12, 22) + 6 * exp.dificuldade + exp.bonusDoCombate);
    string texto;
    if (tipo == NO_CHEFE) {
        ouro += ouroDoSaque(70);
        exp.saque.cristais += 6;
        string item = acharItem(exp, 3, 3);
        texto = string(reinoInimigo(exp.regiao).rei) + " caiu! +" + to_string(ouro + 0) + " ouro, +6 cristais";
        if (!item.empty()) texto += ", " + item;
        exp.desfecho = CHEFE_VENCIDO;
    } else if (tipo == NO_ELITE) {
        ouro += ouroDoSaque(20);
        exp.saque.cristais += 2;
        string item = acharItem(exp, 2, 2);
        texto = "Elite derrotado: +" + to_string(ouro) + " ouro, +2 cristais";
        if (!item.empty()) texto += ", " + item;
    } else {
        texto = "Vitória: +" + to_string(ouro) + " ouro";
        if (chance(35)) {
            string item = acharItem(exp, 1);
            if (!item.empty()) texto += ", " + item;
        }
    }
    exp.saque.ouro += ouro;
    exp.bonusDoCombate = 0;
    avisar(exp, texto + ".");
}

// primeiro inimigo vivo a partir do alvo pedido (-1 se acabou)
int alvoVivo(const Expedicao& exp, int alvo) {
    for (int k = 0; k < exp.totalInimigos; k++) {
        int i = (alvo + k) % exp.totalInimigos;
        if (i >= 0 && exp.inimigos[i].vida > 0) return i;
    }
    return -1;
}

void ferirInimigo(Expedicao& exp, int alvo, int ataque, Explorador& quem) {
    InimigoExp& inimigo = exp.inimigos[alvo];
    int dano = calcularDano(ataque, inimigo.defesa);
    if (dano > inimigo.vida) dano = inimigo.vida;
    inimigo.vida -= dano;
    exp.danoNoInimigo[alvo] += dano;
    if (modeloHeroi(quem.heroi).poder == VAMPIRO) {
        quem.vida += dano / 2;
        if (quem.vida > quem.vidaMax) quem.vida = quem.vidaMax;
    }
}

// todo herói vivo bate no alvo; quem acerta em área pega os vizinhos também
void ataqueDoGrupo(Expedicao& exp, int alvo) {
    for (int i = 0; i < exp.tamanho; i++) {
        Explorador& e = exp.grupo[i];
        if (e.vida <= 0) continue;
        int golpes = modeloHeroi(e.heroi).alvos > 0 ? modeloHeroi(e.heroi).alvos : 1;
        for (int g = 0; g < golpes; g++) {
            int vivo = alvoVivo(exp, alvo + g);
            if (vivo < 0) return;
            ferirInimigo(exp, vivo, e.ataque + exp.forcaExtra, e);
        }
    }
}

// a curandeira cura quem está pior, toda rodada
void curasDoGrupo(Expedicao& exp) {
    for (int i = 0; i < exp.tamanho; i++) {
        if (exp.grupo[i].vida <= 0 || modeloHeroi(exp.grupo[i].heroi).poder != CURA) continue;
        int pior = -1;
        for (int j = 0; j < exp.tamanho; j++) {
            const Explorador& e = exp.grupo[j];
            if (e.vida <= 0 || e.vida == e.vidaMax) continue;
            if (pior < 0 || e.vidaMax - e.vida > exp.grupo[pior].vidaMax - exp.grupo[pior].vida) pior = j;
        }
        if (pior < 0) continue;
        Explorador& e = exp.grupo[pior];
        e.vida = e.vida + 5 > e.vidaMax ? e.vidaMax : e.vida + 5;
    }
}

void vezDosInimigos(Expedicao& exp) {
    for (int i = 0; i < exp.totalInimigos; i++) {
        if (exp.inimigos[i].vida <= 0) continue;
        int vivos[MAX_GRUPO];
        int total = 0;
        for (int h = 0; h < exp.tamanho; h++) {
            if (exp.grupo[h].vida > 0) vivos[total++] = h;
        }
        if (total == 0) return;
        int alvo = vivos[sortear(0, total - 1)];
        Explorador& e = exp.grupo[alvo];
        int dano = calcularDano(exp.inimigos[i].ataque, e.defesa);
        if (exp.defendendo) dano = (dano + 1) / 2;
        if (dano > e.vida) dano = e.vida;
        e.vida -= dano;
        exp.danoNoHeroi[alvo] += dano;
    }
}

bool usarPocao(Expedicao& exp) {
    if (exp.pocoes <= 0) return false;
    exp.pocoes--;
    for (int i = 0; i < exp.tamanho; i++) {
        Explorador& e = exp.grupo[i];
        if (e.vida <= 0) continue;
        e.vida = e.vida + 12 > e.vidaMax ? e.vidaMax : e.vida + 12;
    }
    return true;
}

void montarGrupo(Expedicao& exp, const Reino& reino) {
    int ataqueExtra = bonusAtaqueHerois(reino), vidaExtra = bonusVidaHerois(reino);
    for (int h = 0; h < TOTAL_HEROIS; h++) {            // auras valem pro grupo todo
        if (!reino.herois.naEquipe[h]) continue;
        int poder = modeloHeroi(h).poder;
        if (poder == AURA_ATAQUE) ataqueExtra += 1;
        if (poder == AURA_VIDA) vidaExtra += 5;
        if (poder == AURA_REAL) {
            ataqueExtra += 2;
            vidaExtra += 5;
        }
    }
    exp.tamanho = 0;
    for (int h = 0; h < TOTAL_HEROIS && exp.tamanho < MAX_GRUPO; h++) {
        if (!reino.herois.naEquipe[h]) continue;
        Explorador& e = exp.grupo[exp.tamanho++];
        e.heroi = h;
        e.vidaMax = e.vida = vidaDoHeroi(reino.herois, h) + vidaExtra;
        e.ataque = ataqueDoHeroi(reino.herois, h) + ataqueExtra;
        e.defesa = modeloHeroi(h).defesa;
    }
}

// cada item vira uma coisa diferente no reino. Usa a busca na árvore do catálogo.
string guardarItem(Reino& reino, const NoArvore* catalogo, int id) {
    int comparacoes = 0;
    const Item* item = buscarItem(catalogo, id, comparacoes);
    if (item == nullptr) return "";
    if (item->tipo == "poção") {
        reino.pocoes++;
        return item->nome;
    }
    if (item->tipo == "arma" && reino.armas < MAX_ARSENAL) {
        reino.armas++;
        return item->nome + " (+1 ataque)";
    }
    if (item->tipo == "armadura" && reino.armaduras < MAX_ARSENAL) {
        reino.armaduras++;
        return item->nome + " (+3 vida)";
    }
    if (item->tipo == "comida") {
        reino.estoque.comida += item->valor;
        return item->nome;
    }
    int preco = item->valor * economia().vendaPct / 100;    // o resto é vendido (o mercador paga só uma parte)
    reino.estoque.ouro += preco;
    return item->nome + " (vendido por " + to_string(preco) + ")";
}

}

const char* tituloDoEvento(int evento) { return EVENTOS[evento].titulo; }
const char* textoDoEvento(int evento) { return EVENTOS[evento].texto; }
const char* escolhaDoEvento(int evento, int opcao) { return opcao == 0 ? EVENTOS[evento].arriscar : EVENTOS[evento].seguro; }

const char* nomeDoNo(int tipo) {
    switch (tipo) {
        case NO_ENTRADA: return "Entrada";
        case NO_COMBATE: return "Combate";
        case NO_EVENTO: return "Evento";
        case NO_TESOURO: return "Tesouro";
        case NO_ACAMPAMENTO: return "Acampamento";
        case NO_ELITE: return "Elite";
        default: return "Chefe";
    }
}

// mapa

// Liga uma camada do mapa à seguinte. Cada nó daqui ganha pelo menos uma saída
// e cada nó de lá ganha pelo menos uma entrada, então nunca sobra beco sem saída.
static void ligarCamadas(NoMapa* daqui[], int nDaqui, NoMapa* dali[], int nDali) {
    for (int p = 0; p < nDaqui; p++) {
        int reto = p * nDali / nDaqui;
        ligar(daqui[p], dali[reto]);
        if (chance(50) && reto + 1 < nDali) ligar(daqui[p], dali[reto + 1]);
    }
    for (int q = 0; q < nDali; q++) {
        bool temEntrada = false;
        for (int p = 0; p < nDaqui; p++) temEntrada = temEntrada || jaLigado(daqui[p], dali[q]);
        if (!temEntrada) ligar(daqui[q * nDaqui / nDali], dali[q]);
    }
}

void gerarMapa(MapaExpedicao& mapa) {
    liberarMapa(mapa);
    NoMapa* camadas[CAMADAS][MAX_CAMINHOS] = {};
    int tamanho[CAMADAS] = {};

    camadas[0][0] = mapa.entrada = criarNo(mapa, NO_ENTRADA, 0, 0);
    tamanho[0] = 1;
    for (int c = 1; c < CAMADAS - 1; c++) {
        tamanho[c] = sortear(2, 3);
        for (int p = 0; p < tamanho[c]; p++) camadas[c][p] = criarNo(mapa, sortearTipo(c), c, p);
    }
    camadas[CAMADAS - 1][0] = mapa.chefe = criarNo(mapa, NO_CHEFE, CAMADAS - 1, 0);
    tamanho[CAMADAS - 1] = 1;

    for (int c = 0; c < CAMADAS - 1; c++) ligarCamadas(camadas[c], tamanho[c], camadas[c + 1], tamanho[c + 1]);
    // da entrada dá pra ir pra qualquer nó da 1ª camada
    for (int q = 0; q < tamanho[1]; q++) ligar(camadas[0][0], camadas[1][q]);
    mapa.entrada->visitado = true;
}

void liberarMapa(MapaExpedicao& mapa) {
    NoMapa* no = mapa.todos;
    while (no != nullptr) {
        NoMapa* proximo = no->proximo;
        delete no;
        no = proximo;
    }
    mapa = MapaExpedicao();
}

bool existeCaminho(const NoMapa* de, const NoMapa* ate, bool visitados[]) {
    if (de == nullptr || ate == nullptr) return false;
    if (de == ate) return true;
    if (visitados[de->id]) return false;        // já passou por aqui
    visitados[de->id] = true;
    for (int i = 0; i < de->totalCaminhos; i++) {
        if (existeCaminho(de->caminhos[i], ate, visitados)) return true;
    }
    return false;
}

bool mapaValido(const MapaExpedicao& mapa) {
    if (mapa.entrada == nullptr || mapa.chefe == nullptr || mapa.total > MAX_NOS) return false;
    for (const NoMapa* no = mapa.todos; no != nullptr; no = no->proximo) {
        bool visitados[MAX_NOS] = {};
        if (!existeCaminho(no, mapa.chefe, visitados)) return false;
        bool outros[MAX_NOS] = {};
        if (!existeCaminho(mapa.entrada, no, outros)) return false;
    }
    return true;
}

// expedição

int vivosNoGrupo(const Expedicao& exp) {
    int total = 0;
    for (int i = 0; i < exp.tamanho; i++) total += exp.grupo[i].vida > 0;
    return total;
}

int inimigosVivos(const Expedicao& exp) {
    int total = 0;
    for (int i = 0; i < exp.totalInimigos; i++) total += exp.inimigos[i].vida > 0;
    return total;
}

string podeExplorar(const Reino& reino) {
    if (tamanhoDaEquipe(reino.herois) == 0) return "Precisa de pelo menos um herói na equipe.";
    if (reino.feridos > 0) return "Os heróis estão feridos. Voltam em " + to_string(reino.feridos) + " turno(s).";
    if (reino.explorouNoTurno) return "Os heróis já saíram neste turno.";
    if (reino.estoque.comida < CUSTO_EXPEDICAO) return "A expedição precisa de " + to_string(CUSTO_EXPEDICAO) + " de comida.";
    return "";
}

string iniciarExpedicao(Expedicao& exp, Reino& reino, const NoArvore* catalogo, int regiao, int dificuldade) {
    string problema = podeExplorar(reino);
    if (!problema.empty()) return problema;

    liberarMapa(exp.mapa);
    exp = Expedicao();
    exp.ativa = true;
    exp.regiao = regiao;
    exp.dificuldade = dificuldade;
    exp.catalogo = catalogo;
    reino.estoque.comida -= CUSTO_EXPEDICAO;
    reino.explorouNoTurno = true;
    exp.pocoes = reino.pocoes;          // as poções vão na mochila
    reino.pocoes = 0;
    montarGrupo(exp, reino);
    gerarMapa(exp.mapa);
    exp.atual = exp.mapa.entrada;
    exp.mensagem = "Território inimigo: " + string(reinoInimigo(regiao).nome) + ".";
    return "";
}

string avancar(Expedicao& exp, int caminho) {
    if (!exp.ativa || exp.fase != EXP_MAPA) return "Agora não dá pra andar.";
    if (caminho < 0 || caminho >= exp.atual->totalCaminhos) return "Esse caminho não existe.";
    NoMapa* destino = exp.atual->caminhos[caminho];
    if (destino->visitado) return "Vocês já passaram por aí.";

    exp.atual = destino;
    destino->visitado = true;
    switch (destino->tipo) {
        case NO_TESOURO: abrirTesouro(exp); break;
        case NO_ACAMPAMENTO: descansar(exp); break;
        case NO_EVENTO:
            exp.evento = sortear(0, TOTAL_EVENTOS - 1);
            exp.fase = EXP_EVENTO;
            exp.mensagem = EVENTOS[exp.evento].titulo;
            break;
        default: comecarCombate(exp, destino->tipo); break;
    }
    return "";
}

// Caravana: 30 de ouro garantido, mas às vezes é emboscada.
static void abrirCaravana(Expedicao& exp) {
    exp.saque.ouro += 30;
    if (chance(35)) {
        comecarCombate(exp, NO_COMBATE);
        exp.mensagem = "+30 ouro... e era uma emboscada!";
    } else {
        avisar(exp, "Acharam 30 de ouro na carroça.");
    }
}

// Altar: troca 20 de ouro do saque por +2 de ataque até o fim da expedição.
static void rezarNoAltar(Expedicao& exp) {
    if (exp.saque.ouro < 20) {
        avisar(exp, "Não tem 20 de ouro no saque. O altar ficou em silêncio.");
        return;
    }
    exp.saque.ouro -= 20;
    exp.forcaExtra += 2;
    avisar(exp, "O altar brilhou: +2 de ataque até o fim da expedição.");
}

void escolherNoEvento(Expedicao& exp, int opcao) {
    if (exp.fase != EXP_EVENTO) return;
    if (opcao != 0) {
        avisar(exp, "O grupo seguiu viagem.");
        return;
    }
    switch (exp.evento) {
        case CARAVANA_ABANDONADA:
            abrirCaravana(exp);
            break;
        case VIAJANTE_FERIDO:
            machucarTodos(exp, 3);
            exp.saque.cristais += 3;
            avisar(exp, "O viajante agradeceu com 3 cristais. Cada herói perdeu 3 de vida.");
            break;
        case PONTE_DESTRUIDA:
            machucarTodos(exp, 4);
            exp.saque.madeira += 15;
            exp.saque.pedra += 10;
            avisar(exp, "+15 madeira e +10 pedra. A travessia custou 4 de vida de cada um.");
            break;
        case ALTAR_ANTIGO:
            rezarNoAltar(exp);
            break;
        default:
            exp.bonusDoCombate = 25;
            comecarCombate(exp, NO_COMBATE);
            exp.mensagem = "Ataque surpresa! O baú vale 25 de ouro a mais.";
            break;
    }
}

// Fuga: 55% de chance, menos contra o chefe. Fugir custa 1/4 do ouro do saque.
// Retorno: true se o grupo escapou (o combate acaba aqui).
static bool tentarFugir(Expedicao& exp) {
    if (exp.atual->tipo != NO_CHEFE && chance(55)) {
        int perdido = exp.saque.ouro / 4;
        exp.saque.ouro -= perdido;
        avisar(exp, "O grupo fugiu e deixou " + to_string(perdido) + " de ouro pra trás.");
        return true;
    }
    exp.mensagem = exp.atual->tipo == NO_CHEFE ? "Não dá pra fugir do chefe!" : "Não deu pra fugir!";
    return false;
}

// Aplica a ação escolhida pelo jogador. Retorno: true se o combate acabou por fuga.
static bool acaoDoGrupo(Expedicao& exp, int acao, int alvo) {
    switch (acao) {
        case ATACAR:
            ataqueDoGrupo(exp, alvo);
            exp.mensagem = "O grupo atacou.";
            return false;
        case DEFENDER:
            exp.defendendo = true;
            exp.mensagem = "Escudos erguidos: o dano desta rodada cai pela metade.";
            return false;
        case USAR_POCAO:
            usarPocao(exp);
            exp.mensagem = "Poção: +12 de vida pra cada herói.";
            return false;
        default:
            return tentarFugir(exp);
    }
}

void agirNoCombate(Expedicao& exp, int acao, int alvo) {
    if (exp.fase != EXP_COMBATE) return;
    if (acao < ATACAR || acao > FUGIR) {
        exp.mensagem = "Ação inválida.";
        return;
    }
    if (acao == USAR_POCAO && exp.pocoes <= 0) {         // não gasta a rodada
        exp.mensagem = "Não tem poção na mochila.";
        return;
    }
    zerarDanos(exp);
    exp.rodada++;
    exp.defendendo = false;

    if (acaoDoGrupo(exp, acao, alvo)) return;           // 1. a ação do jogador
    curasDoGrupo(exp);
    if (inimigosVivos(exp) == 0) {                      // 2. acabaram os inimigos?
        recompensar(exp);
        return;
    }
    vezDosInimigos(exp);                                // 3. os que sobraram atacam
    if (vivosNoGrupo(exp) == 0) {                       // 4. o grupo caiu?
        exp.desfecho = DERROTADA;
        exp.fase = EXP_FIM;
        exp.mensagem = "O grupo foi derrotado. Todo o saque ficou pra trás.";
    }
}

void continuarExpedicao(Expedicao& exp) {
    if (exp.fase != EXP_AVISO) return;
    if (exp.desfecho == CHEFE_VENCIDO) {
        exp.fase = EXP_FIM;
        exp.mensagem = "Região conquistada! O grupo volta pra casa com o saque.";
        return;
    }
    exp.fase = EXP_MAPA;
}

void voltarAoReino(Expedicao& exp) {
    if (exp.fase != EXP_MAPA) return;
    exp.desfecho = VOLTOU;
    exp.fase = EXP_FIM;
    exp.mensagem = "O grupo voltou em segurança com o saque.";
}

string encerrarExpedicao(Expedicao& exp, Reino& reino) {
    string resumo;
    if (exp.desfecho == DERROTADA) {
        reino.feridos = 2;
        resumo = "Expedição perdida. Heróis feridos por 2 turnos.";
    } else {
        const Saque& s = exp.saque;
        reino.estoque.ouro += s.ouro;
        reino.estoque.madeira += s.madeira;
        reino.estoque.pedra += s.pedra;
        reino.cristais += s.cristais;
        reino.pocoes += exp.pocoes;                 // as que sobraram voltam
        resumo = "Saque: +" + to_string(s.ouro) + " ouro";
        if (s.cristais > 0) resumo += ", +" + to_string(s.cristais) + " cristais";
        for (int i = 0; i < s.totalItens; i++) {
            string item = guardarItem(reino, exp.catalogo, s.itens[i]);
            if (!item.empty() && i < 2) resumo += ", " + item;
        }
        if (s.totalItens > 2) resumo += " e mais " + to_string(s.totalItens - 2);
        resumo += ".";
    }
    liberarMapa(exp.mapa);
    exp.ativa = false;
    exp.atual = nullptr;
    return resumo;
}
