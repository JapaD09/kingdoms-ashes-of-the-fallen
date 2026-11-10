#include "fonte.h"

namespace {

const int ALTURA_GLIFO = 9;    // 7 de altura + 2 pra "perna" do g, p, q, y
const int ALTURA_LINHA = 11;

// Cada letra é um desenho de até 5 colunas por 9 linhas. '#' = pixel aceso.
// Linhas que faltam no fim ficam vazias.
struct Glifo {
    char letra;
    const char* linhas[ALTURA_GLIFO];
};

const Glifo GLIFOS[] = {
    {'A', {".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
    {'B', {"####.", "#...#", "#...#", "####.", "#...#", "#...#", "####."}},
    {'C', {".###.", "#...#", "#....", "#....", "#....", "#...#", ".###."}},
    {'D', {"####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####."}},
    {'E', {"#####", "#....", "#....", "####.", "#....", "#....", "#####"}},
    {'F', {"#####", "#....", "#....", "####.", "#....", "#....", "#...."}},
    {'G', {".###.", "#...#", "#....", "#.###", "#...#", "#...#", ".####"}},
    {'H', {"#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
    {'I', {"###", ".#.", ".#.", ".#.", ".#.", ".#.", "###"}},
    {'J', {"..###", "...#.", "...#.", "...#.", "...#.", "#..#.", ".##.."}},
    {'K', {"#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#"}},
    {'L', {"#....", "#....", "#....", "#....", "#....", "#....", "#####"}},
    {'M', {"#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#"}},
    {'N', {"#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#", "#...#"}},
    {'O', {".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
    {'P', {"####.", "#...#", "#...#", "####.", "#....", "#....", "#...."}},
    {'Q', {".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#"}},
    {'R', {"####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"}},
    {'S', {".####", "#....", "#....", ".###.", "....#", "....#", "####."}},
    {'T', {"#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."}},
    {'U', {"#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
    {'V', {"#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."}},
    {'W', {"#...#", "#...#", "#...#", "#.#.#", "#.#.#", "##.##", "#...#"}},
    {'X', {"#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#"}},
    {'Y', {"#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.."}},
    {'Z', {"#####", "....#", "...#.", "..#..", ".#...", "#....", "#####"}},

    {'a', {"", "", ".###.", "....#", ".####", "#...#", ".####"}},
    {'b', {"#....", "#....", "####.", "#...#", "#...#", "#...#", "####."}},
    {'c', {"", "", ".###", "#...", "#...", "#...", ".###"}},
    {'d', {"....#", "....#", ".####", "#...#", "#...#", "#...#", ".####"}},
    {'e', {"", "", ".###.", "#...#", "#####", "#....", ".###."}},
    {'f', {"..##", ".#..", "###.", ".#..", ".#..", ".#..", ".#.."}},
    {'g', {"", "", ".####", "#...#", "#...#", "#...#", ".####", "....#", ".###."}},
    {'h', {"#....", "#....", "####.", "#...#", "#...#", "#...#", "#...#"}},
    {'i', {"#", "", "#", "#", "#", "#", "#"}},
    {'j', {"..#", "", "..#", "..#", "..#", "..#", "..#", "#.#", ".#."}},
    {'k', {"#...", "#...", "#..#", "#.#.", "##..", "#.#.", "#..#"}},
    {'l', {"#.", "#.", "#.", "#.", "#.", "#.", ".#"}},
    {'m', {"", "", "##.#.", "#.#.#", "#.#.#", "#.#.#", "#...#"}},
    {'n', {"", "", "####.", "#...#", "#...#", "#...#", "#...#"}},
    {'o', {"", "", ".###.", "#...#", "#...#", "#...#", ".###."}},
    {'p', {"", "", "####.", "#...#", "#...#", "#...#", "####.", "#....", "#...."}},
    {'q', {"", "", ".####", "#...#", "#...#", "#...#", ".####", "....#", "....#"}},
    {'r', {"", "", "#.##", "##..", "#...", "#...", "#..."}},
    {'s', {"", "", ".###", "#...", ".##.", "...#", "###."}},
    {'t', {".#..", ".#..", "###.", ".#..", ".#..", ".#..", "..##"}},
    {'u', {"", "", "#...#", "#...#", "#...#", "#...#", ".####"}},
    {'v', {"", "", "#...#", "#...#", "#...#", ".#.#.", "..#.."}},
    {'w', {"", "", "#...#", "#...#", "#.#.#", "#.#.#", ".#.#."}},
    {'x', {"", "", "#...#", ".#.#.", "..#..", ".#.#.", "#...#"}},
    {'y', {"", "", "#...#", "#...#", "#...#", "#...#", ".####", "....#", ".###."}},
    {'z', {"", "", "#####", "...#.", "..#..", ".#...", "#####"}},

    {'0', {".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###."}},
    {'1', {".#.", "##.", ".#.", ".#.", ".#.", ".#.", "###"}},
    {'2', {".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"}},
    {'3', {"####.", "....#", "....#", ".###.", "....#", "....#", "####."}},
    {'4', {"...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#."}},
    {'5', {"#####", "#....", "####.", "....#", "....#", "#...#", ".###."}},
    {'6', {".###.", "#....", "#....", "####.", "#...#", "#...#", ".###."}},
    {'7', {"#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..."}},
    {'8', {".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###."}},
    {'9', {".###.", "#...#", "#...#", ".####", "....#", "....#", ".###."}},

    {'.', {"", "", "", "", "", "", "#"}},
    {',', {"", "", "", "", "", "", "#", "#"}},
    {':', {"", "", "", "#", "", "", "#"}},
    {'!', {"#", "#", "#", "#", "#", "", "#"}},
    {'?', {".###.", "#...#", "....#", "...#.", "..#..", "", "..#.."}},
    {'-', {"", "", "", "", "###"}},
    {'+', {"", "", "..#..", "..#..", "#####", "..#..", "..#.."}},
    {'/', {"....#", "...#.", "...#.", "..#..", ".#...", ".#...", "#...."}},
    {'(', {".#", "#.", "#.", "#.", "#.", "#.", ".#"}},
    {')', {"#.", ".#", ".#", ".#", ".#", ".#", "#."}},
    {'\'', {"#", "#"}},
    {'&', {".##..", "#..#.", "#.#..", ".#...", "#.#.#", "#..#.", ".##.#"}},
    {'>', {"#...", ".#..", "..#.", "...#", "..#.", ".#..", "#..."}},
};
const int TOTAL_GLIFOS = sizeof(GLIFOS) / sizeof(GLIFOS[0]);

enum Acento { NENHUM, AGUDO, CRASE, CIRCUNFLEXO, TIL, CEDILHA };

// letra acentuada = letra normal + acento desenhado por cima (ou cedilha embaixo)
struct LetraAcentuada {
    int codigo;      // código Unicode (Latin-1)
    char base;
    Acento acento;
};

const LetraAcentuada ACENTUADAS[] = {
    {0xE1, 'a', AGUDO}, {0xE0, 'a', CRASE}, {0xE2, 'a', CIRCUNFLEXO}, {0xE3, 'a', TIL},
    {0xE9, 'e', AGUDO}, {0xEA, 'e', CIRCUNFLEXO}, {0xED, 'i', AGUDO},
    {0xF3, 'o', AGUDO}, {0xF4, 'o', CIRCUNFLEXO}, {0xF5, 'o', TIL},
    {0xFA, 'u', AGUDO}, {0xE7, 'c', CEDILHA},
    {0xC1, 'A', AGUDO}, {0xC0, 'A', CRASE}, {0xC2, 'A', CIRCUNFLEXO}, {0xC3, 'A', TIL},
    {0xC9, 'E', AGUDO}, {0xCA, 'E', CIRCUNFLEXO}, {0xCD, 'I', AGUDO},
    {0xD3, 'O', AGUDO}, {0xD4, 'O', CIRCUNFLEXO}, {0xD5, 'O', TIL},
    {0xDA, 'U', AGUDO}, {0xC7, 'C', CEDILHA},
};
const int TOTAL_ACENTUADAS = sizeof(ACENTUADAS) / sizeof(ACENTUADAS[0]);

struct Letra {
    const Glifo* glifo;   // nullptr = espaço
    char base;
    Acento acento;
};

const Glifo* acharGlifo(char letra) {
    for (int i = 0; i < TOTAL_GLIFOS; i++) {
        if (GLIFOS[i].letra == letra) return &GLIFOS[i];
    }
    return nullptr;
}

// Lê um caractere UTF-8 e avança o ponteiro.
// Só precisamos de ASCII e das letras acentuadas (2 bytes); o resto vira '?'.
int lerCodigo(const char*& p) {
    unsigned char c = static_cast<unsigned char>(*p++);
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0 && *p) {
        int codigo = ((c & 0x1F) << 6) | (static_cast<unsigned char>(*p) & 0x3F);
        p++;
        return codigo;
    }
    while ((static_cast<unsigned char>(*p) & 0xC0) == 0x80) p++;
    return '?';
}

Letra traduzir(int codigo) {
    if (codigo == ' ') return {nullptr, ' ', NENHUM};
    if (codigo < 0x80) {
        const Glifo* g = acharGlifo(static_cast<char>(codigo));
        return {g ? g : acharGlifo('?'), static_cast<char>(codigo), NENHUM};
    }
    for (int i = 0; i < TOTAL_ACENTUADAS; i++) {
        if (ACENTUADAS[i].codigo == codigo) {
            char base = ACENTUADAS[i].base;
            return {acharGlifo(base), base, ACENTUADAS[i].acento};
        }
    }
    return {acharGlifo('?'), '?', NENHUM};
}

int larguraGlifo(const Glifo* g) {
    if (g == nullptr) return 3;
    int largura = 1;
    for (int l = 0; l < ALTURA_GLIFO; l++) {
        const char* linha = g->linhas[l];
        for (int c = 0; linha && linha[c]; c++) {
            if (linha[c] == '#' && c + 1 > largura) largura = c + 1;
        }
    }
    return largura;
}

void desenharGlifo(const Letra& letra, int x, int y, int e, Color cima, Color baixo) {
    bool semPingo = letra.base == 'i' && letra.acento != NENHUM;   // "í" perde o pingo
    for (int l = 0; l < ALTURA_GLIFO; l++) {
        const char* linha = letra.glifo->linhas[l];
        if (linha == nullptr || (semPingo && l == 0)) continue;
        for (int c = 0; linha[c]; c++) {
            if (linha[c] == '#') {
                DrawRectangle(x + c * e, y + l * e, e, e, l < 4 ? cima : baixo);
            }
        }
    }
}

void desenharAcento(const Letra& letra, int x, int y, int e, int largura, Color cor) {
    bool maiuscula = letra.base >= 'A' && letra.base <= 'Z';
    int cx = x + (largura / 2) * e;
    int topo = y + (maiuscula ? -3 : 0) * e;
    auto ponto = [&](int dx, int dy) { DrawRectangle(cx + dx * e, topo + dy * e, e, e, cor); };

    switch (letra.acento) {
        case AGUDO:       ponto(1, 0); ponto(0, 1); break;
        case CRASE:       ponto(-1, 0); ponto(0, 1); break;
        case CIRCUNFLEXO: ponto(0, 0); ponto(-1, 1); ponto(1, 1); break;
        case TIL:         ponto(0, 0); ponto(2, 0); ponto(-1, 1); ponto(1, 1); break;
        case CEDILHA:     DrawRectangle(cx, y + 7 * e, e, e, cor);
                          DrawRectangle(cx - e, y + 8 * e, e, e, cor); break;
        default: break;
    }
}

void desenharDuasCores(const char* texto, int x, int y, int e, Color cima, Color baixo) {
    int cx = x;
    const char* p = texto;
    while (*p) {
        if (*p == '\n') {
            cx = x;
            y += ALTURA_LINHA * e;
            p++;
            continue;
        }
        Letra letra = traduzir(lerCodigo(p));
        int largura = larguraGlifo(letra.glifo);
        if (letra.glifo) desenharGlifo(letra, cx, y, e, cima, baixo);
        if (letra.acento != NENHUM) desenharAcento(letra, cx, y, e, largura, cima);
        cx += (largura + 1) * e;
    }
}

}  // namespace

int larguraTexto(const char* texto, int escala) {
    int maior = 0, atual = 0;
    const char* p = texto;
    while (*p) {
        if (*p == '\n') {
            p++;
            atual = 0;
            continue;
        }
        atual += larguraGlifo(traduzir(lerCodigo(p)).glifo) + 1;
        if (atual - 1 > maior) maior = atual - 1;
    }
    return maior * escala;
}

void desenharTexto(const char* texto, int x, int y, int escala, Color cor) {
    desenharDuasCores(texto, x, y, escala, cor, cor);
}

void desenharTextoSombra(const char* texto, int x, int y, int escala, Color cor) {
    Color sombra = {38, 26, 22, cor.a};
    desenharDuasCores(texto, x, y + escala, escala, sombra, sombra);
    desenharDuasCores(texto, x, y, escala, cor, cor);
}

void desenharTextoContorno(const char* texto, int x, int y, int escala,
                           Color cima, Color baixo, Color contorno) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            desenharDuasCores(texto, x + dx * escala, y + dy * escala, escala, contorno, contorno);
        }
    }
    desenharDuasCores(texto, x, y + 2 * escala, escala, contorno, contorno);   // sombra embaixo
    desenharDuasCores(texto, x, y, escala, cima, baixo);
}
