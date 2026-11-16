#include "tela_expedicao.h"
#include "som.h"
#include "figuras.h"
#include "fonte.h"
#include "inimigos.h"
#include "paleta.h"
#include "pincel.h"
#include "sprites.h"
#include "ui.h"
#include <cmath>

using namespace std;

namespace {

struct EstadoTela {
    int alvo = 0;               // inimigo escolhido pra atacar
    int rodadaVista = 0;
    float tempoDano = 9;        // tempo desde a última rodada (números de dano)
};

EstadoTela estado;

const Color FUNDO_EXPEDICAO = {46, 38, 36, 255};
const Color PAPEL_MAPA = {214, 190, 142, 255};
const Color TRILHA = {120, 96, 70, 255};

const Rectangle AREA_MAPA = {90, 22, 226, 124};
const Rectangle BOTAO_VOLTAR = {226, 158, 90, 17};
const Rectangle BOTAO_CONTINUAR = {125, 122, 70, 16};
const Rectangle BOTAO_OPCAO_A = {74, 112, 84, 16};
const Rectangle BOTAO_OPCAO_B = {162, 112, 84, 16};
const char* NOMES_ACOES[4] = {"Atacar", "Defender", "Poção", "Fugir"};

Rectangle botaoDaAcao(int acao) {
    return {92.0f + acao * 57, 158, 54, 17};
}

int nosNaCamada(const MapaExpedicao& mapa, int camada) {
    int total = 0;
    for (const NoMapa* no = mapa.todos; no != nullptr; no = no->proximo) total += no->camada == camada;
    return total;
}

Vector2 lugarDoNo(const MapaExpedicao& mapa, const NoMapa* no) {
    int quantos = nosNaCamada(mapa, no->camada);
    float x = AREA_MAPA.x + 20 + no->camada * 37.0f;
    float y = AREA_MAPA.y + AREA_MAPA.height / 2 + (no->posicao - (quantos - 1) / 2.0f) * 36;
    return {x, y};
}

Rectangle areaDoNo(const MapaExpedicao& mapa, const NoMapa* no) {
    Vector2 p = lugarDoNo(mapa, no);
    return {p.x - 9, p.y - 9, 18, 18};
}

Color corDoNo(int tipo) {
    switch (tipo) {
        case NO_COMBATE: return {172, 78, 62, 255};
        case NO_EVENTO: return {96, 110, 170, 255};
        case NO_TESOURO: return DOURADO_ESCURO;
        case NO_ACAMPAMENTO: return FOLHA_CLARA;
        case NO_ELITE: return ROXO;
        case NO_CHEFE: return {60, 40, 44, 255};
        default: return MADEIRA;
    }
}

void iconeDoNo(int tipo, int x, int y) {
    switch (tipo) {
        case NO_COMBATE: iconeEspadas(x, y, 1); break;
        case NO_EVENTO: desenharTextoSombra("?", x + 1, y, 1, TEXTO_CLARO); break;
        case NO_TESOURO: iconeBau(x, y, 1); break;
        case NO_ACAMPAMENTO:
            ret(x + 1, y + 5, 5, 1.5f, MADEIRA_ESCURA);
            ret(x + 2, y + 1.5f, 3, 4, BRASA);
            ret(x + 3, y + 3, 1, 2.5f, JANELA);
            break;
        case NO_ELITE: iconeEstrela(x, y, 1); break;
        case NO_CHEFE: iconeCoroa(x, y, 1); break;
        default: iconePorta(x, y, 1); break;
    }
}

bool ehProximo(const Expedicao& exp, const NoMapa* no) {
    for (int i = 0; i < exp.atual->totalCaminhos; i++) {
        if (exp.atual->caminhos[i] == no) return true;
    }
    return false;
}

// trilha pontilhada entre dois nós
void desenharTrilha(Vector2 a, Vector2 b, Color cor, float grossura) {
    float dx = b.x - a.x, dy = b.y - a.y;
    int passos = static_cast<int>(sqrtf(dx * dx + dy * dy) / 3);
    for (int i = 3; i < passos - 2; i++) {
        float t = static_cast<float>(i) / passos;
        ret(roundf((a.x + dx * t) * 2) / 2, roundf((a.y + dy * t) * 2) / 2, grossura, grossura, cor);
    }
}

// Um nó do mapa: cor pelo tipo, pulso dourado se dá pra ir até ele, bandeira se o grupo está nele.
static void desenharNo(const Expedicao& exp, const NoMapa* no) {
    Rectangle r = areaDoNo(exp.mapa, no);
    bool proximo = exp.fase == EXP_MAPA && ehProximo(exp, no);
    bool sobre = proximo && mouseSobre(r);
    Color cor = corDoNo(no->tipo);
    if (no->visitado && no != exp.atual) cor = ColorLerp(cor, TRILHA, 0.65f);
    if (sobre) r = {r.x - 1, r.y - 1, r.width + 2, r.height + 2};
    caixaComSombra(r, cor);
    iconeDoNo(no->tipo, r.x + r.width / 2 - 3.5f, r.y + r.height / 2 - 3.5f);
    if (proximo) {
        float pulso = 0.5f + 0.5f * sinf(tempo() * 6);
        DrawRectangleLinesEx({r.x - 2, r.y - 2, r.width + 4, r.height + 4}, 1, Fade(DOURADO, pulso));
    }
    if (no == exp.atual) {                              // bandeirinha do grupo
        int pula = static_cast<int>(fabsf(sinf(tempo() * 4)) * 2);
        ret(r.x + 7, r.y - 9 - pula, 1, 8, CONTORNO);
        ret(r.x + 8, r.y - 9 - pula, 5, 3.5f, TELHADO_AZUL);
    }
    if (sobre) textoCentralizadoEm(nomeDoNo(no->tipo), r.x + r.width / 2, r.y + r.height + 2, CONTORNO);
}

void desenharMapa(const Expedicao& exp) {
    caixaComSombra(AREA_MAPA, PAPEL_MAPA);
    for (int i = 0; i < 40; i++) {                      // manchinhas do papel
        ret(AREA_MAPA.x + 4 + (i * 53) % 216, AREA_MAPA.y + 4 + (i * 37) % 114, 1.5f, 0.5f, Fade(TRILHA, 0.25f));
    }
    for (const NoMapa* no = exp.mapa.todos; no != nullptr; no = no->proximo) {
        for (int i = 0; i < no->totalCaminhos; i++) {
            bool daqui = no == exp.atual && exp.fase == EXP_MAPA;
            desenharTrilha(lugarDoNo(exp.mapa, no), lugarDoNo(exp.mapa, no->caminhos[i]),
                           daqui ? VERMELHO_ESC : Fade(TRILHA, 0.7f), daqui ? 1.0f : 0.5f);
        }
    }
    for (const NoMapa* no = exp.mapa.todos; no != nullptr; no = no->proximo) desenharNo(exp, no);
}

void desenharGrupo(const Expedicao& exp) {
    caixaComSombra({4, 22, 82, 152}, PERGAMINHO);
    desenharTexto("Grupo", 9, 26, 1, VERMELHO_ESC);
    for (int i = 0; i < exp.tamanho; i++) {
        const Explorador& e = exp.grupo[i];
        int y = 40 + i * 27;
        Aparencia a = aparenciaHeroi(e.heroi);
        if (e.vida > 0) desenharFigura(18, y + 21, a, false, 0, 0);
        else desenharFiguraCaida(18, y + 21, a, false);
        desenharTexto(modeloHeroi(e.heroi).nome, 32, y + 3, 1, e.vida > 0 ? CONTORNO : PERGAMINHO_ESC);
        barraDeVida(32, y + 14, 48, e.vida, e.vidaMax, e.vida * 3 < e.vidaMax ? TELHADO_VERM : FOLHA_CLARA);
        desenharTexto(TextFormat("%d/%d", e.vida, e.vidaMax), 32, y + 18, 1, MADEIRA_ESCURA);
    }
    DrawRectangle(9, 122, 72, 1, PERGAMINHO_ESC);
    desenharTexto("Saque", 9, 125, 1, VERMELHO_ESC);
    iconeOuro(9, 136, 1);
    desenharTexto(TextFormat("%d", exp.saque.ouro), 18, 136, 1, CONTORNO);
    iconeCristal(46, 136, 1);
    desenharTexto(TextFormat("%d", exp.saque.cristais), 55, 136, 1, CONTORNO);
    iconeBau(9, 147, 1);
    desenharTexto(TextFormat("%d itens", exp.saque.totalItens), 18, 147, 1, CONTORNO);
    desenharTexto(TextFormat("Poções: %d", exp.pocoes), 9, 160, 1, MADEIRA_ESCURA);
}

void desenharTopo(const Jogo& jogo) {
    const Expedicao& exp = jogo.expedicao;
    DrawRectangle(0, 0, VL, 19, MADEIRA_ESCURA);
    DrawRectangle(0, 19, VL, 1, CONTORNO);
    desenharBandeira(5, 5, exp.regiao, 1);
    desenharTextoSombra(TextFormat("Expedição: %s", reinoInimigo(exp.regiao).nome), 16, 6, 1, DOURADO);
    const char* chefes = TextFormat("Chefes %d/%d", chefesDerrotados(jogo), TOTAL_REINOS);
    desenharTextoSombra(chefes, VL - 6 - larguraTexto(chefes, 1), 6, 1, TEXTO_CLARO);
}

void caixaDeTexto(const char* titulo, const string& texto, int altura) {
    DrawRectangle(0, 20, VL, VA - 20, Fade(CONTORNO, 0.45f));
    caixaComSombra({60, 50, 200, static_cast<float>(altura)}, PERGAMINHO);
    if (titulo != nullptr) textoCentralizadoEm(titulo, VL / 2, 56, VERMELHO_ESC);
    string quebrado = quebrarTexto(texto, 184);
    int y = titulo != nullptr ? 70 : 60;
    size_t inicio = 0;
    while (inicio <= quebrado.size()) {                    // centraliza linha por linha
        size_t fim = quebrado.find('\n', inicio);
        if (fim == string::npos) fim = quebrado.size();
        textoCentralizadoEm(quebrado.substr(inicio, fim - inicio).c_str(), VL / 2, y, CONTORNO);
        y += 10;
        inicio = fim + 1;
    }
}

// combate

Vector2 lugarDoHeroi(int i) {
    return {124.0f + (i % 2) * 20, 82.0f + i * 22};
}

Vector2 lugarDoInimigo(int i, bool chefe) {
    if (chefe) return {268, 112};
    return {222.0f + (i % 2) * 22, 78.0f + i * 22};
}

Aparencia aparenciaDoInimigo(const Expedicao& exp, const InimigoExp& inimigo) {
    Aparencia a = aparenciaInimigo(exp.regiao, inimigo.nivel);
    if (a.especial != DRAGAO) a.tamanho = a.tamanho == 1 ? 2 : 3;
    return a;
}

Rectangle areaDoInimigo(const Expedicao& exp, int i) {
    Vector2 p = lugarDoInimigo(i, exp.inimigos[i].chefe);
    int t = aparenciaDoInimigo(exp, exp.inimigos[i]).tamanho;
    return {p.x - 6.0f * t, p.y - 15.0f * t, 12.0f * t, 16.0f * t};
}

void numeroDeDano(Vector2 p, int dano) {
    if (dano <= 0 || estado.tempoDano > 0.9f) return;
    float t = estado.tempoDano / 0.9f;
    const char* texto = TextFormat("-%d", dano);
    int x = static_cast<int>(p.x) - larguraTexto(texto, 1) / 2, y = static_cast<int>(p.y - 34 - t * 10);
    desenharTexto(texto, x + 1, y + 1, 1, Fade(CONTORNO, 1 - t));
    desenharTexto(texto, x, y, 1, Fade(JANELA, 1 - t));
}

// Céu, campo e a fileira de árvores do fundo da luta.
static void desenharCampoDeLuta() {
    DrawRectangleGradientV(90, 22, 226, 44, {110, 150, 196, 255}, {196, 214, 222, 255});
    DrawRectangle(90, 66, 226, 80, GRAMA);
    DrawRectangle(90, 66, 226, 2, GRAMA_ESCURA);
    for (int i = 0; i < 40; i++) ret(92 + (i * 53) % 220, 70 + (i * 31) % 74, 0.5f, 1, i % 3 ? GRAMA_CLARA : GRAMA_ESCURA);
    for (int i = 0; i < 20; i++) {                          // árvores lá no fundo
        int alto = 6 + (i * 3) % 5;
        DrawRectangle(92 + i * 11, 66 - alto, 8, alto, i % 3 ? FOLHA : FOLHA_CLARA);
    }
    DrawRectangleLinesEx(AREA_MAPA, 1, CONTORNO);
}

// Um herói do grupo: caído, ou de pé com barra de vida, escudo e dano levado.
static void desenharHeroiNaLuta(const Expedicao& exp, int i, bool brilha) {
    const Explorador& e = exp.grupo[i];
    Vector2 p = lugarDoHeroi(i);
    Aparencia a = aparenciaHeroi(e.heroi);
    a.tamanho = 2;
    if (e.vida <= 0) {
        desenharFiguraCaida(p.x, p.y, a, false);
        return;
    }
    float avanca = (estado.tempoDano < 0.25f && exp.danoNoInimigo[estado.alvo % MAX_INIMIGOS_EXP] > 0) ? 4 : 0;
    ret(p.x - 6, p.y - 1, 14, 2, Fade(CONTORNO, 0.25f));
    desenharFigura(p.x + avanca, p.y, a, false, 0, brilha && exp.danoNoHeroi[i] > 0 ? 0.8f : 0);
    barraDeVida(static_cast<int>(p.x) - 6, static_cast<int>(p.y) - 30, 12, e.vida, e.vidaMax, DOURADO);
    if (exp.defendendo) ret(p.x + 8, p.y - 16, 2, 10, Fade(CRISTAL, 0.7f));     // escudo erguido
    numeroDeDano(p, exp.danoNoHeroi[i]);
}

// Setinha dourada pulando em cima do inimigo escolhido como alvo.
static void desenharSetaDoAlvo(float x, float topo) {
    int pula = static_cast<int>(fabsf(sinf(tempo() * 5)) * 2);
    ret(x - 2, topo - 12 - pula, 5, 1, DOURADO);
    ret(x - 1, topo - 11 - pula, 3, 1, DOURADO);
    ret(x, topo - 10 - pula, 1, 1, DOURADO);
}

// Um inimigo: caído, ou de pé com vida, seta de alvo e contorno quando o mouse passa.
static void desenharInimigoNaLuta(const Expedicao& exp, int i, bool brilha) {
    const InimigoExp& inimigo = exp.inimigos[i];
    Vector2 p = lugarDoInimigo(i, inimigo.chefe);
    Aparencia a = aparenciaDoInimigo(exp, inimigo);
    if (inimigo.vida <= 0) {
        desenharFiguraCaida(p.x, p.y, a, true);
        numeroDeDano(p, exp.danoNoInimigo[i]);
        return;
    }
    Rectangle r = areaDoInimigo(exp, i);
    ret(p.x - 3.0f * a.tamanho, p.y - 1, 7.0f * a.tamanho, 2, Fade(CONTORNO, 0.25f));
    desenharFigura(p.x, p.y, a, true, 0, brilha && exp.danoNoInimigo[i] > 0 ? 0.8f : 0);
    int largura = inimigo.chefe ? 24 : 12;
    barraDeVida(static_cast<int>(p.x) - largura / 2, static_cast<int>(r.y) - 4, largura, inimigo.vida, inimigo.vidaMax, TELHADO_VERM);
    desenharTexto(TextFormat("%d", inimigo.vida), static_cast<int>(p.x) + largura / 2 + 2, static_cast<int>(r.y) - 7, 1, CONTORNO);
    if (i == estado.alvo) desenharSetaDoAlvo(p.x, r.y);
    if (mouseSobre(r)) DrawRectangleLinesEx(r, 1, Fade(TEXTO_CLARO, 0.6f));
    numeroDeDano(p, exp.danoNoInimigo[i]);
}

void desenharCombate(const Expedicao& exp) {
    desenharCampoDeLuta();
    bool brilha = estado.tempoDano < 0.15f;
    for (int i = 0; i < exp.tamanho; i++) desenharHeroiNaLuta(exp, i, brilha);
    for (int i = 0; i < exp.totalInimigos; i++) desenharInimigoNaLuta(exp, i, brilha);
    for (int acao = 0; acao < 4; acao++) {
        const char* texto = acao == USAR_POCAO ? TextFormat("Poção (%d)", exp.pocoes) : NOMES_ACOES[acao];
        if (acao == ATACAR) botaoPrincipal(botaoDaAcao(acao), texto);
        else botaozinho(botaoDaAcao(acao), texto, false);
    }
}

void desenharMensagem(const Expedicao& exp) {
    caixa({90, 147, 226, 10}, Fade(CONTORNO, 0.8f), Fade(CONTORNO, 0.6f));
    desenharTexto(exp.mensagem.c_str(), 94, 148, 1, TEXTO_CLARO);
}

int primeiroVivo(const Expedicao& exp) {
    for (int i = 0; i < exp.totalInimigos; i++) {
        if (exp.inimigos[i].vida > 0) return i;
    }
    return 0;
}

void agir(Expedicao& exp, int acao) {
    agirNoCombate(exp, acao, estado.alvo);
    tocar(acao == ATACAR ? SOM_GOLPE : SOM_CLIQUE);
    if (exp.rodada != estado.rodadaVista) {
        estado.rodadaVista = exp.rodada;
        estado.tempoDano = 0;
    }
    if (exp.fase == EXP_COMBATE && exp.inimigos[estado.alvo].vida <= 0) estado.alvo = primeiroVivo(exp);
}

void cliquesDoCombate(Expedicao& exp) {
    for (int i = 0; i < exp.totalInimigos; i++) {
        if (exp.inimigos[i].vida > 0 && clicouEm(areaDoInimigo(exp, i))) estado.alvo = i;
    }
    if (estado.tempoDano < 0.35f) return;                   // espera o golpe aparecer
    for (int acao = 0; acao < 4; acao++) {
        if (clicouEm(botaoDaAcao(acao)) || IsKeyPressed(KEY_ONE + acao)) {
            agir(exp, acao);
            return;
        }
    }
}

void cliquesDoMapa(Expedicao& exp) {
    if (clicouEm(BOTAO_VOLTAR)) {
        voltarAoReino(exp);
        return;
    }
    for (int i = 0; i < exp.atual->totalCaminhos; i++) {
        if (clicouEm(areaDoNo(exp.mapa, exp.atual->caminhos[i])) || IsKeyPressed(KEY_ONE + i)) {
            avancar(exp, i);
            if (exp.fase == EXP_COMBATE) {
                estado.alvo = 0;
                estado.rodadaVista = 0;
                estado.tempoDano = 9;
            }
            return;
        }
    }
}

}

void comecarTelaExpedicao() {
    estado = EstadoTela();
}

bool atualizarTelaExpedicao(Jogo& jogo) {
    Expedicao& exp = jogo.expedicao;
    estado.tempoDano += GetFrameTime();
    bool confirmou = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    switch (exp.fase) {
        case EXP_MAPA:
            cliquesDoMapa(exp);
            break;
        case EXP_COMBATE:
            cliquesDoCombate(exp);
            break;
        case EXP_EVENTO:
            if (clicouEm(BOTAO_OPCAO_A) || IsKeyPressed(KEY_ONE)) escolherNoEvento(exp, 0);
            else if (clicouEm(BOTAO_OPCAO_B) || IsKeyPressed(KEY_TWO)) escolherNoEvento(exp, 1);
            if (exp.fase == EXP_COMBATE) {
                estado.alvo = 0;
                estado.rodadaVista = 0;
                estado.tempoDano = 9;
            }
            break;
        case EXP_AVISO:
            if (clicouEm(BOTAO_CONTINUAR) || confirmou) continuarExpedicao(exp);
            break;
        default:
            return clicouEm(BOTAO_CONTINUAR) || confirmou;
    }
    return false;
}

// Caixa final: como a expedição acabou e o que o grupo trouxe.
static void desenharFimDaExpedicao(const Expedicao& exp) {
    const char* titulo = exp.desfecho == DERROTADA ? "Expedição perdida" : exp.desfecho == CHEFE_VENCIDO ? "Região conquistada!" : "De volta pra casa";
    string texto = exp.mensagem;
    if (exp.desfecho != DERROTADA) {
        texto += "\n" + string(TextFormat("%d ouro, %d cristais, %d itens", exp.saque.ouro, exp.saque.cristais, exp.saque.totalItens));
    }
    caixaDeTexto(titulo, texto, 94);
    botaoPrincipal(BOTAO_CONTINUAR, "Pro reino");
}

void desenharTelaExpedicao(const Jogo& jogo) {
    const Expedicao& exp = jogo.expedicao;
    DrawRectangle(0, 0, VL, VA, FUNDO_EXPEDICAO);
    desenharTopo(jogo);
    desenharGrupo(exp);

    if (exp.fase == EXP_COMBATE) {
        desenharCombate(exp);
        desenharMensagem(exp);
        return;
    }
    desenharMapa(exp);
    if (exp.fase == EXP_MAPA) {
        desenharMensagem(exp);
        desenharTexto("Clique num nó ligado\npra seguir viagem.", 92, 160, 1, PERGAMINHO_ESC);
        botaozinho(BOTAO_VOLTAR, "Voltar pra casa", false);
    }
    if (exp.fase == EXP_EVENTO) {
        caixaDeTexto(tituloDoEvento(exp.evento), textoDoEvento(exp.evento), 84);
        botaoPrincipal(BOTAO_OPCAO_A, escolhaDoEvento(exp.evento, 0));
        botaozinho(BOTAO_OPCAO_B, escolhaDoEvento(exp.evento, 1), false);
    }
    if (exp.fase == EXP_AVISO) {
        caixaDeTexto(nullptr, exp.mensagem, 94);
        botaoPrincipal(BOTAO_CONTINUAR, "Continuar");
    }
    if (exp.fase == EXP_FIM) desenharFimDaExpedicao(exp);
}
