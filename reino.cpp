#include "reino.h"
#include <cstdlib>

using namespace std;

namespace {

// nome, custo, produção por turno, defesa
const ModeloEdificio MODELOS[TOTAL_TIPOS_EDIFICIO] = {
    {"Fazenda",  {30, 20, 0, 0, 0},  {0, 0, 0, 8, 0}, 0},
    {"Serraria", {35, 15, 0, 0, 0},  {0, 6, 0, 0, 0}, 0},
    {"Quartel",  {50, 30, 10, 0, 0}, {0, 0, 0, 0, 0}, 0},
    {"Muralha",  {40, 0, 25, 0, 0},  {0, 0, 0, 0, 0}, 5},
    {"Oficina",  {45, 20, 0, 0, 15}, {0, 0, 0, 0, 0}, 0},
};

const int HABITANTES_POR_COMIDA = 5;
const int SOLDADOS_POR_COMIDA = 3;
const int HABITANTES_POR_OURO = 2;   // imposto

void somar(Recursos& destino, const Recursos& valor, int vezes = 1) {
    destino.ouro += valor.ouro * vezes;
    destino.madeira += valor.madeira * vezes;
    destino.pedra += valor.pedra * vezes;
    destino.comida += valor.comida * vezes;
    destino.ferro += valor.ferro * vezes;
}

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

}

void iniciarReino(Reino& reino) {
    reino.estoque = {100, 60, 30, 80, 10};
    reino.populacao = 20;
    reino.soldados = 6;
    reino.defesa = 0;
    reino.proximoIdEdificio = 1;
    liberarReino(reino);
}

void liberarReino(Reino& reino) {
    liberarEdificios(reino.edificios);
    limparPilha(reino.historico);
    limparFila(reino.ordens);
}

const ModeloEdificio& modeloEdificio(int tipo) {
    return MODELOS[tipo];
}

bool loteLiberado(int linha, int coluna) {
    return abs(linha - 2) <= 1 && abs(coluna - 2) <= 1;
}

bool ehCastelo(int linha, int coluna) {
    return linha == 2 && coluna == 2;
}

bool temRecursos(const Recursos& estoque, const Recursos& custo) {
    return estoque.ouro >= custo.ouro && estoque.madeira >= custo.madeira
        && estoque.pedra >= custo.pedra && estoque.comida >= custo.comida
        && estoque.ferro >= custo.ferro;
}

// motivo pra não construir ("" = pode). A fila não se bloqueia sozinha.
static string motivoBloqueio(const Reino& reino, int tipo, int linha, int coluna, bool olharFila) {
    if (tipo < 0 || tipo >= TOTAL_TIPOS_EDIFICIO) return "Edifício desconhecido.";
    if (!loteLiberado(linha, coluna)) return "Floresta fechada. Expanda o reino primeiro.";
    if (ehCastelo(linha, coluna)) return "Esse lote é do castelo.";
    if (buscarEdificioNoLote(reino.edificios, linha, coluna) != nullptr) {
        return "Esse lote já está ocupado.";
    }
    if (olharFila && loteNaFila(reino.ordens, linha, coluna)) {
        return "Esse lote já tem uma ordem na fila.";
    }
    if (!temRecursos(reino.estoque, MODELOS[tipo].custo)) {
        return "Recursos insuficientes para " + string(MODELOS[tipo].nome) + ".";
    }
    return "";
}

string validarConstrucao(const Reino& reino, int tipo, int linha, int coluna) {
    return motivoBloqueio(reino, tipo, linha, coluna, true);
}

// cobra e coloca na lista (sem validar)
static Edificio erguer(Reino& reino, int tipo, int linha, int coluna) {
    const ModeloEdificio& modelo = MODELOS[tipo];
    somar(reino.estoque, modelo.custo, -1);
    reino.defesa += modelo.defesa;

    Edificio novo;
    novo.id = reino.proximoIdEdificio++;
    novo.tipo = tipo;
    novo.linha = linha;
    novo.coluna = coluna;
    inserirEdificio(reino.edificios, novo);
    return novo;
}

string construirEdificio(Reino& reino, int tipo, int linha, int coluna) {
    string problema = validarConstrucao(reino, tipo, linha, coluna);
    if (!problema.empty()) return problema;

    Acao acao;
    acao.tipo = CONSTRUIU;
    acao.edificio = erguer(reino, tipo, linha, coluna);
    empilhar(reino.historico, acao);
    return "Nova construção: " + string(MODELOS[tipo].nome) + ".";
}

// devolve metade do ouro
string demolirEdificio(Reino& reino, int linha, int coluna) {
    const Edificio* alvo = buscarEdificioNoLote(reino.edificios, linha, coluna);
    if (alvo == nullptr) return "Não tem nada pra demolir aqui.";

    const ModeloEdificio& modelo = MODELOS[alvo->tipo];
    int reembolso = modelo.custo.ouro / 2;
    reino.estoque.ouro += reembolso;
    reino.defesa -= modelo.defesa;

    Acao acao;
    acao.tipo = DEMOLIU;
    acao.edificio = *alvo;          // copia antes de remover
    acao.ouroDevolvido = reembolso;
    empilhar(reino.historico, acao);

    string nome = modelo.nome;
    removerEdificio(reino.edificios, alvo->id);
    return "Demolição: " + nome + ". Voltaram " + to_string(reembolso) + " de ouro.";
}

string desfazerUltimaAcao(Reino& reino) {
    const Acao* topo = topoDaPilha(reino.historico);
    if (topo == nullptr) return "Nada pra desfazer neste turno.";
    if (topo->tipo == DEMOLIU && reino.estoque.ouro < topo->ouroDevolvido) {
        return "Precisa devolver o ouro da demolição pra desfazer.";
    }
    Acao acao;
    desempilhar(reino.historico, acao);
    const ModeloEdificio& modelo = MODELOS[acao.edificio.tipo];
    if (acao.tipo == CONSTRUIU) {
        removerEdificio(reino.edificios, acao.edificio.id);
        somar(reino.estoque, modelo.custo);         // devolve tudo
        reino.defesa -= modelo.defesa;
        return "Desfeito: " + string(modelo.nome) + " (recursos devolvidos).";
    }
    inserirEdificio(reino.edificios, acao.edificio);
    reino.estoque.ouro -= acao.ouroDevolvido;
    reino.defesa += modelo.defesa;
    return "Desfeito: " + string(modelo.nome) + " voltou.";
}

string agendarConstrucao(Reino& reino, int tipo, int linha, int coluna) {
    string problema = motivoBloqueio(reino, tipo, linha, coluna, true);
    bool soFaltaRecurso = problema.rfind("Recursos insuficientes", 0) == 0;
    if (!problema.empty() && !soFaltaRecurso) return problema;

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
        bool faltaRecurso = problema.rfind("Recursos insuficientes", 0) == 0;
        if (faltaRecurso) break;

        OrdemConstrucao ordem;
        desenfileirar(reino.ordens, ordem);
        if (!problema.empty()) continue;            // lote ocupado: cancela
        erguer(reino, ordem.tipoEdificio, ordem.linha, ordem.coluna);
        if (!resumo.empty()) resumo += ", ";
        resumo += MODELOS[ordem.tipoEdificio].nome;
    }
    return resumo.empty() ? "" : "Da fila: " + resumo + ".";
}

// soma a produção de todos os edifícios
Recursos previsaoDoTurno(const Reino& reino) {
    Recursos total;
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        somar(total, MODELOS[no->dados.tipo].producao);
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
