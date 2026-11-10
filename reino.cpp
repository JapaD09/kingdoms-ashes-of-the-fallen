#include "reino.h"
#include <cstdlib>

using namespace std;

namespace {

// nome, custo, produção por nível, defesa, tropa, tropas por nível, moradores
const ModeloEdificio MODELOS[TOTAL_TIPOS_EDIFICIO] = {
    {"Fazenda",      {30, 20, 0, 0, 0},    {0, 0, 0, 8, 0},  0, SEM_TROPA, 0, 0},
    {"Serraria",     {35, 15, 0, 0, 0},    {0, 6, 0, 0, 0},  0, SEM_TROPA, 0, 0},
    {"Quartel",      {40, 25, 10, 0, 0},   {},               0, SOLDADO, 3, 0},
    {"Muralha",      {40, 0, 25, 0, 0},    {},               2, SEM_TROPA, 0, 0},
    {"Oficina",      {45, 20, 0, 0, 15},   {},               0, SEM_TROPA, 0, 0},
    {"Mina",         {35, 20, 0, 0, 0},    {0, 0, 4, 0, 2},  0, SEM_TROPA, 0, 0},
    {"Mercado",      {60, 25, 10, 0, 0},   {10, 0, 0, 0, 0}, 0, SEM_TROPA, 0, 0},
    {"Casa",         {25, 25, 0, 0, 0},    {},               0, SEM_TROPA, 0, 6},
    {"Torre",        {50, 15, 30, 0, 0},   {},               0, SEM_TROPA, 0, 0},
    {"Arquearia",    {45, 30, 0, 0, 5},    {},               0, ARQUEIRO, 2, 0},
    {"Estábulo",     {60, 30, 10, 10, 10}, {},               0, CAVALEIRO, 1, 0},
    {"Torre Arcana", {80, 10, 30, 0, 20},  {},               0, MAGO, 1, 0},
};

const Recursos GANHO_CARAVANA = {50, 20, 10, 0, 0};

const int HABITANTES_POR_COMIDA = 5;
const int SOLDADOS_POR_COMIDA = 3;
const int HABITANTES_POR_OURO = 2;   // imposto
const int PRECO_GUARDAS = 20;

int consumoDeComida(const Reino& reino) {
    return reino.populacao / HABITANTES_POR_COMIDA + reino.soldados / SOLDADOS_POR_COMIDA;
}

// ex: "ouro +10"
void anotar(string& texto, const char* nome, int valor) {
    if (valor == 0) return;
    if (!texto.empty()) texto += "  ";
    texto += string(nome) + (valor > 0 ? " +" : " ") + to_string(valor);
}

// sem comida, o povo vai embora
string aplicarFome(Reino& reino) {
    if (reino.estoque.comida >= 0) return "";
    int falta = -reino.estoque.comida;
    int foramEmbora = 1 + falta / 2;
    if (foramEmbora > reino.populacao) foramEmbora = reino.populacao;

    reino.estoque.comida = 0;
    reino.populacao -= foramEmbora;
    return "Faltou comida! " + to_string(foramEmbora) + " pessoas foram embora.";
}

void mudarPopulacao(Reino& reino, int quanto) {
    reino.populacao += quanto;
    if (reino.populacao < 0) reino.populacao = 0;
}

string faltaRecurso(const char* nome) {
    return "Recursos insuficientes para " + string(nome) + ".";
}

bool comecaCom(const string& texto, const char* inicio) {
    return texto.rfind(inicio, 0) == 0;
}

}

void somar(Recursos& destino, const Recursos& valor, int vezes) {
    destino.ouro += valor.ouro * vezes;
    destino.madeira += valor.madeira * vezes;
    destino.pedra += valor.pedra * vezes;
    destino.comida += valor.comida * vezes;
    destino.ferro += valor.ferro * vezes;
}

bool temRecursos(const Recursos& estoque, const Recursos& custo) {
    return estoque.ouro >= custo.ouro && estoque.madeira >= custo.madeira
        && estoque.pedra >= custo.pedra && estoque.comida >= custo.comida
        && estoque.ferro >= custo.ferro;
}

void iniciarReino(Reino& reino) {
    liberarReino(reino);
    reino.estoque = {100, 60, 30, 80, 10};
    reino.populacao = 20;
    reino.soldados = 6;
    reino.cristais = 10;
    reino.vidaCastelo = VIDA_CASTELO;
    reino.proximoIdEdificio = 1;
    reino.herois = ColecaoHerois();
    for (int l = 0; l < TAMANHO_GRADE; l++) {
        for (int c = 0; c < TAMANHO_GRADE; c++) {
            reino.liberado[l][c] = abs(l - 2) <= 1 && abs(c - 2) <= 1;   // começa com 3x3
        }
    }
    montarBaralho(reino.baralho);
    comprarMao(reino.baralho);
}

void liberarReino(Reino& reino) {
    liberarEdificios(reino.edificios);
    limparPilha(reino.historico);
    limparFila(reino.ordens);
    liberarBaralho(reino.baralho);
}

const ModeloEdificio& modeloEdificio(int tipo) {
    return MODELOS[tipo];
}

bool loteLiberado(const Reino& reino, int linha, int coluna) {
    if (linha < 0 || coluna < 0 || linha >= TAMANHO_GRADE || coluna >= TAMANHO_GRADE) return false;
    return reino.liberado[linha][coluna];
}

bool ehCastelo(int linha, int coluna) {
    return linha == 2 && coluna == 2;
}

int defesaDoReino(const Reino& reino) {
    int total = 0;
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        total += MODELOS[no->dados.tipo].defesa * no->dados.nivel;
    }
    return total;
}

// motivo pra não construir ("" = pode). A fila não se bloqueia sozinha.
static string motivoBloqueio(const Reino& reino, int tipo, int linha, int coluna, bool olharFila) {
    if (tipo < 0 || tipo >= TOTAL_TIPOS_EDIFICIO) return "Edifício desconhecido.";
    if (!loteLiberado(reino, linha, coluna)) return "Floresta fechada. Use uma carta de Expansão.";
    if (ehCastelo(linha, coluna)) return "Esse lote é do castelo.";
    if (buscarEdificioNoLote(reino.edificios, linha, coluna) != nullptr) {
        return "Esse lote já está ocupado.";
    }
    if (olharFila && loteNaFila(reino.ordens, linha, coluna)) {
        return "Esse lote já tem uma ordem na fila.";
    }
    if (!temRecursos(reino.estoque, MODELOS[tipo].custo)) return faltaRecurso(MODELOS[tipo].nome);
    return "";
}

string validarConstrucao(const Reino& reino, int tipo, int linha, int coluna) {
    return motivoBloqueio(reino, tipo, linha, coluna, true);
}

// cobra e coloca na lista (sem validar)
static Edificio erguer(Reino& reino, int tipo, int linha, int coluna) {
    const ModeloEdificio& modelo = MODELOS[tipo];
    somar(reino.estoque, modelo.custo, -1);
    mudarPopulacao(reino, modelo.moradores);

    Edificio novo;
    novo.id = reino.proximoIdEdificio++;
    novo.tipo = tipo;
    novo.linha = linha;
    novo.coluna = coluna;
    inserirEdificio(reino.edificios, novo);
    return novo;
}

static void guardarNaPilha(Reino& reino, int tipo, const Edificio& edificio, int carta) {
    Acao acao;
    acao.tipo = tipo;
    acao.edificio = edificio;
    acao.carta = carta;
    empilhar(reino.historico, acao);
}

string construirEdificio(Reino& reino, int tipo, int linha, int coluna) {
    string problema = validarConstrucao(reino, tipo, linha, coluna);
    if (!problema.empty()) return problema;
    guardarNaPilha(reino, CONSTRUIU, erguer(reino, tipo, linha, coluna), -1);
    return "Nova construção: " + string(MODELOS[tipo].nome) + ".";
}

// devolve metade do ouro
string demolirEdificio(Reino& reino, int linha, int coluna) {
    const Edificio* alvo = buscarEdificioNoLote(reino.edificios, linha, coluna);
    if (alvo == nullptr) return "Não tem nada pra demolir aqui.";

    const ModeloEdificio& modelo = MODELOS[alvo->tipo];
    int reembolso = modelo.custo.ouro / 2;
    reino.estoque.ouro += reembolso;
    mudarPopulacao(reino, -modelo.moradores * alvo->nivel);

    Acao acao;
    acao.tipo = DEMOLIU;
    acao.edificio = *alvo;          // copia antes de remover
    acao.ouroDevolvido = reembolso;
    empilhar(reino.historico, acao);

    string nome = modelo.nome;
    removerEdificio(reino.edificios, alvo->id);
    return "Demolição: " + nome + ". Voltaram " + to_string(reembolso) + " de ouro.";
}

// cartas

// mesma carta em cima do mesmo prédio = sobe de nível
static string subirNivel(Reino& reino, int indice, int carta, const Edificio& atual) {
    const ModeloEdificio& modelo = MODELOS[atual.tipo];
    if (atual.nivel >= NIVEL_MAXIMO) return string(modelo.nome) + " já está no nível máximo.";
    if (!temRecursos(reino.estoque, modelo.custo)) return faltaRecurso(modelo.nome);

    guardarNaPilha(reino, MELHOROU, atual, carta);
    Edificio* edificio = acharEdificio(reino.edificios, atual.id);
    edificio->nivel++;
    somar(reino.estoque, modelo.custo, -1);
    mudarPopulacao(reino, modelo.moradores);
    usarCarta(reino.baralho, indice);
    return string(modelo.nome) + " subiu pro nível " + to_string(edificio->nivel) + "!";
}

static string jogarPredio(Reino& reino, int indice, int carta, int linha, int coluna) {
    int tipo = modeloCarta(carta).alvo;
    const Edificio* ocupante = buscarEdificioNoLote(reino.edificios, linha, coluna);
    if (ocupante != nullptr && ocupante->tipo == tipo) return subirNivel(reino, indice, carta, *ocupante);

    string problema = validarConstrucao(reino, tipo, linha, coluna);
    if (comecaCom(problema, "Recursos insuficientes")) {
        usarCarta(reino.baralho, indice);              // a carta vira a ordem da fila
        return agendarConstrucao(reino, tipo, linha, coluna);
    }
    if (!problema.empty()) return problema;

    guardarNaPilha(reino, CONSTRUIU, erguer(reino, tipo, linha, coluna), carta);
    usarCarta(reino.baralho, indice);
    return "Nova construção: " + string(MODELOS[tipo].nome) + ".";
}

static string jogarEncanto(Reino& reino, int indice, int carta, int linha, int coluna) {
    const ModeloCarta& modelo = modeloCarta(carta);
    const Edificio* alvo = buscarEdificioNoLote(reino.edificios, linha, coluna);
    if (alvo == nullptr) return "Use o encanto em cima de um prédio.";
    bool deTropa = MODELOS[alvo->tipo].tropa != SEM_TROPA;
    if (modelo.alvo != BENCAO && !deTropa) return string(modelo.nome) + " só vale pra prédio de tropa.";
    if (modelo.alvo == BENCAO && alvo->nivel >= NIVEL_MAXIMO) return "Esse prédio já está no nível máximo.";
    if (!temRecursos(reino.estoque, modelo.custo)) return faltaRecurso(modelo.nome);

    guardarNaPilha(reino, MELHOROU, *alvo, carta);
    Edificio* edificio = acharEdificio(reino.edificios, alvo->id);
    switch (modelo.alvo) {
        case LAMINAS: edificio->bonusAtaque += 2; break;
        case ARMADURAS: edificio->bonusVida += 5; break;
        case ESTANDARTE: edificio->bonusTropas += 1; break;
        default:
            edificio->nivel++;
            mudarPopulacao(reino, MODELOS[edificio->tipo].moradores);
            break;
    }
    somar(reino.estoque, modelo.custo, -1);
    usarCarta(reino.baralho, indice);
    return string(modelo.nome) + " em " + MODELOS[edificio->tipo].nome + ".";
}

static bool vizinhoAberto(const Reino& reino, int linha, int coluna) {
    return loteLiberado(reino, linha - 1, coluna) || loteLiberado(reino, linha + 1, coluna)
        || loteLiberado(reino, linha, coluna - 1) || loteLiberado(reino, linha, coluna + 1);
}

static string jogarExpansao(Reino& reino, int indice, int carta, int linha, int coluna) {
    const ModeloCarta& modelo = modeloCarta(carta);
    if (linha < 0 || coluna < 0 || linha >= TAMANHO_GRADE || coluna >= TAMANHO_GRADE) return "Fora do mapa.";
    if (loteLiberado(reino, linha, coluna)) return "Esse lote já está aberto.";
    if (!vizinhoAberto(reino, linha, coluna)) return "Só dá pra abrir lote colado no reino.";
    if (!temRecursos(reino.estoque, modelo.custo)) return faltaRecurso(modelo.nome);

    Edificio lugar;
    lugar.linha = linha;
    lugar.coluna = coluna;
    guardarNaPilha(reino, EXPANDIU, lugar, carta);
    reino.liberado[linha][coluna] = true;
    somar(reino.estoque, modelo.custo, -1);
    usarCarta(reino.baralho, indice);
    return "Os lenhadores abriram um lote novo.";
}

static string jogarInstante(Reino& reino, int indice, int carta) {
    guardarNaPilha(reino, USOU_CARTA, Edificio(), carta);
    somar(reino.estoque, GANHO_CARAVANA);
    usarCarta(reino.baralho, indice);
    return "Chegou uma caravana: +50 ouro, +20 madeira, +10 pedra.";
}

string jogarCarta(Reino& reino, int indiceMao, int linha, int coluna) {
    if (indiceMao < 0 || indiceMao >= reino.baralho.naMao) return "Escolha uma carta lá embaixo.";
    int carta = reino.baralho.mao[indiceMao];
    switch (modeloCarta(carta).tipo) {
        case CARTA_PREDIO:
        case CARTA_TROPA:
            return jogarPredio(reino, indiceMao, carta, linha, coluna);
        case CARTA_ENCANTO:
            return jogarEncanto(reino, indiceMao, carta, linha, coluna);
        case CARTA_EXPANSAO:
            return jogarExpansao(reino, indiceMao, carta, linha, coluna);
        default:
            return jogarInstante(reino, indiceMao, carta);
    }
}

// motivo pra não dar pra desfazer o topo ("" = dá)
static string impedeDesfazer(const Reino& reino, const Acao& acao) {
    if (acao.tipo == DEMOLIU && reino.estoque.ouro < acao.ouroDevolvido) {
        return "Precisa devolver o ouro da demolição pra desfazer.";
    }
    if (acao.tipo == USOU_CARTA && !temRecursos(reino.estoque, GANHO_CARAVANA)) {
        return "Você já gastou o que a caravana trouxe.";
    }
    if (acao.tipo == EXPANDIU && loteNaFila(reino.ordens, acao.edificio.linha, acao.edificio.coluna)) {
        return "Tem uma ordem da fila nesse lote.";
    }
    if (acao.carta >= 0 && reino.baralho.naMao >= TAMANHO_MAO) return "A mão está cheia.";
    return "";
}

string desfazerUltimaAcao(Reino& reino) {
    const Acao* topo = topoDaPilha(reino.historico);
    if (topo == nullptr) return "Nada pra desfazer neste turno.";
    string problema = impedeDesfazer(reino, *topo);
    if (!problema.empty()) return problema;

    Acao acao;
    desempilhar(reino.historico, acao);
    const ModeloEdificio& modelo = MODELOS[acao.edificio.tipo];
    string texto;
    switch (acao.tipo) {
        case CONSTRUIU:
            removerEdificio(reino.edificios, acao.edificio.id);
            somar(reino.estoque, modelo.custo);         // devolve tudo
            mudarPopulacao(reino, -modelo.moradores);
            texto = "Desfeito: " + string(modelo.nome) + ".";
            break;
        case DEMOLIU:
            inserirEdificio(reino.edificios, acao.edificio);
            reino.estoque.ouro -= acao.ouroDevolvido;
            mudarPopulacao(reino, modelo.moradores * acao.edificio.nivel);
            texto = "Desfeito: " + string(modelo.nome) + " voltou.";
            break;
        case MELHOROU: {
            Edificio* edificio = acharEdificio(reino.edificios, acao.edificio.id);
            mudarPopulacao(reino, -modelo.moradores * (edificio->nivel - acao.edificio.nivel));
            *edificio = acao.edificio;                  // volta como era
            somar(reino.estoque, custoDaCarta(acao.carta));
            texto = "Desfeito: " + string(modeloCarta(acao.carta).nome) + ".";
            break;
        }
        case EXPANDIU:
            reino.liberado[acao.edificio.linha][acao.edificio.coluna] = false;
            somar(reino.estoque, custoDaCarta(acao.carta));
            texto = "Desfeito: a floresta voltou.";
            break;
        default:
            somar(reino.estoque, GANHO_CARAVANA, -1);
            texto = "Desfeito: a caravana foi embora.";
            break;
    }
    if (acao.carta >= 0) devolverCarta(reino.baralho, acao.carta);
    return texto;
}

string agendarConstrucao(Reino& reino, int tipo, int linha, int coluna) {
    string problema = motivoBloqueio(reino, tipo, linha, coluna, true);
    if (!problema.empty() && !comecaCom(problema, "Recursos insuficientes")) return problema;

    OrdemConstrucao ordem;
    ordem.tipoEdificio = tipo;
    ordem.linha = linha;
    ordem.coluna = coluna;
    enfileirar(reino.ordens, ordem);
    return string(MODELOS[tipo].nome) + " na fila (posição " + to_string(reino.ordens.tamanho) + ").";
}

// Regra: a primeira ordem espera até ter recurso; as de trás não passam na frente.
// Se o lote da ordem ficou ocupado, ela é cancelada.
string processarFila(Reino& reino) {
    string resumo;
    while (!filaVazia(reino.ordens)) {
        const OrdemConstrucao* frente = frenteDaFila(reino.ordens);
        string problema = motivoBloqueio(reino, frente->tipoEdificio, frente->linha, frente->coluna, false);
        if (comecaCom(problema, "Recursos insuficientes")) break;

        OrdemConstrucao ordem;
        desenfileirar(reino.ordens, ordem);
        if (!problema.empty()) continue;            // lote ocupado: cancela
        erguer(reino, ordem.tipoEdificio, ordem.linha, ordem.coluna);
        if (!resumo.empty()) resumo += ", ";
        resumo += MODELOS[ordem.tipoEdificio].nome;
    }
    return resumo.empty() ? "" : "Da fila: " + resumo + ".";
}

string recrutarGuardas(Reino& reino) {
    if (contarEdificiosDoTipo(reino.edificios, QUARTEL) == 0) return "Precisa de um Quartel pra recrutar.";
    if (reino.estoque.ouro < PRECO_GUARDAS) return "Recrutar 2 guardas custa 20 de ouro.";
    if (reino.soldados + 2 > reino.populacao / 2) return "Não tem gente sobrando pra virar guarda.";
    reino.estoque.ouro -= PRECO_GUARDAS;
    reino.soldados += 2;
    return "2 guardas novos no castelo.";
}

string invocar(Reino& reino, int& heroi) {
    heroi = -1;
    if (reino.cristais < CUSTO_INVOCACAO) {
        return "Precisa de " + to_string(CUSTO_INVOCACAO) + " cristais. Vença batalhas pra ganhar.";
    }
    reino.cristais -= CUSTO_INVOCACAO;
    heroi = invocarHeroi(reino.herois);
    const ModeloHeroi& h = modeloHeroi(heroi);
    int estrelas = reino.herois.estrelas[heroi];
    string texto = string(h.nome) + ", " + h.titulo + " (" + nomeRaridade(h.raridade) + ")";
    return estrelas > 1 ? texto + " subiu pra " + to_string(estrelas) + " estrelas!" : texto + " chegou!";
}

// soma a produção de todos os edifícios
Recursos previsaoDoTurno(const Reino& reino) {
    Recursos total;
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        somar(total, MODELOS[no->dados.tipo].producao, no->dados.nivel);
    }
    total.ouro += reino.populacao / HABITANTES_POR_OURO;
    total.comida -= consumoDeComida(reino);
    return total;
}

string processarTurnoReino(Reino& reino) {
    Recursos mudanca = previsaoDoTurno(reino);
    somar(reino.estoque, mudanca);
    limparPilha(reino.historico);     // virou turno: não dá mais pra desfazer

    string fome = aplicarFome(reino);
    if (!fome.empty()) return fome;

    string resumo;
    anotar(resumo, "ouro", mudanca.ouro);
    anotar(resumo, "madeira", mudanca.madeira);
    anotar(resumo, "pedra", mudanca.pedra);
    anotar(resumo, "comida", mudanca.comida);
    anotar(resumo, "ferro", mudanca.ferro);
    return resumo.empty() ? "Um turno tranquilo." : resumo;
}
