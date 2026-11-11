#include "modelos3d.h"
#include "natureza.h"
#include "paleta.h"
#include "reino.h"
#include "rlgl.h"
#include <cmath>

namespace {

float alfaAtual = 1.0f;

const Color TRONCO = {92, 62, 42, 255};
const Color TRONCO_SECO = {104, 88, 76, 255};
const Color CASCA_BETULA = {226, 222, 208, 255};

// Não tem luz de verdade: cada face é pintada um pouco mais clara ou escura
// conforme o lado pra onde olha (o sol vem do sul, meio de cima).
const float LUZ_TOPO = 1.08f, LUZ_SUL = 0.9f, LUZ_OESTE = 0.8f, LUZ_LESTE = 0.7f, LUZ_NORTE = 0.58f;

Color sombrear(Color cor, float luz) {
    auto canal = [luz](unsigned char v) {
        float r = v * luz;
        return static_cast<unsigned char>(r > 255 ? 255 : r);
    };
    return {canal(cor.r), canal(cor.g), canal(cor.b), static_cast<unsigned char>(cor.a * alfaAtual)};
}

// uma face de 4 cantos (triângulo = repete o último canto)
void face(Vector3 a, Vector3 b, Vector3 c, Vector3 d, Color cor, float luz) {
    Color t = sombrear(cor, luz);
    rlBegin(RL_QUADS);
    rlColor4ub(t.r, t.g, t.b, t.a);
    rlVertex3f(a.x, a.y, a.z);
    rlVertex3f(b.x, b.y, b.z);
    rlVertex3f(c.x, c.y, c.z);
    rlVertex3f(d.x, d.y, d.z);
    rlEnd();
}

// caixa: canto (x, z), largura w (leste-oeste), fundo d (norte-sul), do chão y até y + h
void bloco(float x, float z, float w, float d, float y, float h, Color cor) {
    float x1 = x + w, z1 = z + d, y1 = y + h;
    face({x, y1, z}, {x1, y1, z}, {x1, y1, z1}, {x, y1, z1}, cor, LUZ_TOPO);
    face({x, y, z1}, {x1, y, z1}, {x1, y1, z1}, {x, y1, z1}, cor, LUZ_SUL);
    face({x, y, z}, {x1, y, z}, {x1, y1, z}, {x, y1, z}, cor, LUZ_NORTE);
    face({x, y, z}, {x, y, z1}, {x, y1, z1}, {x, y1, z}, cor, LUZ_OESTE);
    face({x1, y, z}, {x1, y, z1}, {x1, y1, z1}, {x1, y1, z}, cor, LUZ_LESTE);
}

// telhado de duas águas, com a cumeeira no sentido leste-oeste
void telhado(float x, float z, float w, float d, float y, float h, Color cor) {
    float x1 = x + w, z1 = z + d, zm = z + d / 2, y1 = y + h;
    face({x, y, z1}, {x1, y, z1}, {x1, y1, zm}, {x, y1, zm}, cor, 1.0f);
    face({x, y, z}, {x1, y, z}, {x1, y1, zm}, {x, y1, zm}, cor, 0.66f);
    face({x, y, z}, {x, y, z1}, {x, y1, zm}, {x, y1, zm}, cor, LUZ_OESTE);
    face({x1, y, z}, {x1, y, z1}, {x1, y1, zm}, {x1, y1, zm}, cor, LUZ_LESTE);
}

// telhado de 4 lados que fecha num ponto
void piramide(float cx, float cz, float w, float d, float y, float h, Color cor) {
    float x0 = cx - w / 2, x1 = cx + w / 2, z0 = cz - d / 2, z1 = cz + d / 2;
    Vector3 ponta = {cx, y + h, cz};
    face({x0, y, z1}, {x1, y, z1}, ponta, ponta, cor, 1.0f);
    face({x0, y, z0}, {x1, y, z0}, ponta, ponta, cor, 0.62f);
    face({x0, y, z0}, {x0, y, z1}, ponta, ponta, cor, LUZ_OESTE);
    face({x1, y, z0}, {x1, y, z1}, ponta, ponta, cor, LUZ_LESTE);
}

// cone de 6 lados (pinheiro)
void cone(float raio, float y, float h, Color cor) {
    const int LADOS = 6;
    Vector3 ponta = {0, y + h, 0};
    for (int i = 0; i < LADOS; i++) {
        float a0 = i * 2 * PI / LADOS, a1 = (i + 1) * 2 * PI / LADOS, meio = (a0 + a1) / 2;
        float luz = 0.78f + 0.22f * sinf(meio) - 0.08f * cosf(meio);     // sul claro, norte escuro
        face({cosf(a0) * raio, y, sinf(a0) * raio}, {cosf(a1) * raio, y, sinf(a1) * raio}, ponta, ponta, cor, luz);
    }
    face({-raio * 0.8f, y, -raio * 0.8f}, {raio * 0.8f, y, -raio * 0.8f},
         {raio * 0.8f, y, raio * 0.8f}, {-raio * 0.8f, y, raio * 0.8f}, cor, 0.5f);   // fundo da saia
}

// copa "bolinha": três blocos cruzados, parece uma esfera de blocos
void copa(float raio, float y, Color escuro, Color medio, Color claro) {
    float r = raio, miolo = r * 0.68f;
    bloco(-r, -miolo, 2 * r, 2 * miolo, y + r * 0.3f, r * 1.15f, medio);
    bloco(-miolo, -r, 2 * miolo, 2 * r, y + r * 0.3f, r * 1.15f, medio);
    bloco(-miolo, -miolo, 2 * miolo, 2 * miolo, y, r * 0.3f, escuro);
    bloco(-miolo, -miolo, 2 * miolo, 2 * miolo, y + r * 1.45f, r * 0.4f, claro);
}

void mastro(float x, float z, float y, float h, Color pano, float tempo) {
    bloco(x - 0.4f, z - 0.4f, 0.8f, 0.8f, y, h, CONTORNO);
    float onda = sinf(tempo * 4) * 0.5f;
    bloco(x + 0.4f, z - 0.2f, 1.6f, 0.4f, y + h - 2.2f, 2, pano);
    bloco(x + 2.0f, z - 0.2f + onda, 1.6f, 0.4f, y + h - 2.2f, 2, pano);
}

// janela e porta são plaquinhas finas na parede sul (z = frente da parede)
void porta(float x, float z, float w, float h, Color cor) {
    bloco(x, z, w, 0.3f, 0, h, cor);
}

void janelinha(float x, float z, float y, Color cor = JANELA) {
    bloco(x - 0.3f, z, 2.6f, 0.25f, y - 0.3f, 2.6f, MADEIRA_ESCURA);
    bloco(x, z + 0.1f, 2, 0.25f, y, 2, cor);
}

void ameias(float x, float z, float w, float d, float y, Color cor) {
    for (float i = 0; i + 1.5f <= w; i += 3) {
        bloco(x + i, z, 1.5f, 1.2f, y, 1.5f, cor);
        bloco(x + i, z + d - 1.2f, 1.5f, 1.2f, y, 1.5f, cor);
    }
}

void fazenda() {
    bloco(-9, -8, 18, 16, 0, 0.8f, TERRA);
    for (float z = -6.5f; z < 7; z += 4) {
        bloco(-8, z, 16, 1.6f, 0.8f, 1.4f, GRAMA_CLARA);
        for (float x = -7; x < 8; x += 3) bloco(x, z + 0.4f, 0.8f, 0.8f, 2.2f, 1.6f, DOURADO);
    }
    for (float x = -9.5f; x <= 9; x += 4.5f) {              // cerca
        bloco(x, -8.5f, 0.7f, 0.7f, 0, 3, MADEIRA);
        bloco(x, 7.8f, 0.7f, 0.7f, 0, 3, MADEIRA);
    }
    bloco(-9.5f, -8.3f, 19, 0.3f, 2, 0.5f, MADEIRA_ESCURA);
    bloco(-9.5f, 8.0f, 19, 0.3f, 2, 0.5f, MADEIRA_ESCURA);
}

void serraria() {
    bloco(-9, -4, 12, 9, 0, 8, MADEIRA);
    telhado(-10, -5, 14, 11, 8, 5, TELHADO_VERM);
    bloco(-1, -2.5f, 2, 2, 9, 6.5f, PEDRA);
    porta(-4.5f, 5, 3, 5, MADEIRA_ESCURA);
    janelinha(-8, 5, 3.5f);
    bloco(4, 1, 7, 2.6f, 0, 2.6f, MADEIRA);                  // toras empilhadas
    bloco(4, 3.8f, 7, 2.6f, 0, 2.6f, MADEIRA);
    bloco(4, 2.3f, 7, 2.6f, 2.6f, 2.4f, MADEIRA);
    bloco(10.9f, 1.3f, 0.3f, 2, 0.3f, 2, PERGAMINHO);
    bloco(10.9f, 2.6f, 0.3f, 2, 2.9f, 1.8f, PERGAMINHO);
}

void quartel(float tempo) {
    bloco(-9, -5, 16, 10, 0, 9, PERGAMINHO);
    telhado(-10, -6, 18, 12, 9, 5, TELHADO_VERM);
    porta(-3, 5, 4, 6, MADEIRA_ESCURA);
    janelinha(-7.5f, 5, 4.5f);
    janelinha(3.5f, 5, 4.5f);
    mastro(9, 4, 0, 18, TELHADO_AZUL, tempo);
}

void muralha() {
    bloco(-10, -2.5f, 20, 5, 0, 11, PEDRA);
    bloco(-10.3f, -2.8f, 20.6f, 5.6f, 3.5f, 0.6f, PEDRA_CLARA);
    bloco(-10.3f, -2.8f, 20.6f, 5.6f, 7.5f, 0.6f, PEDRA_CLARA);
    ameias(-10, -2.5f, 20, 5, 11, PEDRA);
}

void oficina() {
    bloco(-9, -5, 13, 10, 0, 9, PEDRA);
    telhado(-10, -6, 15, 12, 9, 4, MADEIRA_ESCURA);
    bloco(0, -3, 2.5f, 2.5f, 9, 8, PEDRA_CLARA);
    porta(-5, 5, 4, 6, CONTORNO);
    bloco(-4.5f, 5.1f, 3, 0.3f, 0, 3.5f, BRASA);             // fogo da forja
    bloco(-3.8f, 5.2f, 1.6f, 0.3f, 0, 2, JANELA);
    janelinha(-8.5f, 5, 5);
    bloco(6, 2.5f, 3, 2, 0, 1.6f, CONTORNO);                 // bigorna
    bloco(5.2f, 2, 4.6f, 3, 1.6f, 1.4f, CONTORNO);
}

void mina() {
    bloco(-9, -7, 18, 13, 0, 8, PEDRA);
    bloco(-7, -6, 14, 10, 8, 3, PEDRA);
    bloco(-4, -5, 8, 7, 11, 2.5f, PEDRA_CLARA);
    bloco(-10, -3, 3, 6, 0, 4, PEDRA_CLARA);
    bloco(-3, 6, 6, 0.4f, 0, 7, CONTORNO);                   // boca da mina
    bloco(-4, 6, 1, 0.8f, 0, 8, MADEIRA);
    bloco(3, 6, 1, 0.8f, 0, 8, MADEIRA);
    bloco(-4, 6, 8, 0.8f, 7.2f, 1, MADEIRA);
    bloco(-7, 6.1f, 1, 0.5f, 5, 1.5f, JANELA);               // lanterna
    bloco(5, 7.5f, 4.5f, 3, 0.6f, 2.4f, MADEIRA_ESCURA);     // carrinho
    bloco(5.5f, 8, 3.5f, 2, 3, 0.8f, PEDRA_CLARA);
    bloco(6, 8.3f, 1, 1, 3.6f, 0.7f, DOURADO);
}

void mercado() {
    bloco(-8, -1, 16, 5, 0, 5, MADEIRA);
    bloco(-8.3f, -1.3f, 16.6f, 5.6f, 5, 0.6f, MADEIRA_ESCURA);
    for (float x = -8.5f; x <= 8; x += 16) {
        bloco(x, -3.5f, 1, 1, 0, 11, MADEIRA_ESCURA);
        bloco(x, 4.5f, 1, 1, 0, 10, MADEIRA_ESCURA);
    }
    for (int i = 0; i < 6; i++) {                            // toldo listrado
        telhado(-9 + i * 3, -4.5f, 3, 11, 10, 3, i % 2 ? PERGAMINHO : TELHADO_VERM);
    }
    bloco(-6.5f, 0.5f, 2, 2, 5.6f, 1.2f, TELHADO_VERM);      // maçãs, pão e tecido
    bloco(-2, 0.5f, 3, 2, 5.6f, 1, DOURADO_ESCURO);
    bloco(3.5f, 0.5f, 3, 2, 5.6f, 1.4f, TELHADO_AZUL);
    bloco(6, 5, 3, 3, 0, 2.5f, PERGAMINHO_ESC);
}

void casa() {
    bloco(-6, -5, 12, 10, 0, 8, PERGAMINHO);
    bloco(-6.2f, -5.2f, 0.8f, 10.4f, 0, 8, MADEIRA_ESCURA);  // vigas do enxaimel
    bloco(5.4f, -5.2f, 0.8f, 10.4f, 0, 8, MADEIRA_ESCURA);
    bloco(-6.2f, 4.7f, 12.4f, 0.5f, 4, 0.7f, MADEIRA_ESCURA);
    telhado(-7.5f, -6.5f, 15, 13, 8, 6.5f, DOURADO_ESCURO);
    bloco(-7.5f, 5.6f, 15, 0.9f, 8, 0.8f, DOURADO);
    bloco(2.5f, -3, 2, 2, 10, 6, PEDRA);
    porta(-2, 5, 3, 4.5f, MADEIRA_ESCURA);
    janelinha(2.2f, 5, 4.3f);
    bloco(2, 5.4f, 2.6f, 0.9f, 3.2f, 0.8f, MADEIRA);         // floreira
    bloco(2.4f, 5.6f, 0.6f, 0.6f, 4, 0.7f, FLOR_AMARELA);
    bloco(3.6f, 5.6f, 0.6f, 0.6f, 4, 0.7f, CORACAO_CLARO);
}

void torre(float tempo) {
    bloco(-4, -4, 8, 8, 0, 15, PEDRA);
    bloco(-4.3f, -4.3f, 8.6f, 8.6f, 7, 0.7f, PEDRA_CLARA);
    bloco(-5, -5, 10, 10, 15, 2, PEDRA);
    ameias(-5, -5, 10, 10, 17, PEDRA_CLARA);
    porta(-2, 4, 4, 4, MADEIRA_ESCURA);
    bloco(-1, 4, 2, 0.3f, 8.5f, 3, CONTORNO);                // seteiras
    bloco(4, -1, 0.3f, 2, 8.5f, 3, CONTORNO);
    bloco(-4.3f, -1, 0.3f, 2, 8.5f, 3, CONTORNO);
    mastro(0, 0, 17, 7, TELHADO_VERM, tempo);
}

void arquearia() {
    bloco(-10, -4, 9, 8, 0, 8, MADEIRA);
    telhado(-11, -5, 11, 10, 8, 4, FOLHA);
    porta(-7, 4, 3, 5, MADEIRA_ESCURA);
    bloco(3.5f, 1.5f, 0.8f, 0.8f, 0, 6, MADEIRA_ESCURA);     // alvo no cavalete
    bloco(8.7f, 1.5f, 0.8f, 0.8f, 0, 6, MADEIRA_ESCURA);
    bloco(3, 2, 7, 1, 4.5f, 7, PERGAMINHO);
    bloco(4, 3, 5, 0.3f, 5.5f, 5, TELHADO_VERM);
    bloco(5, 3.2f, 3, 0.3f, 6.5f, 3, PERGAMINHO);
    bloco(6, 3.4f, 1, 0.3f, 7.5f, 1, TELHADO_VERM);
    bloco(6.3f, 3.4f, 0.5f, 4, 7.8f, 0.5f, MADEIRA);         // flecha cravada
}

void estabulo() {
    bloco(-8, -5, 16, 10, 0, 10, TELHADO_VERM);
    bloco(-8.2f, -5.2f, 16.4f, 10.4f, 0, 2, VERMELHO_ESC);
    telhado(-9, -6, 18, 12, 10, 6, MADEIRA_ESCURA);
    porta(-3, 5, 6, 7, MADEIRA);
    bloco(-3, 5.3f, 6, 0.2f, 6.4f, 0.6f, PERGAMINHO);
    bloco(-3, 5.3f, 0.6f, 0.2f, 0, 7, PERGAMINHO);
    bloco(2.4f, 5.3f, 0.6f, 0.2f, 0, 7, PERGAMINHO);
    janelinha(-1, 5, 11, DOURADO);
    bloco(6, 6, 4, 3, 0, 3, DOURADO);                        // feno
    bloco(6, 6.8f, 4, 0.5f, 1.2f, 0.5f, DOURADO_ESCURO);
}

void torreArcana(float tempo) {
    bloco(-4.5f, -4.5f, 9, 9, 0, 2, PEDRA);
    bloco(-3.5f, -3.5f, 7, 7, 2, 12, ROXO);
    piramide(0, 0, 10, 10, 14, 9, ROXO_ESC);
    porta(-1, 3.5f, 2, 5, CONTORNO);
    bloco(-1, 3.5f, 2, 0.3f, 8, 3, CRISTAL);                 // janelas brilhando
    bloco(3.5f, -1, 0.3f, 2, 8, 3, CRISTAL);
    bloco(-3.8f, -1, 0.3f, 2, 8, 3, CRISTAL);
    float flutua = sinf(tempo * 2) * 0.8f;
    bloco(-1, -1, 2, 2, 25 + flutua, 2.4f, CRISTAL);         // cristal flutuando
    bloco(-0.5f, -0.5f, 1, 1, 24.3f + flutua, 3.8f, TEXTO_CLARO);
}

void castelo(float tempo) {
    bloco(-8, -6, 16, 12, 0, 10, PEDRA);
    ameias(-8, -6, 16, 12, 10, PEDRA);
    for (int i = 0; i < 4; i++) {                            // quatro torres nos cantos
        float x = (i % 2) ? 5.0f : -10.0f, z = (i / 2) ? 3.0f : -8.0f;
        bloco(x, z, 5, 5, 0, 15, PEDRA_CLARA);
        piramide(x + 2.5f, z + 2.5f, 6.4f, 6.4f, 15, 5, TELHADO_AZUL);
        bloco(x + 1.7f, z + 5, 1.6f, 0.3f, 9, 2.5f, JANELA);
    }
    bloco(-3.5f, -3.5f, 7, 7, 0, 18, PEDRA_CLARA);           // torre do meio
    piramide(0, 0, 9, 9, 18, 6, TELHADO_VERM);
    bloco(-1, 3.5f, 2, 0.3f, 13, 2.5f, JANELA);
    bloco(-2, 6, 4, 0.3f, 0, 6, CONTORNO);                   // portão
    bloco(-2.6f, 6, 0.6f, 0.6f, 0, 6.6f, PEDRA_CLARA);
    bloco(2, 6, 0.6f, 0.6f, 0, 6.6f, PEDRA_CLARA);
    bloco(-2.6f, 6, 5.2f, 0.6f, 6, 0.8f, PEDRA_CLARA);
    mastro(0, 0, 24, 6, TELHADO_VERM, tempo);
}

void comecarModelo(Vector3 base, float cresce) {
    float escala = 1.0f / UNIDADES_POR_METRO;
    rlSetTexture(rlGetTextureIdDefault());   // senão as faces saem pintadas com a textura do chão
    rlPushMatrix();
    rlTranslatef(base.x, base.y, base.z);
    rlScalef(escala * (2 - cresce), escala * cresce, escala * (2 - cresce));
}

}

void desenharPredio3D(int tipo, Vector3 base, float cresce, float alfa, float tempo) {
    alfaAtual = alfa;
    comecarModelo(base, cresce);
    switch (tipo) {
        case FAZENDA: fazenda(); break;
        case SERRARIA: serraria(); break;
        case QUARTEL: quartel(tempo); break;
        case MURALHA: muralha(); break;
        case OFICINA: oficina(); break;
        case MINA: mina(); break;
        case MERCADO: mercado(); break;
        case CASA: casa(); break;
        case TORRE: torre(tempo); break;
        case ARQUEARIA: arquearia(); break;
        case ESTABULO: estabulo(); break;
        default: torreArcana(tempo); break;
    }
    rlPopMatrix();
    alfaAtual = 1.0f;
}

void desenharCastelo3D(Vector3 base, float tempo) {
    comecarModelo(base, 1);
    castelo(tempo);
    rlPopMatrix();
}

void desenharArvore3D(int tipo, int tamanho, int tom, Vector3 base, float vento) {
    Color escuro, medio, claro;
    coresDaFolha(tom, escuro, medio, claro);
    comecarModelo(base, 1);
    rlRotatef(vento * 2.5f, 0, 0, 1);                        // balança com o vento
    switch (tipo) {
        case PINHEIRO: {
            float altura = 8 + tamanho * 3, raio = 3 + tamanho;
            bloco(-0.6f, -0.6f, 1.2f, 1.2f, 0, 2.5f, TRONCO);
            cone(raio, 2, altura * 0.5f, escuro);
            cone(raio * 0.78f, 2 + altura * 0.3f, altura * 0.45f, medio);
            cone(raio * 0.52f, 2 + altura * 0.58f, altura * 0.42f, claro);
            break;
        }
        case CARVALHO: {
            float raio = 3 + tamanho, tronco = 2.5f + tamanho * 0.5f;
            bloco(-0.7f, -0.7f, 1.4f, 1.4f, 0, tronco + 1, TRONCO);
            copa(raio, tronco, escuro, medio, claro);
            break;
        }
        case BETULA: {
            float tronco = 4 + tamanho * 2, raio = 2 + tamanho;
            bloco(-0.5f, -0.5f, 1, 1, 0, tronco + 1, CASCA_BETULA);
            for (float y = 1; y < tronco; y += 2) bloco(-0.55f, -0.55f, 1.1f, 1.1f, y, 0.4f, CONTORNO);
            copa(raio, tronco, escuro, medio, claro);
            break;
        }
        case ARBUSTO: {
            float raio = 2 + tamanho;
            copa(raio, 0, escuro, medio, claro);
            if (tamanho == 1) {
                bloco(-raio - 0.2f, 0.5f, 0.7f, 0.7f, raio, 0.7f, {196, 52, 52, 255});   // frutinhas
                bloco(1, raio * 0.68f, 0.7f, 0.7f, raio * 1.2f, 0.7f, {196, 52, 52, 255});
            }
            break;
        }
        default: {
            float altura = 6 + tamanho * 2;
            bloco(-0.5f, -0.5f, 1, 1, 0, altura, TRONCO_SECO);
            bloco(-2.5f, -0.3f, 2, 0.6f, altura - 3, 0.6f, TRONCO_SECO);
            bloco(-2.5f, -0.3f, 0.6f, 0.6f, altura - 3, 2, TRONCO_SECO);
            bloco(0.5f, -0.3f, 2, 0.6f, altura - 1.5f, 0.6f, TRONCO_SECO);
            bloco(2, -0.3f, 0.6f, 0.6f, altura - 1.5f, 2, TRONCO_SECO);
            break;
        }
    }
    rlPopMatrix();
}
