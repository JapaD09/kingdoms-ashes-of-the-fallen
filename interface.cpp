// Telas do jogo (raylib). Desenha em 320x180 e amplia pra janela.

#include "interface.h"
#include "algoritmos.h"
#include "figuras.h"
#include "fonte.h"
#include "natureza.h"
#include "visao3d.h"
#include "paleta.h"
#include "pincel.h"
#include "sprites.h"
#include "ui.h"
#include "tela_batalha.h"
#include "tela_herois.h"
#include "raylib.h"
#include <cmath>
#include <ctime>
#include <string>

using namespace std;

namespace {

const int ESCALA = 4;
const int GRADE = 5;
const int LOTE = 22;
const int GRADE_X = 105;
const int GRADE_Y = 19;
const int MAX_BRASAS = 40;

enum Tela { TELA_TITULO, TELA_REINO, TELA_BATALHA, TELA_DERROTA };

// ações dos botões
enum AcaoBotao { SAIR, RECRUTAR, RECURSOS, EXPEDICAO, CATALOGO, FIM_DO_TURNO, DESFAZER, TROCAR_VISTA, HEROIS };

// fagulhas do fundo
struct Brasa {
    float x, y;
    float velocidade;
};

struct Botao {
    Rectangle area;
    const char* texto;
    int acao;
    Desenho icone;
};

struct EstadoTela {
    Tela atual = TELA_TITULO;
    Brasa brasas[MAX_BRASAS];
    string mensagem;
    float tempoMensagem = 0.0f;
    float tempoTransicao = 0.0f;
    int cartaSelecionada = -1;     // índice na mão
    bool painelRecursos = false;
    const char* dica = nullptr;    // texto perto do mouse
    string textoDica;
    string resumoDoTurno;          // produção, mostrada na batalha
};

// tela do catálogo (árvore, ordenação e buscas)
struct EstadoCatalogo {
    bool aberto = false;
    bool porPreco = false;
    Item porId[20];              // in-ordem da árvore = ordenado por id
    Item naTela[20];             // o que aparece na lista
    int total = 0;
    int compOrdenacao = 0;
    string busca;
    bool buscou = false;
    int idAchado = -1;
    int compArvore = 0, compSequencial = 0, compBinaria = 0;
};

EstadoCatalogo catalogo;

// animações leves da tela do reino
struct Animacoes {
    float nascimento[5][5];       // quando o prédio do lote apareceu ou subiu de nível
    int nivelVisto[5][5];         // 0 = lote vazio
    float cartaSobe[TAMANHO_MAO]; // quanto cada carta está levantada
    float tempoMao;               // tempo desde que a mão chegou
    int turnoVisto;
    int valorVisto[9];            // números da barra de cima
    int diferenca[9];
    float pulso[9];
    float escurecer;              // troca de tela
};

Animacoes anim;

const Rectangle BOTAO_POR_ID = {204, 30, 90, 13};
const Rectangle BOTAO_POR_PRECO = {204, 46, 90, 13};
const Rectangle CAIXA_BUSCA = {204, 89, 90, 13};
const Rectangle BOTAO_BUSCAR = {204, 105, 90, 13};
const Rectangle BOTAO_FECHAR = {250, 157, 44, 13};
const Rectangle BOTAO_NOVO_JOGO = {120, 152, 80, 17};

EstadoTela tela;
RenderTexture2D alvo;
Cenario cenarioReino;
Cenario cenarioTitulo;
const Reino* reinoAtual = nullptr;   // pras galinhas saberem onde tem prédio
int fazendasContadas = -1;
float tempoFumaca = 0;
bool modo3D = true;          // vista meio 3D (V troca)

const Botao BOTOES[] = {
    {{4, 20, 70, 16}, "Recrutar", RECRUTAR, iconeSoldado},
    {{4, 39, 70, 16}, "Heróis", HEROIS, iconeEstrela},
    {{4, 58, 70, 16}, "Expedição", EXPEDICAO, iconeMapa},
    {{4, 77, 70, 16}, "Catálogo", CATALOGO, iconeLivro},
    {{4, 96, 70, 16}, "Recursos", RECURSOS, iconeBau},
    {{4, 115, 70, 16}, "Desfazer", DESFAZER, iconeDesfazer},
    {{4, 134, 70, 16}, "Vista 2D", TROCAR_VISTA, iconeVista},
    {{246, 150, 70, 27}, "Fim do turno", FIM_DO_TURNO, iconeAmpulheta},
    {{4, 160, 44, 17}, "Sair", SAIR, iconePorta},
};
const int TOTAL_BOTOES = sizeof(BOTOES) / sizeof(BOTOES[0]);

// utilidades

void zerarAnimacoes() {
    anim = Animacoes();
    for (int& v : anim.valorVisto) v = -1;
    for (auto& linha : anim.nascimento)
        for (float& n : linha) n = -10;
}

// 0 a 1, desacelerando no fim
float suavizar(float t) {
    if (t <= 0) return 0;
    if (t >= 1) return 1;
    return 1 - (1 - t) * (1 - t) * (1 - t);
}

// prédio novo cresce do chão e passa um pouco do tamanho antes de assentar
float crescimentoDoLote(int linha, int coluna) {
    float t = (tempo() - anim.nascimento[linha][coluna]) / 0.4f;
    if (t >= 1) return 1;
    return suavizar(t) * (1 + 0.3f * sinf(t * PI));
}

void avisar(const string& mensagem) {
    if (mensagem.empty()) return;
    tela.mensagem = mensagem;
    tela.tempoMensagem = 3.5f;
}

void dicaCom(const string& texto) {
    tela.textoDica = texto;
    tela.dica = tela.textoDica.c_str();
}

Rectangle areaLote(int linha, int coluna) {
    return {static_cast<float>(GRADE_X + coluna * LOTE),
            static_cast<float>(GRADE_Y + linha * LOTE),
            static_cast<float>(LOTE), static_cast<float>(LOTE)};
}

Rectangle areaCarta(int i) {
    return {static_cast<float>(75 + i * 34), 138, 32, 42};
}

// cartas

int cartaNaMao(const Reino& reino, int i) {
    return (i >= 0 && i < reino.baralho.naMao) ? reino.baralho.mao[i] : -1;
}

int cartaEscolhida(const Reino& reino) {
    return cartaNaMao(reino, tela.cartaSelecionada);
}

int tipoEscolhido(const Reino& reino) {
    int carta = cartaEscolhida(reino);
    return carta < 0 ? -1 : modeloCarta(carta).tipo;
}

bool ehPredio(int tipoCarta) {
    return tipoCarta == CARTA_PREDIO || tipoCarta == CARTA_TROPA;
}

Desenho artDaCarta(int carta) {
    const ModeloCarta& m = modeloCarta(carta);
    switch (m.tipo) {
        case CARTA_PREDIO:
        case CARTA_TROPA:
            return desenhoDoEdificio(m.alvo);
        case CARTA_ENCANTO: {
            const Desenho artes[] = {artLaminas, artArmaduras, artEstandarte, artBencao};
            return artes[m.alvo];
        }
        case CARTA_EXPANSAO:
            return artExpansao;
        default:
            return artCaravana;
    }
}

Color corDoTipo(int tipoCarta) {
    switch (tipoCarta) {
        case CARTA_PREDIO: return FOLHA_CLARA;
        case CARTA_TROPA: return TELHADO_VERM;
        case CARTA_ENCANTO: return ROXO;
        default: return DOURADO_ESCURO;
    }
}

Color corDaRaridade(int raridade) {
    return raridade >= 3 ? ROXO : raridade == 2 ? CRISTAL_ESC : PEDRA;
}

bool podePagar(const Reino& reino, int carta) {
    return temRecursos(reino.estoque, custoDaCarta(carta));
}

int cartaSobMouse(const Reino& reino) {
    for (int i = 0; i < reino.baralho.naMao; i++) {
        if (mouseSobre(areaCarta(i))) return i;
    }
    return -1;
}

// mouse em cima de botão, painel ou barra (no 3D o mapa passa por baixo deles)
bool mouseNaInterface(const Reino& reino) {
    Vector2 m = mouseVirtual();
    if (m.y < 16 || cartaSobMouse(reino) >= 0) return true;
    if (CheckCollisionPointRec(m, {246, 20, 70, 124})) return true;
    for (int i = 0; i < TOTAL_BOTOES; i++) {
        if (mouseSobre(BOTOES[i].area)) return true;
    }
    return false;
}

// lote embaixo do mouse (no 3D, onde o raio do mouse bate no chão)
bool loteSobMouse(const Reino& reino, int& linha, int& coluna) {
    if (mouseNaInterface(reino)) return false;
    Vector2 m = mouseVirtual();
    if (modo3D && !mouseNoMapa3D(m, m)) return false;
    if (m.x < GRADE_X || m.y < GRADE_Y) return false;
    coluna = static_cast<int>(m.x - GRADE_X) / LOTE;
    linha = static_cast<int>(m.y - GRADE_Y) / LOTE;
    return linha < GRADE && coluna < GRADE;
}

bool expansaoValida(const Reino& reino, int linha, int coluna) {
    if (loteLiberado(reino, linha, coluna)) return false;
    return loteLiberado(reino, linha - 1, coluna) || loteLiberado(reino, linha + 1, coluna)
        || loteLiberado(reino, linha, coluna - 1) || loteLiberado(reino, linha, coluna + 1);
}

// brasas

void reiniciarBrasa(Brasa& b, bool espalhar) {
    b.x = GetRandomValue(0, VL);
    b.y = espalhar ? GetRandomValue(0, VA) : VA + GetRandomValue(0, 20);
    b.velocidade = GetRandomValue(6, 16);
}

void atualizarBrasas(float dt) {
    for (Brasa& b : tela.brasas) {
        b.y -= b.velocidade * dt;
        b.x += sinf(b.y * 0.1f) * 0.05f;
        if (b.y < 0) reiniciarBrasa(b, false);
    }
}

void desenharBrasas() {
    for (const Brasa& b : tela.brasas) {
        float alpha = b.y / VA;
        DrawRectangle(static_cast<int>(b.x), static_cast<int>(b.y), 1, 1, Fade(BRASA, alpha));
    }
}

// cenário

// sombras de nuvem
void desenharSombrasNuvens() {
    Color sombra = Fade(CONTORNO, 0.13f);
    for (int i = 0; i < 3; i++) {
        float andou = fmodf(tempo() * 4.0f + i * 140.0f, VL + 120.0f);
        int x = static_cast<int>(andou) - 60;
        int y = 34 + i * 48;
        DrawRectangle(x + 8, y - 4, 30, 4, sombra);
        DrawRectangle(x, y, 50, 10, sombra);
        DrawRectangle(x + 4, y + 10, 36, 4, sombra);
    }
}

// bordas escuras
void desenharVinheta() {
    for (int i = 0; i < 8; i++) {
        Rectangle borda = {static_cast<float>(i), static_cast<float>(i),
                           static_cast<float>(VL - 2 * i), static_cast<float>(VA - 2 * i)};
        DrawRectangleLinesEx(borda, 1, Fade(CONTORNO, 0.32f - i * 0.04f));
    }
}

void desenharLote(const Reino& reino, int linha, int coluna, bool comArvores = true) {
    Rectangle r = areaLote(linha, coluna);
    int x = r.x, y = r.y;
    bool livre = loteLiberado(reino, linha, coluna);
    Color base = (linha + coluna) % 2 ? GRAMA : GRAMA_ALT;

    DrawRectangle(x, y, LOTE, LOTE, livre ? base : GRAMA_ESCURA);
    int semente = linha * 7 + coluna * 13 + 5;
    for (int i = 0; i < 5; i++) {   // tufos de grama
        int tx = x + 2 + (semente * (i + 3)) % 18;
        int ty = y + 2 + (semente * (i + 5) + i * 7) % 18;
        ret(tx, ty, 0.5f, 1, livre ? GRAMA_CLARA : GRAMA);            // folhinhas de grama
        ret(tx + 1, ty + 0.5f, 0.5f, 0.5f, livre ? GRAMA_CLARA : GRAMA);
        ret(tx + 6 - i, ty + 9 - i * 3, 0.5f, 0.5f, livre ? GRAMA_ESCURA : CONTORNO);
    }
    if (!livre && comArvores) {
        for (int i = 0; i < 3; i++) {
            int dx, dy, tipo, tamanho, tom;
            arvoreDoLote(semente, i, dx, dy, tipo, tamanho, tom);
            float vento = ventoNaArvore(tempo(), semente + i * 2.0f);
            desenharArvoreVariada(x + dx, y + dy, tipo, tamanho, tom, vento);
        }
    }
    if (!livre) DrawRectangle(x, y, LOTE, LOTE, Fade(CONTORNO, 0.15f));
    ret(x, y, LOTE, 0.5f, Fade(CONTORNO, 0.22f));        // divisa fina entre os lotes
    ret(x, y, 0.5f, LOTE, Fade(CONTORNO, 0.22f));
    if (livre) ret(x + 0.5f, y + 0.5f, LOTE - 0.5f, 0.5f, Fade(WHITE, 0.08f));
}

// chão embaixo da grade
void desenharPlanalto() {
    int lado = GRADE * LOTE + 6;
    int x = GRADE_X - 3, y = GRADE_Y - 3;
    DrawRectangle(x + 3, y + lado + 6, lado, 4, Fade(CONTORNO, 0.35f));
    DrawRectangle(x, y, lado, lado, GRAMA_ESCURA);
    DrawRectangle(x, y + lado, lado, 6, TERRA);
    for (int i = 0; i < lado - 4; i += 7) {
        DrawRectangle(x + 2 + i, y + lado + 1 + (i / 7 % 2) * 2, 4, 2, TERRA_ESCURA);
    }
    DrawRectangleLines(x - 1, y - 1, lado + 2, lado + 8, CONTORNO);
}

// contorno pulsando
void contornoPulsando(Rectangle r, Color cor) {
    float pulso = 0.55f + 0.45f * sinf(tempo() * 6.0f);
    DrawRectangle(r.x, r.y, LOTE, LOTE, Fade(TEXTO_CLARO, 0.15f * pulso));
    DrawRectangleLinesEx(r, 1, Fade(cor, pulso));
}

// lotes onde a carta escolhida pode ir piscam
void desenharLotesValidos(const Reino& reino) {
    int tipo = tipoEscolhido(reino);
    if (tipo != CARTA_EXPANSAO && tipo != CARTA_ENCANTO) return;
    for (int l = 0; l < GRADE; l++) {
        for (int c = 0; c < GRADE; c++) {
            bool valido = tipo == CARTA_EXPANSAO ? expansaoValida(reino, l, c)
                                                 : buscarEdificioNoLote(reino.edificios, l, c) != nullptr;
            if (valido) DrawRectangleLinesEx(areaLote(l, c), 1, Fade(tipo == CARTA_EXPANSAO ? DOURADO : ROXO, 0.5f));
        }
    }
}

void dicaDoPredio(const Edificio& e) {
    string texto = modeloEdificio(e.tipo).nome;
    if (e.nivel > 1) texto += " nv " + to_string(e.nivel);
    if (e.bonusAtaque > 0) texto += " +" + to_string(e.bonusAtaque) + "atq";
    if (e.bonusVida > 0) texto += " +" + to_string(e.bonusVida) + "vida";
    if (e.bonusTropas > 0) texto += " +" + to_string(e.bonusTropas) + "tropa";
    dicaCom(texto);
}

void desenharDestaqueLote(const Reino& reino, bool comPrevia = true) {
    int linha, coluna;
    if (!loteSobMouse(reino, linha, coluna)) return;
    Rectangle r = areaLote(linha, coluna);
    const Edificio* ocupante = buscarEdificioNoLote(reino.edificios, linha, coluna);
    if (ocupante != nullptr) dicaDoPredio(*ocupante);
    if (ehCastelo(linha, coluna)) tela.dica = "Castelo";

    int carta = cartaEscolhida(reino);
    int tipo = tipoEscolhido(reino);
    bool livre = loteLiberado(reino, linha, coluna) && !ehCastelo(linha, coluna) && ocupante == nullptr;
    if (ehPredio(tipo) && livre) {
        if (comPrevia) desenharComContorno(desenhoDoEdificio(modeloCarta(carta).alvo), r.x, r.y, 1);
        contornoPulsando(r, DOURADO);
        return;
    }
    if (ehPredio(tipo) && ocupante != nullptr && ocupante->tipo == modeloCarta(carta).alvo) {
        contornoPulsando(r, DOURADO);
        tela.dica = ocupante->nivel < NIVEL_MAXIMO ? "Subir de nível" : "Nível máximo";
        return;
    }
    if ((tipo == CARTA_ENCANTO && ocupante != nullptr) || (tipo == CARTA_EXPANSAO && expansaoValida(reino, linha, coluna))) {
        contornoPulsando(r, tipo == CARTA_ENCANTO ? ROXO : DOURADO);
        return;
    }
    DrawRectangleLinesEx(r, 1, Fade(TEXTO_CLARO, 0.6f));
}

// bolinhas de nível e marquinhas dos encantos
void desenharMarcasDoPredio(const Edificio& e, Rectangle r) {
    for (int i = 1; i < e.nivel; i++) {
        DrawRectangle(r.x + 1 + (i - 1) * 4, r.y + 1, 3, 3, CONTORNO);
        DrawRectangle(r.x + 2 + (i - 1) * 4, r.y + 2, 1, 1, DOURADO);
    }
    int x = r.x + LOTE - 4;
    if (e.bonusAtaque > 0) { DrawRectangle(x, r.y + 1, 3, 3, TELHADO_VERM); x -= 4; }
    if (e.bonusVida > 0) { DrawRectangle(x, r.y + 1, 3, 3, PEDRA_CLARA); x -= 4; }
    if (e.bonusTropas > 0) DrawRectangle(x, r.y + 1, 3, 3, TELHADO_AZUL);
}

// percorre a lista de edifícios
void desenharEdificios(const Reino& reino) {
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        const Edificio& e = no->dados;
        Rectangle r = areaLote(e.linha, e.coluna);
        float cresce = crescimentoDoLote(e.linha, e.coluna);
        ret(r.x + 2, r.y + 19.5f, 18, 1.5f, Fade(CONTORNO, 0.22f));      // sombra no chão
        comecarEscala(r.x + 11, r.y + 20, 2 - cresce, cresce);
        desenharComContorno(desenhoDoEdificio(e.tipo), r.x, r.y, 1);
        terminarEscala();
        if (cresce == 1) desenharMarcasDoPredio(e, r);
    }
}

// ordens da fila aparecem como prédio "fantasma" com a posição
void desenharFilaNoMapa(const Reino& reino) {
    int posicao = 1;
    for (NoFila* no = reino.ordens.inicio; no != nullptr; no = no->proximo, posicao++) {
        Rectangle r = areaLote(no->ordem.linha, no->ordem.coluna);
        desenhoDoEdificio(no->ordem.tipoEdificio)(r.x, r.y, 1);
        DrawRectangleRec(r, Fade(GRAMA, 0.55f));
        DrawRectangleLinesEx(r, 1, Fade(DOURADO, 0.8f));
        DrawRectangle(r.x + 15, r.y + 1, 6, 9, CONTORNO);
        desenharTexto(TextFormat("%d", posicao), r.x + 16, r.y + 2, 1, DOURADO);
    }
}

void desenharGrade(const Reino& reino) {
    desenharPlanalto();
    for (int l = 0; l < GRADE; l++) {
        for (int c = 0; c < GRADE; c++) {
            desenharLote(reino, l, c);
        }
    }
    ret(GRADE_X + 2 * LOTE + 1, GRADE_Y + 2 * LOTE + 20.5f, 20, 1.5f, Fade(CONTORNO, 0.22f));
    desenharComContorno(desenharCastelo, GRADE_X + 2 * LOTE, GRADE_Y + 2 * LOTE, 1);
    desenharEdificios(reino);
    desenharFilaNoMapa(reino);
    desenharGalinhas(tempo());
    desenharFumaca();
    desenharLotesValidos(reino);
    desenharDestaqueLote(reino);
}

// HUD

// quando o número muda ele pisca (verde subiu, vermelho desceu) e a diferença aparece embaixo
int itemTopo(int indice, Desenho icone, int valor, const char* nome, int x, Color cor = TEXTO_CLARO) {
    if (anim.valorVisto[indice] >= 0 && anim.valorVisto[indice] != valor) {
        anim.diferenca[indice] = valor - anim.valorVisto[indice];
        anim.pulso[indice] = 1.0f;
    }
    anim.valorVisto[indice] = valor;
    float pulso = anim.pulso[indice];
    bool subiu = anim.diferenca[indice] > 0;

    const char* numero = TextFormat("%d", valor);
    int largura = 12 + larguraTexto(numero, 1);
    Rectangle r = {static_cast<float>(x), 2, static_cast<float>(largura), 11};

    caixa(r, Fade(CONTORNO, 0.6f), Fade(CONTORNO, 0.35f));
    if (pulso > 0) {
        Color destaque = subiu ? GRAMA_CLARA : CORACAO_CLARO;
        cor = ColorLerp(cor, destaque, pulso);
        const char* texto = TextFormat("%+d", anim.diferenca[indice]);
        int desce = static_cast<int>((1 - pulso) * 6);
        desenharTextoSombra(texto, x + 4, 17 + desce, 1, Fade(destaque, fminf(pulso * 2, 1)));
    }
    icone(x + 2, pulso > 0.7f ? 3 : 4, 1);
    desenharTexto(numero, x + 10, 4, 1, cor);
    if (mouseSobre(r)) tela.dica = nome;
    return x + largura + 2;
}

void desenharBarraTopo(const Jogo& jogo) {
    const Reino& reino = jogo.reino;
    DrawRectangle(0, 0, VL, 15, MADEIRA_ESCURA);
    for (int i = 37; i < VL; i += 46) DrawRectangle(i, 0, 1, 15, Fade(CONTORNO, 0.4f));
    DrawRectangle(0, 0, VL, 1, MADEIRA);
    DrawRectangle(0, 15, VL, 1, CONTORNO);

    iconeCoroa(3, 4, 1);
    desenharTextoSombra(TextFormat("T%d", jogo.turno), 12, 4, 1, DOURADO);

    int x = 30;
    x = itemTopo(0, iconeOuro, reino.estoque.ouro, "Ouro", x);
    x = itemTopo(1, iconeMadeira, reino.estoque.madeira, "Madeira", x);
    x = itemTopo(2, iconePedra, reino.estoque.pedra, "Pedra", x);
    x = itemTopo(3, iconeComida, reino.estoque.comida, "Comida", x);
    x = itemTopo(4, iconeFerro, reino.estoque.ferro, "Ferro", x);
    x = itemTopo(5, iconePovo, reino.populacao, "População", x);
    x = itemTopo(6, iconeSoldado, reino.soldados, "Guardas do castelo", x);
    x = itemTopo(7, iconeCristal, reino.cristais, "Cristais (heróis)", x, CRISTAL);
    bool fraco = reino.vidaCastelo <= VIDA_CASTELO / 3;
    itemTopo(8, iconeCoracao, reino.vidaCastelo, "Vida do castelo", x, fraco ? CORACAO_CLARO : TEXTO_CLARO);
}

void desenharBotao(const Botao& original) {
    Botao b = original;
    if (b.acao == TROCAR_VISTA) b.texto = modo3D ? "Vista 2D" : "Vista 3D";
    bool sobre = mouseSobre(b.area);
    bool principal = b.acao == FIM_DO_TURNO;
    Color fundo = principal ? TELHADO_VERM : MADEIRA_ESCURA;
    if (sobre) fundo = principal ? Color{214, 84, 64, 255} : MADEIRA;
    Color corTexto = sobre ? DOURADO : TEXTO_CLARO;
    int x = b.area.x, y = b.area.y, w = b.area.width, h = b.area.height;

    if (sobre && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {       // afunda quando aperta
        y += 1;
        caixa({b.area.x, b.area.y + 1, b.area.width, b.area.height}, CONTORNO, fundo);
    } else {
        caixaComSombra(b.area, fundo);
    }
    if (!principal) {                                          // veios da madeira
        ret(x + 3, y + 5.5f, w - 9, 0.5f, Fade(CONTORNO, 0.18f));
        ret(x + 8, y + 10.5f, w - 12, 0.5f, Fade(CONTORNO, 0.18f));
        ret(x + w - 5, y + 3, 1, 0.5f, Fade(CONTORNO, 0.25f));
    }
    if (principal) {
        b.icone(x + (w - 7) / 2, y + 4, 1);
        desenharTextoSombra(b.texto, x + (w - larguraTexto(b.texto, 1)) / 2, y + 14, 1, corTexto);
    } else {
        b.icone(x + 4, y + (h - 7) / 2, 1);
        desenharTextoSombra(b.texto, x + 14, y + (h - 7) / 2, 1, corTexto);
    }
}

// seta da carta escolhida
void desenharSeta(int x, int y) {
    int pulo = static_cast<int>(sinf(tempo() * 6.0f) * 1.5f);
    DrawRectangle(x - 2, y + pulo, 5, 1, DOURADO);
    DrawRectangle(x - 1, y + 1 + pulo, 3, 1, DOURADO);
    DrawRectangle(x, y + 2 + pulo, 1, 1, DOURADO);
}

void fundoDaArte(int x, int y, int tipoCarta, int i) {
    switch (tipoCarta) {
        case CARTA_ENCANTO:
            DrawRectangle(x, y, 26, 23, Color{70, 52, 104, 255});
            DrawRectangle(x, y + 17, 26, 6, Color{56, 40, 86, 255});
            break;
        case CARTA_TROPA:
            DrawRectangle(x, y, 26, 23, (i % 2) ? GRAMA : GRAMA_ALT);
            DrawRectangle(x, y + 19, 26, 4, TERRA);
            break;
        case CARTA_PREDIO:
            DrawRectangle(x, y, 26, 23, (i % 2) ? GRAMA : GRAMA_ALT);
            break;
        default:
            DrawRectangle(x, y, 26, 23, Color{196, 214, 230, 255});
            DrawRectangle(x, y + 17, 26, 6, GRAMA_ALT);
            break;
    }
}

void desenharCarta(int i, const Reino& reino) {
    int carta = cartaNaMao(reino, i);
    const ModeloCarta& modelo = modeloCarta(carta);
    Rectangle r = areaCarta(i);
    bool selecionada = i == tela.cartaSelecionada;
    bool podeComprar = podePagar(reino, carta);
    // chega deslizando de baixo (uma depois da outra) e levanta quando o mouse passa
    float chegada = suavizar((anim.tempoMao - i * 0.07f) / 0.3f);
    r.y += roundf((1 - chegada) * 48 - anim.cartaSobe[i]);
    int x = r.x, y = r.y;

    caixaComSombra(r, PERGAMINHO);
    DrawRectangle(x + 2, y + 2, 28, 3, corDoTipo(modelo.tipo));
    DrawRectangle(x + 2, y + 5, 28, 25, PERGAMINHO_ESC);
    fundoDaArte(x + 3, y + 6, modelo.tipo, i);
    desenharComContorno(artDaCarta(carta), x + 5, y + 7, 1);
    ret(x + 3, y + 6, 26, 0.5f, Fade(CONTORNO, 0.35f));        // moldura fina da figura
    ret(x + 3, y + 6, 0.5f, 23, Fade(CONTORNO, 0.35f));
    ret(x + 2.5f, y + 2.5f, 27, 0.5f, Fade(WHITE, 0.35f));
    if (modelo.raridade >= 2) {                                 // brilho passando nas raras
        float brilho = fmodf(tempo() * 0.6f + i * 0.2f, 2.5f) * 30 - 6;
        if (brilho < 24) ret(x + 3 + brilho, y + 6, 1.5f, 23, Fade(WHITE, 0.22f));
    }
    if (!podeComprar) DrawRectangle(x + 3, y + 6, 26, 23, Fade(CONTORNO, 0.45f));

    Recursos custo = custoDaCarta(carta);
    if (custo.ouro > 0) {
        iconeOuro(x + 3, y + 32, 1);
        desenharTexto(TextFormat("%d", custo.ouro), x + 11, y + 32, 1, podeComprar ? CONTORNO : TELHADO_VERM);
    } else {
        iconeOuro(x + 3, y + 32, 1);
        desenharTexto("0", x + 11, y + 32, 1, VERDE_TEXTO);
    }
    DrawRectangle(x + 25, y + 32, 5, 7, CONTORNO);              // pedra da raridade
    DrawRectangle(x + 26, y + 33, 3, 5, corDaRaridade(modelo.raridade));
    if (selecionada) {
        DrawRectangleLinesEx({r.x - 1, r.y - 1, r.width + 2, r.height + 2}, 1, DOURADO);
        desenharSeta(x + 16, y - 6);
    }
}

int linhaCusto(Desenho icone, int custo, int tenho, int y) {
    if (custo <= 0) return y;
    icone(251, y, 1);
    Color cor = tenho >= custo ? CONTORNO : TELHADO_VERM;
    desenharTexto(TextFormat("%d", custo), 261, y, 1, cor);
    return y + 9;
}

void desenharInfoCarta(int carta, const Reino& reino) {
    const Recursos& r = reino.estoque;
    const ModeloCarta& modelo = modeloCarta(carta);
    desenharTexto(modelo.nome, 251, 24, 1, VERMELHO_ESC);
    desenharTexto(nomeTipoCarta(modelo.tipo), 251, 33, 1, corDoTipo(modelo.tipo));
    DrawRectangle(251, 42, 60, 1, PERGAMINHO_ESC);
    Recursos custo = custoDaCarta(carta);
    int y = linhaCusto(iconeOuro, custo.ouro, r.ouro, 45);
    y = linhaCusto(iconeMadeira, custo.madeira, r.madeira, y);
    y = linhaCusto(iconePedra, custo.pedra, r.pedra, y);
    y = linhaCusto(iconeComida, custo.comida, r.comida, y);
    y = linhaCusto(iconeFerro, custo.ferro, r.ferro, y);
    if (y == 45) {
        desenharTexto("grátis", 251, y, 1, VERDE_TEXTO);
        y += 9;
    }
    DrawRectangle(251, y + 1, 60, 1, PERGAMINHO_ESC);
    desenharTexto(modelo.efeito, 251, y + 5, 1, MADEIRA_ESCURA);
    const char* dica = ehPredio(modelo.tipo) ? "Em cima de\noutro igual:\nsobe nível"
                     : modelo.tipo == CARTA_INSTANTE ? "Clique na\ncarta pra usar" : nullptr;
    if (dica != nullptr) desenharTexto(dica, 251, 112, 1, PERGAMINHO_ESC);
}

// verde se ganha, vermelho se perde (duas colunas)
void itemPrevisao(Desenho icone, int valor, int& coluna, int& y) {
    if (valor == 0) return;
    int x = coluna == 0 ? 251 : 282;
    icone(x, y, 1);
    desenharTexto(TextFormat("%+d", valor), x + 9, y, 1, valor > 0 ? FOLHA : TELHADO_VERM);
    coluna = 1 - coluna;
    if (coluna == 0) y += 9;
}

void desenharProximoAtaque(const Jogo& jogo) {
    int reino = reinoDaVez(jogo);
    const ReinoInimigo& inimigo = reinoInimigo(reino);
    desenharTexto("Próx. ataque", 251, 24, 1, MADEIRA_ESCURA);
    desenharBandeira(251, 34, reino, 1);
    desenharTexto(inimigo.placa, 261, 34, 1, VERMELHO_ESC);
    iconeEspadas(251, 54, 1);
    if (turnoDeChefe(jogo.turno)) {
        desenharTexto("CHEFE!", 261, 54, 1, TELHADO_VERM);
    } else {
        desenharTexto(TextFormat("força %d", poderDaOnda(jogo.turno)), 261, 54, 1, CONTORNO);
    }
}

void desenharResumoReino(const Jogo& jogo) {
    const Reino& reino = jogo.reino;
    desenharProximoAtaque(jogo);
    DrawRectangle(251, 64, 60, 1, PERGAMINHO_ESC);
    desenharTexto("Por turno:", 251, 67, 1, MADEIRA_ESCURA);

    Recursos p = previsaoDoTurno(reino);
    int coluna = 0, y = 77;
    itemPrevisao(iconeOuro, p.ouro, coluna, y);
    itemPrevisao(iconeMadeira, p.madeira, coluna, y);
    itemPrevisao(iconePedra, p.pedra, coluna, y);
    itemPrevisao(iconeComida, p.comida, coluna, y);
    itemPrevisao(iconeFerro, p.ferro, coluna, y);
    if (coluna == 1) y += 9;

    DrawRectangle(251, y + 1, 60, 1, PERGAMINHO_ESC);
    desenharTexto(TextFormat("Defesa: %d", defesaDoReino(reino)), 251, y + 5, 1, MADEIRA_ESCURA);
    desenharTexto(TextFormat("Monte: %d", reino.baralho.monte.tamanho), 251, y + 14, 1, MADEIRA_ESCURA);
    if (reino.ordens.tamanho > 0) {
        desenharTexto(TextFormat("Na fila: %d", reino.ordens.tamanho), 251, y + 23, 1, MADEIRA_ESCURA);
    }
    iconeCoracao(251, 127, 1);
    desenharTexto(TextFormat("%d", reino.vidaCastelo), 260, 127, 1, VERMELHO_ESC);
    barraDeVida(251, 137, 60, reino.vidaCastelo, VIDA_CASTELO, TELHADO_VERM);
}

void desenharPainelInfo(const Jogo& jogo) {
    caixaComSombra({246, 20, 70, 124}, PERGAMINHO);
    DrawRectangle(248, 22, 66, 120, Fade(PERGAMINHO_ESC, 0.25f));
    int i = cartaSobMouse(jogo.reino);
    if (i < 0) i = tela.cartaSelecionada;
    int carta = cartaNaMao(jogo.reino, i);
    if (carta >= 0) {
        desenharInfoCarta(carta, jogo.reino);
        return;
    }
    desenharResumoReino(jogo);
}

void linhaRecurso(Desenho icone, const char* nome, int valor, int y) {
    icone(108, y, 1);
    desenharTexto(nome, 118, y, 1, CONTORNO);
    const char* numero = TextFormat("%d", valor);
    desenharTexto(numero, 212 - larguraTexto(numero, 1), y, 1, VERMELHO_ESC);
}

void desenharPainelRecursos(const Reino& reino) {
    caixaComSombra({100, 26, 120, 120}, PERGAMINHO);
    desenharTexto("Tesouro do Reino", 108, 31, 1, VERMELHO_ESC);
    DrawRectangle(108, 41, 104, 1, PERGAMINHO_ESC);
    linhaRecurso(iconeOuro, "Ouro", reino.estoque.ouro, 45);
    linhaRecurso(iconeMadeira, "Madeira", reino.estoque.madeira, 55);
    linhaRecurso(iconePedra, "Pedra", reino.estoque.pedra, 65);
    linhaRecurso(iconeComida, "Comida", reino.estoque.comida, 75);
    linhaRecurso(iconeFerro, "Ferro", reino.estoque.ferro, 85);
    linhaRecurso(iconePovo, "População", reino.populacao, 95);
    linhaRecurso(iconeSoldado, "Guardas", reino.soldados, 105);
    linhaRecurso(iconeCristal, "Cristais", reino.cristais, 115);
    linhaRecurso(iconeCoracao, "Castelo", reino.vidaCastelo, 125);
}

Color corRaridade(int raridade) {
    return raridade == 3 ? DOURADO : raridade == 2 ? TELHADO_AZUL : PEDRA;
}

void desenharListaCatalogo() {
    desenharTexto("ID", 26, 36, 1, PERGAMINHO_ESC);
    desenharTexto("Item", 44, 36, 1, PERGAMINHO_ESC);
    desenharTexto("Preço", 166, 36, 1, PERGAMINHO_ESC);
    for (int i = 0; i < catalogo.total; i++) {
        const Item& item = catalogo.naTela[i];
        int y = 45 + i * 9;
        if (item.id == catalogo.idAchado) DrawRectangle(24, y - 1, 172, 9, Fade(DOURADO, 0.6f));
        desenharTexto(TextFormat("%d", item.id), 26, y, 1, MADEIRA_ESCURA);
        DrawRectangle(40, y + 2, 2, 3, corRaridade(item.raridade));
        desenharTexto(item.nome.c_str(), 44, y, 1, CONTORNO);
        const char* preco = TextFormat("%d", item.valor);
        desenharTexto(preco, 192 - larguraTexto(preco, 1), y, 1, VERMELHO_ESC);
    }
}

void desenharResultadoBusca() {
    if (!catalogo.buscou) {
        desenharTexto("Digite um ID\ne aperte Enter", 205, 122, 1, PERGAMINHO_ESC);
        return;
    }
    const char* achou = catalogo.idAchado >= 0 ? "Achou!" : "Não existe.";
    desenharTexto(achou, 205, 121, 1, catalogo.idAchado >= 0 ? VERDE_TEXTO : TELHADO_VERM);
    desenharTexto(TextFormat("Árvore: %d", catalogo.compArvore), 205, 130, 1, CONTORNO);
    desenharTexto(TextFormat("Sequencial: %d", catalogo.compSequencial), 205, 139, 1, CONTORNO);
    desenharTexto(TextFormat("Binária: %d", catalogo.compBinaria), 205, 148, 1, CONTORNO);
}

void desenharCatalogo(const Jogo& jogo) {
    DrawRectangle(0, 0, VL, VA, Fade(CONTORNO, 0.5f));
    caixaComSombra({20, 22, 280, 152}, PERGAMINHO);
    desenharTexto("Catálogo", 26, 26, 1, VERMELHO_ESC);
    desenharTexto(TextFormat("altura da árvore: %d", alturaArvore(jogo.catalogo)), 80, 26, 1, PERGAMINHO_ESC);
    DrawRectangle(200, 26, 1, 144, PERGAMINHO_ESC);
    desenharListaCatalogo();

    botaozinho(BOTAO_POR_ID, "Por ID", !catalogo.porPreco);
    botaozinho(BOTAO_POR_PRECO, "Por preço", catalogo.porPreco);
    if (catalogo.porPreco) {
        desenharTexto(TextFormat("ordenou com\n%d comparações", catalogo.compOrdenacao), 205, 61, 1, MADEIRA_ESCURA);
    }
    desenharTexto("Buscar ID:", 205, 81, 1, CONTORNO);
    caixa(CAIXA_BUSCA, CONTORNO, TEXTO_CLARO);
    bool cursor = static_cast<int>(tempo() * 2) % 2 == 0;
    desenharTexto((catalogo.busca + (cursor ? "_" : "")).c_str(), 207, 92, 1, CONTORNO);
    botaozinho(BOTAO_BUSCAR, "Buscar", false);
    desenharResultadoBusca();
    botaozinho(BOTAO_FECHAR, "Fechar", false);
}

void desenharMensagem() {
    if (tela.tempoMensagem <= 0) return;
    float alpha = tela.tempoMensagem > 1 ? 1.0f : tela.tempoMensagem;
    const char* texto = tela.mensagem.c_str();
    int largura = larguraTexto(texto, 1) + 12;
    Rectangle r = {static_cast<float>((VL - largura) / 2), 116, static_cast<float>(largura), 14};
    caixa(r, Fade(DOURADO_ESCURO, alpha), Fade(CONTORNO, alpha * 0.9f));
    desenharTexto(texto, r.x + 6, 120, 1, Fade(TEXTO_CLARO, alpha));
}

void desenharDica() {
    if (tela.dica == nullptr) return;
    Vector2 m = mouseVirtual();
    int largura = larguraTexto(tela.dica, 1) + 8;
    int x = static_cast<int>(m.x) + 6;
    if (x + largura > VL) x = VL - largura;
    caixa({static_cast<float>(x), m.y + 6, static_cast<float>(largura), 13}, CONTORNO, PERGAMINHO);
    desenharTexto(tela.dica, x + 4, static_cast<int>(m.y) + 9, 1, CONTORNO);
}

void desenharTransicao(int turno) {
    if (tela.tempoTransicao <= 0) return;
    float alpha = tela.tempoTransicao > 0.4f ? 1.0f : tela.tempoTransicao / 0.4f;
    DrawRectangle(0, 0, VL, VA, Fade(CONTORNO, alpha * 0.55f));
    if (alpha < 1.0f) return;
    desenharFaixa(100, 74, 120, 26);
    const char* texto = TextFormat("Turno %d", turno);
    desenharTextoContorno(texto, (VL - larguraTexto(texto, 2)) / 2, 80, 2,
                          BRILHO_OURO, DOURADO, CONTORNO);
}

// telas

void desenharIlhaTitulo() {
    DrawRectangle(106, 143, 112, 4, Fade(CONTORNO, 0.35f));
    DrawRectangle(104, 131, 112, 8, GRAMA);
    DrawRectangle(100, 133, 120, 4, GRAMA);
    DrawRectangle(104, 139, 112, 4, TERRA);
    DrawRectangle(104, 143, 112, 1, TERRA_ESCURA);
    for (int i = 0; i < 112; i += 9) DrawRectangle(106 + i, 132 + i % 4, 1, 1, GRAMA_CLARA);
}

void desenharTelaTitulo() {
    desenharCenario(cenarioTitulo, tempo());
    desenharSombrasNuvens();
    desenharIlhaTitulo();
    desenharComContorno(desenharCastelo, 127, 74, 3);
    desenharBrasas();

    int x = (VL - larguraTexto("KINGDOMS", 4)) / 2;
    int flutua = static_cast<int>(roundf(sinf(tempo() * 1.5f) * 1.5f));
    desenharTextoContorno("KINGDOMS", x, 10 + flutua, 4, BRILHO_OURO, DOURADO_ESCURO, CONTORNO);
    desenharFaixa(96, 46, 128, 15);
    textoCentralizado("Ashes of the Fallen", 50, 1, TEXTO_CLARO);
    if (static_cast<int>(tempo() * 2) % 2 == 0) {
        textoCentralizado("Clique para começar", 158, 1, TEXTO_CLARO);
    }
    desenharTexto("Eduarda & Mateus", 4, 170, 1, Fade(TEXTO_CLARO, 0.35f));
    desenharVinheta();
}

// fogo tremendo em cima do castelo caído
void desenharChamas() {
    const int pontos[][2] = {{134, 84}, {182, 84}, {158, 78}, {146, 104}, {172, 110}, {160, 122}};
    for (int i = 0; i < 6; i++) {
        int altura = 6 + static_cast<int>((sinf(tempo() * 9 + i * 1.7f) + 1) * 3);
        int x = pontos[i][0], y = pontos[i][1];
        DrawRectangle(x - 3, y - altura, 7, altura, Fade(BRASA, 0.85f));
        DrawRectangle(x - 1, y - altura - 3, 3, 3, Fade(BRASA, 0.6f));
        DrawRectangle(x - 1, y - altura / 2, 3, altura / 2, JANELA);
    }
}

void desenharTelaDerrota(const Jogo& jogo) {
    desenharCenario(cenarioTitulo, tempo());
    DrawRectangle(0, 0, VL, VA, Fade(CONTORNO, 0.55f));
    desenharIlhaTitulo();
    desenharComContorno(desenharCastelo, 127, 74, 3);
    DrawRectangle(127, 62, 66, 76, Fade(CONTORNO, 0.35f));
    desenharChamas();
    desenharBrasas();
    desenharFaixa(90, 22, 140, 26);
    desenharTextoContorno("O castelo caiu", (VL - larguraTexto("O castelo caiu", 2)) / 2, 28, 2,
                          BRILHO_OURO, DOURADO, CONTORNO);
    textoCentralizado(TextFormat("Ashenfall resistiu até o turno %d.", jogo.turno), 56, 1, TEXTO_CLARO);
    botaoPrincipal(BOTAO_NOVO_JOGO, "Novo jogo");
    desenharVinheta();
}

// chão que vira textura na vista 3D: sem árvores e sem prédios (eles ficam em pé)
void desenharChaoDoReino(const Reino& reino) {
    desenharChaoCenario(cenarioReino, tempo());
    int lado = GRADE * LOTE + 6;
    DrawRectangle(GRADE_X - 3, GRADE_Y - 3, lado, lado, GRAMA_ESCURA);
    for (int l = 0; l < GRADE; l++) {
        for (int c = 0; c < GRADE; c++) desenharLote(reino, l, c, false);
    }
    desenharSombrasNuvens();
    desenharLotesValidos(reino);
    desenharDestaqueLote(reino, false);
}

Previa3D previaDaCarta(const Reino& reino) {
    Previa3D previa;
    for (int l = 0; l < GRADE; l++) {
        for (int c = 0; c < GRADE; c++) previa.crescimento[l][c] = crescimentoDoLote(l, c);
    }
    int linha, coluna;
    int carta = cartaEscolhida(reino);
    if (carta < 0 || !ehPredio(modeloCarta(carta).tipo) || !loteSobMouse(reino, linha, coluna)) return previa;
    bool livre = loteLiberado(reino, linha, coluna) && !ehCastelo(linha, coluna)
              && !buscarEdificioNoLote(reino.edificios, linha, coluna)
              && !loteNaFila(reino.ordens, linha, coluna);
    if (!livre) return previa;
    previa.linha = linha;
    previa.coluna = coluna;
    previa.tipo = modeloCarta(carta).alvo;
    return previa;
}

void desenharTelaReino(const Jogo& jogo) {
    if (modo3D) {
        desenharCena3D(cenarioReino, jogo.reino, previaDaCarta(jogo.reino), tempo());
    } else {
        desenharCenario(cenarioReino, tempo());
        desenharGrade(jogo.reino);
        desenharSombrasNuvens();
    }
    desenharBrasas();
    desenharVinheta();

    for (int i = 0; i < TOTAL_BOTOES; i++) desenharBotao(BOTOES[i]);
    desenharPainelInfo(jogo);
    for (int i = 0; i < jogo.reino.baralho.naMao; i++) desenharCarta(i, jogo.reino);
    desenharBarraTopo(jogo);
    if (tela.painelRecursos) desenharPainelRecursos(jogo.reino);
    if (catalogo.aberto) desenharCatalogo(jogo);
    if (heroisAbertos()) desenharHerois(jogo.reino);
    desenharMensagem();
    desenharDica();
    desenharTransicao(jogo.turno);
}

// entrada

// catálogo

void abrirCatalogo(const Jogo& jogo) {
    catalogo = EstadoCatalogo();
    catalogo.aberto = true;
    copiarEmOrdem(jogo.catalogo, catalogo.porId, catalogo.total, 20);
    for (int i = 0; i < catalogo.total; i++) catalogo.naTela[i] = catalogo.porId[i];
}

void mostrarCatalogo(bool porPreco) {
    for (int i = 0; i < catalogo.total; i++) catalogo.naTela[i] = catalogo.porId[i];
    catalogo.porPreco = porPreco;
    if (porPreco) ordenarPorValor(catalogo.naTela, catalogo.total, catalogo.compOrdenacao);
}

// as três buscas procuram o mesmo id, pra dar pra comparar
void buscarNoCatalogo(const Jogo& jogo) {
    if (catalogo.busca.empty()) return;
    int id = stoi(catalogo.busca);
    catalogo.compArvore = 0;
    const Item* item = buscarItem(jogo.catalogo, id, catalogo.compArvore);
    buscaSequencial(catalogo.naTela, catalogo.total, id, catalogo.compSequencial);
    buscaBinaria(catalogo.porId, catalogo.total, id, catalogo.compBinaria);
    catalogo.buscou = true;
    catalogo.idAchado = item ? item->id : -1;
}

void lerTecladoCatalogo(const Jogo& jogo) {
    for (int c = GetCharPressed(); c > 0; c = GetCharPressed()) {
        if (c >= '0' && c <= '9' && catalogo.busca.size() < 3) catalogo.busca += static_cast<char>(c);
    }
    if (IsKeyPressed(KEY_BACKSPACE) && !catalogo.busca.empty()) catalogo.busca.pop_back();
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) buscarNoCatalogo(jogo);
    if (IsKeyPressed(KEY_ESCAPE)) catalogo.aberto = false;
}

void tratarCatalogo(const Jogo& jogo) {
    lerTecladoCatalogo(jogo);
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    if (mouseSobre(BOTAO_POR_ID)) mostrarCatalogo(false);
    if (mouseSobre(BOTAO_POR_PRECO)) mostrarCatalogo(true);
    if (mouseSobre(BOTAO_BUSCAR)) buscarNoCatalogo(jogo);
    if (mouseSobre(BOTAO_FECHAR)) catalogo.aberto = false;
}

void comecarBatalha(Jogo& jogo) {
    tela.cartaSelecionada = -1;
    tela.painelRecursos = false;
    tela.resumoDoTurno = encerrarTurno(jogo);
    if (!jogo.emBatalha) return;
    comecarAnimacaoBatalha(jogo.batalha);
    tela.atual = TELA_BATALHA;
    anim.escurecer = 1;
}

void executarAcao(Jogo& jogo, int acao) {
    switch (acao) {
        case RECURSOS:
            tela.painelRecursos = !tela.painelRecursos;
            break;
        case FIM_DO_TURNO:
            comecarBatalha(jogo);
            break;
        case SAIR:
            jogo.rodando = false;
            break;
        case DESFAZER:
            avisar(desfazerUltimaAcao(jogo.reino));
            break;
        case CATALOGO:
            abrirCatalogo(jogo);
            break;
        case HEROIS:
            abrirHerois();
            break;
        case RECRUTAR:
            avisar(recrutarGuardas(jogo.reino));
            break;
        case TROCAR_VISTA:
            modo3D = !modo3D;
            avisar(modo3D ? "Vista 3D: Q/E giram, roda dá zoom, R volta." : "Vista 2D.");
            break;
        default:
            avisar("A expedição ainda está sendo preparada.");
            break;
    }
}

void cliqueNoLote(Reino& reino, int linha, int coluna) {
    const Edificio* ocupante = buscarEdificioNoLote(reino.edificios, linha, coluna);
    if (tela.cartaSelecionada >= 0) {
        int antes = reino.baralho.naMao;
        avisar(jogarCarta(reino, tela.cartaSelecionada, linha, coluna));
        if (reino.baralho.naMao < antes) tela.cartaSelecionada = -1;   // a carta foi usada
    } else if (ehCastelo(linha, coluna)) {
        avisar("O castelo de Ashenfall.");
    } else if (ocupante != nullptr) {
        avisar(string(modeloEdificio(ocupante->tipo).nome) + ". Botão direito demole.");
    } else {
        avisar("Escolha uma carta lá embaixo.");
    }
}

// botão direito: demole ou cancela
void cliqueDireito(Reino& reino) {
    int linha, coluna;
    bool emLote = loteSobMouse(reino, linha, coluna);
    if (tela.cartaSelecionada < 0 && emLote && buscarEdificioNoLote(reino.edificios, linha, coluna) != nullptr) {
        avisar(demolirEdificio(reino, linha, coluna));
        return;
    }
    tela.cartaSelecionada = -1;
    tela.painelRecursos = false;
}

void escolherCarta(Reino& reino, int i) {
    int carta = cartaNaMao(reino, i);
    if (carta < 0) return;
    if (modeloCarta(carta).tipo == CARTA_INSTANTE) {      // instante não precisa de lote
        avisar(jogarCarta(reino, i, 0, 0));
        tela.cartaSelecionada = -1;
        return;
    }
    tela.cartaSelecionada = (i == tela.cartaSelecionada) ? -1 : i;
}

void tratarClique(Jogo& jogo) {
    int carta = cartaSobMouse(jogo.reino);
    if (carta >= 0) {
        escolherCarta(jogo.reino, carta);
        return;
    }
    int linha, coluna;
    if (loteSobMouse(jogo.reino, linha, coluna)) {
        cliqueNoLote(jogo.reino, linha, coluna);
        return;
    }
    for (int i = 0; i < TOTAL_BOTOES; i++) {
        if (mouseSobre(BOTOES[i].area)) {
            executarAcao(jogo, BOTOES[i].acao);
            return;
        }
    }
}

// teclas 1 a 5
void tratarTeclasCartas(Reino& reino) {
    for (int i = 0; i < TAMANHO_MAO; i++) {
        if (IsKeyPressed(KEY_ONE + i)) escolherCarta(reino, i);
    }
}

// galinha não anda em cima de prédio
bool galinhaPodeAndar(float x, float y) {
    int coluna = static_cast<int>(x - GRADE_X) / LOTE;
    int linha = static_cast<int>(y - GRADE_Y) / LOTE;
    if (ehCastelo(linha, coluna) || !loteLiberado(*reinoAtual, linha, coluna)) return false;
    return buscarEdificioNoLote(reinoAtual->edificios, linha, coluna) == nullptr;
}

// mais fazenda = mais galinha
void atualizarGalinhasDoReino(const Reino& reino, float dt) {
    reinoAtual = &reino;
    int fazendas = contarEdificiosDoTipo(reino.edificios, FAZENDA);
    if (fazendas != fazendasContadas) {
        fazendasContadas = fazendas;
        Rectangle area = {GRADE_X + LOTE + 2.0f, GRADE_Y + LOTE + 4.0f, 3 * LOTE - 4.0f, 3 * LOTE - 6.0f};
        prepararGalinhas(area, 2 + 2 * fazendas, galinhaPodeAndar);
    }
    atualizarGalinhas(dt);
}

// fumaça sai das chaminés
void atualizarChamines(const Reino& reino, float dt) {
    atualizarFumaca(dt);
    tempoFumaca += dt;
    if (tempoFumaca < 0.45f) return;
    tempoFumaca = 0;
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        int tipo = no->dados.tipo;
        Rectangle r = areaLote(no->dados.linha, no->dados.coluna);
        if (tipo == OFICINA) soltarFumaca(r.x + 12, r.y);
        if (tipo == SERRARIA) soltarFumaca(r.x + 11, r.y);
        if (tipo == CASA) soltarFumaca(r.x + 14, r.y);
    }
}

// repara em prédio novo (ou que subiu de nível) pra fazer ele crescer e soltar poeira
void repararNosPredios(const Reino& reino) {
    for (int l = 0; l < GRADE; l++) {
        for (int c = 0; c < GRADE; c++) {
            const Edificio* e = buscarEdificioNoLote(reino.edificios, l, c);
            int nivel = e ? e->nivel : 0;
            if (nivel > anim.nivelVisto[l][c]) {
                anim.nascimento[l][c] = tempo();
                Rectangle r = areaLote(l, c);
                for (int i = 0; i < 4; i++) soltarFumaca(r.x + 3 + i * 5, r.y + 16);
            }
            anim.nivelVisto[l][c] = nivel;
        }
    }
}

void atualizarAnimacoes(const Jogo& jogo, float dt) {
    repararNosPredios(jogo.reino);
    if (jogo.turno != anim.turnoVisto) {       // mão nova: as cartas entram de novo
        anim.turnoVisto = jogo.turno;
        anim.tempoMao = 0;
    }
    anim.tempoMao += dt;
    int sobMouse = cartaSobMouse(jogo.reino);
    for (int i = 0; i < TAMANHO_MAO; i++) {
        float alvo = (i == tela.cartaSelecionada || i == sobMouse) ? 5.0f : 0.0f;
        anim.cartaSobe[i] += (alvo - anim.cartaSobe[i]) * fminf(dt * 20, 1);
    }
    for (float& p : anim.pulso) {
        if (p > 0) p -= dt * 0.8f;
    }
}

void atualizarTelaReino(Jogo& jogo) {
    float dt = GetFrameTime();
    atualizarAnimacoes(jogo, dt);
    atualizarGalinhasDoReino(jogo.reino, dt);
    atualizarChamines(jogo.reino, dt);
    if (catalogo.aberto) {
        tratarCatalogo(jogo);
        return;
    }
    if (heroisAbertos()) {
        avisar(atualizarHerois(jogo.reino));
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        tela.cartaSelecionada = -1;
        tela.painelRecursos = false;
    }
    if (tela.tempoTransicao > 0) return;

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) cliqueDireito(jogo.reino);
    tratarTeclasCartas(jogo.reino);
    if (IsKeyPressed(KEY_SPACE)) executarAcao(jogo, FIM_DO_TURNO);
    if (IsKeyPressed(KEY_Z)) executarAcao(jogo, DESFAZER);
    if (IsKeyPressed(KEY_V)) executarAcao(jogo, TROCAR_VISTA);
    if (IsKeyPressed(KEY_H)) executarAcao(jogo, HEROIS);
    if (modo3D) atualizarCamera3D(GetFrameTime());
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) tratarClique(jogo);
}

void atualizarBatalha(Jogo& jogo) {
    if (!atualizarTelaBatalha(jogo.batalha, GetFrameTime())) return;
    string resultado = concluirBatalha(jogo);
    anim.escurecer = 1;
    if (jogo.derrota) {
        tela.atual = TELA_DERROTA;
        return;
    }
    tela.atual = TELA_REINO;
    tela.tempoTransicao = 1.3f;
    avisar(resultado);
}

// etapa 10: tudo da partida velha é liberado e começa outra
void novoJogo(Jogo& jogo) {
    iniciarJogo(jogo, static_cast<unsigned>(time(nullptr)));
    tela.atual = TELA_REINO;
    tela.cartaSelecionada = -1;
    tela.tempoTransicao = 1.3f;
    fazendasContadas = -1;
    zerarAnimacoes();
    anim.escurecer = 1;
    resetarCamera3D();
}

}

void abrirJanela() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(VL * ESCALA, VA * ESCALA, "Kingdoms: Ashes of the Fallen");
    SetWindowMinSize(VL * 2, VA * 2);
    ToggleBorderlessWindowed();   // já abre em tela cheia (F11 volta pra janela)
    SetExitKey(KEY_NULL);   // ESC não fecha o jogo
    SetTargetFPS(60);
    alvo = LoadRenderTexture(VL * FATOR, VA * FATOR);      // cada unidade = 2 pixels de verdade
    zerarAnimacoes();
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_POINT);
    for (Brasa& b : tela.brasas) reiniciarBrasa(b, true);
    Rectangle planalto = {GRADE_X - 6.0f, GRADE_Y - 8.0f, GRADE * LOTE + 12.0f, GRADE * LOTE + 32.0f};
    gerarCenario(cenarioReino, planalto, VL, VA, 88, 7);
    gerarCenario(cenarioTitulo, {92, 60, 136, 92}, VL, VA, 262, 21);
    InfoMapa info = {VL, VA, GRADE_X, GRADE_Y, LOTE, GRADE, {}};
    for (int i = 0; i < TOTAL_TIPOS_EDIFICIO; i++) info.predios[i] = desenhoDoEdificio(i);
    iniciarVisao3D(info);
}

bool janelaAberta() {
    return !WindowShouldClose();
}

void fecharJanela() {
    liberarVisao3D();
    UnloadRenderTexture(alvo);
    CloseWindow();
}

void atualizarInterface(Jogo& jogo) {
    float dt = GetFrameTime();
    if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();
    atualizarBrasas(dt);
    if (tela.tempoMensagem > 0) tela.tempoMensagem -= dt;
    if (tela.tempoTransicao > 0) tela.tempoTransicao -= dt;
    if (anim.escurecer > 0) anim.escurecer -= dt * 2.5f;

    switch (tela.atual) {
        case TELA_TITULO:
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                tela.atual = TELA_REINO;
                tela.tempoTransicao = 1.3f;
                anim.escurecer = 1;
            }
            break;
        case TELA_BATALHA:
            atualizarBatalha(jogo);
            break;
        case TELA_DERROTA:
            if (clicouEm(BOTAO_NOVO_JOGO) || IsKeyPressed(KEY_ENTER)) novoJogo(jogo);
            break;
        default:
            atualizarTelaReino(jogo);
            break;
    }
}

void desenharInterface(const Jogo& jogo) {
    tela.dica = nullptr;
    if (tela.atual == TELA_REINO && modo3D) {
        BeginTextureMode(texturaDoChao());       // o chão do 3D é desenhado antes
        comecarZoom();
        desenharChaoDoReino(jogo.reino);
        terminarZoom();
        EndTextureMode();
    }
    BeginTextureMode(alvo);
    comecarZoom();
    switch (tela.atual) {
        case TELA_TITULO: desenharTelaTitulo(); break;
        case TELA_BATALHA: desenharTelaBatalha(jogo.batalha, jogo.reino, tela.resumoDoTurno); break;
        case TELA_DERROTA: desenharTelaDerrota(jogo); break;
        default: desenharTelaReino(jogo); break;
    }
    if (anim.escurecer > 0) DrawRectangle(0, 0, VL, VA, Fade(CONTORNO, anim.escurecer));   // troca de tela suave
    terminarZoom();
    EndTextureMode();

    // altura negativa: a textura vem invertida
    BeginDrawing();
    ClearBackground(BLACK);
    Rectangle origem = {0, 0, static_cast<float>(VL * FATOR), -static_cast<float>(VA * FATOR)};
    DrawTexturePro(alvo.texture, origem, areaNaJanela(), {0, 0}, 0.0f, WHITE);
    EndDrawing();
}
