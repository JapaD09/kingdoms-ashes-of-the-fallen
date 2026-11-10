#include "reino.h"
#include <cstdlib>

using namespace std;

namespace {

// Valores iniciais de balanceamento (guia, seção 4). Vamos ajustando jogando.
//                    nome         custo {ouro, madeira, pedra, comida, ferro}   produção          defesa
const ModeloEdificio MODELOS[TOTAL_TIPOS_EDIFICIO] = {
    {"Fazenda",  {30, 20, 0, 0, 0},  {0, 0, 0, 8, 0}, 0},
    {"Serraria", {35, 15, 0, 0, 0},  {0, 6, 0, 0, 0}, 0},
    {"Quartel",  {50, 30, 10, 0, 0}, {0, 0, 0, 0, 0}, 0},
    {"Muralha",  {40, 0, 25, 0, 0},  {0, 0, 0, 0, 0}, 5},
    {"Oficina",  {45, 20, 0, 0, 15}, {0, 0, 0, 0, 0}, 0},
};

const int HABITANTES_POR_COMIDA = 5;   // 1 de comida pra cada 5 habitantes
const int SOLDADOS_POR_COMIDA = 3;     // 1 de comida pra cada 3 soldados
const int HABITANTES_POR_OURO = 2;     // imposto: 1 de ouro pra cada 2 habitantes

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

// monta "ouro +10" e junta no texto, pulando o que não mudou
void anotar(string& texto, const char* nome, int valor) {
    if (valor == 0) return;
    if (!texto.empty()) texto += "  ";
    texto += string(nome) + (valor > 0 ? " +" : " ") + to_string(valor);
}

// Se a comida ficou negativa, zera e parte do povo vai embora
string aplicarFome(Reino& reino) {
    if (reino.estoque.comida >= 0) return "";
    int falta = -reino.estoque.comida;
    int foramEmbora = 1 + falta / 2;
    if (foramEmbora > reino.populacao) foramEmbora = reino.populacao;

    reino.estoque.comida = 0;
    reino.populacao -= foramEmbora;
    return "Faltou comida! " + to_string(foramEmbora) + " pessoas foram embora.";
}

}  // namespace

void iniciarReino(Reino& reino) {
    reino.estoque = {100, 60, 30, 80, 10};
    reino.populacao = 20;
    reino.soldados = 6;
    reino.defesa = 0;
    reino.proximoIdEdificio = 1;
    liberarEdificios(reino.edificios);   // se for um reinício, limpa a lista antiga
}

void liberarReino(Reino& reino) {
    liberarEdificios(reino.edificios);
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

string validarConstrucao(const Reino& reino, int tipo, int linha, int coluna) {
    if (tipo < 0 || tipo >= TOTAL_TIPOS_EDIFICIO) return "Edifício desconhecido.";
    if (!loteLiberado(linha, coluna)) return "Floresta fechada. Expanda o reino primeiro.";
    if (ehCastelo(linha, coluna)) return "Esse lote é do castelo.";
    if (buscarEdificioNoLote(reino.edificios, linha, coluna) != nullptr) {
        return "Esse lote já está ocupado.";
    }
    if (!temRecursos(reino.estoque, MODELOS[tipo].custo)) {
        return "Recursos insuficientes para " + string(MODELOS[tipo].nome) + ".";
    }
    return "";
}

string construirEdificio(Reino& reino, int tipo, int linha, int coluna) {
    string problema = validarConstrucao(reino, tipo, linha, coluna);
    if (!problema.empty()) return problema;

    const ModeloEdificio& modelo = MODELOS[tipo];
    somar(reino.estoque, modelo.custo, -1);    // cobra uma vez só
    reino.defesa += modelo.defesa;

    Edificio novo;
    novo.id = reino.proximoIdEdificio++;
    novo.tipo = tipo;
    novo.linha = linha;
    novo.coluna = coluna;
    inserirEdificio(reino.edificios, novo);
    return "Nova construção: " + string(modelo.nome) + ".";
}

// Demolir devolve metade do ouro gasto
string demolirEdificio(Reino& reino, int linha, int coluna) {
    const Edificio* alvo = buscarEdificioNoLote(reino.edificios, linha, coluna);
    if (alvo == nullptr) return "Não tem nada pra demolir aqui.";

    const ModeloEdificio& modelo = MODELOS[alvo->tipo];
    int reembolso = modelo.custo.ouro / 2;
    reino.estoque.ouro += reembolso;
    reino.defesa -= modelo.defesa;

    string nome = modelo.nome;
    removerEdificio(reino.edificios, alvo->id);   // depois daqui "alvo" não vale mais
    return "Demolição: " + nome + ". Voltaram " + to_string(reembolso) + " de ouro.";
}

// Percorre a lista somando a produção de cada edifício
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
