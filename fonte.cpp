#include "fonte.h"
#include "pincel.h"

namespace {

const int ALTURA_GLIFO = 9;    // 7 + 2 pro g, p, q, y
const int ALTURA_LINHA = 11;

// cada letra: 5x9, '#' = pixel aceso
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
    {'_', {"", "", "", "", "", "", "", "####"}},
    {'+', {"", "", "..#..", "..#..", "#####", "..#..", "..#.."}},
    {'/', {"....#", "...#.", "...#.", "..#..", ".#...", ".#...", "#...."}},
    {'(', {".#", "#.", "#.", "#.", "#.", "#.", ".#"}},
    {')', {"#.", ".#", ".#", ".#", ".#", ".#", "#."}},
    {'\'', {"#", "#"}},
    {'&', {".##..", "#..#.", "#.#..", ".#...", "#.#.#", "#..#.", ".##.#"}},
    {'>', {"#...", ".#..", "..#.", "...#", "..#.", ".#..", "#..."}},
    {'%', {"##..#", "##..#", "...#.", "..#..", ".#...", "#..##", "#..##"}},
    {'"', {"#.#", "#.#"}},
};
const int TOTAL_GLIFOS = sizeof(GLIFOS) / sizeof(GLIFOS[0]);

enum Acento { NENHUM, AGUDO, CRASE, CIRCUNFLEXO, TIL, CEDILHA };

// letra + acento
struct LetraAcentuada {
    int codigo;
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

// lê um caractere UTF-8
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

// tem pixel aceso nessa posição do glifo?
bool aceso(const Glifo* g, int l, int c, bool semPingo) {
    if (l < 0 || l >= ALTURA_GLIFO || c < 0) return false;
    if (semPingo && l == 0) return false;
    const char* linha = g->linhas[l];
    if (linha == nullptr) return false;
    for (int i = 0; i <= c; i++) {
        if (linha[i] == '\0') return false;
    }
    return linha[c] == '#';
}

// Cada pixel do glifo vira 4 "meios pixels", e os cantos são arredondados
// com uma regra parecida com a do Scale2x: num pixel apagado, se os dois vizinhos
// de um canto estão acesos (e os outros dois não), o cantinho acende. Tira o serrilhado.
void desenharGlifo(const Letra& letra, int x, int y, int e, Color cima, Color baixo) {
    const Glifo* g = letra.glifo;
    bool semPingo = letra.base == 'i' && letra.acento != NENHUM;
    float m = e / 2.0f;
    for (int l = 0; l < ALTURA_GLIFO; l++) {
        for (int c = 0; c < 6; c++) {
            bool p = aceso(g, l, c, semPingo);
            bool a = aceso(g, l - 1, c, semPingo), b = aceso(g, l, c + 1, semPingo);
            bool d = aceso(g, l + 1, c, semPingo), k = aceso(g, l, c - 1, semPingo);
            bool cantos[4] = {p, p, p, p};      // cima-esq, cima-dir, baixo-esq, baixo-dir
            if (!p) {                           // só preenche canto vazio; cortar deixava o D igual ao 0
                cantos[0] = k && a && !d && !b;
                cantos[1] = a && b && !k && !d;
                cantos[2] = d && k && !b && !a;
                cantos[3] = b && d && !a && !k;
            }
            for (int q = 0; q < 4; q++) {
                if (!cantos[q]) continue;
                ret(x + c * e + (q % 2) * m, y + l * e + (q / 2) * m, m, m, l < 4 ? cima : baixo);
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

}

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
    desenharDuasCores(texto, x, y + 2 * escala, escala, contorno, contorno);
    desenharDuasCores(texto, x, y, escala, cima, baixo);
}
