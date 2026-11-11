#include "visao3d.h"
#include "fonte.h"
#include "paleta.h"
#include "pincel.h"
#include "sprites.h"
#include "rlgl.h"
#include <cmath>

namespace {

const float PIXEL = 16.0f;              // pixels do mapa por unidade 3D
const float ALTURA_PLANALTO = 0.45f;
const Color CEU = {30, 36, 54, 255};
const Color CEU_HORIZONTE = {74, 62, 72, 255};

InfoMapa mapa;
RenderTexture2D chao;
Shader descarte;                        // joga fora pixel transparente
Camera3D camera;
float giro = 0.0f, inclinacao = 0.8f, distancia = 13.0f;

// texturas pré-desenhadas das figuras
RenderTexture2D texArvore[TOTAL_TIPOS_ARVORE][3][4][3];   // tipo, tamanho, tom, vento
RenderTexture2D texPredio[TOTAL_TIPOS_EDIFICIO];
RenderTexture2D texCastelo;
RenderTexture2D texGalinha[2][2][2];                      // direção, passo, cabeça

const char* SHADER_DESCARTE =
    "#version 330\n"
    "in vec2 fragTexCoord; in vec4 fragColor;\n"
    "uniform sampler2D texture0; uniform vec4 colDiffuse;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec4 c = texture(texture0, fragTexCoord) * colDiffuse * fragColor;\n"
    "    if (c.a < 0.5) discard;\n"
    "    finalColor = c;\n"
    "}\n";

// mapa 2D (pixels) -> mundo 3D
Vector3 noMundo(float x, float y, float altura) {
    return {(x - mapa.largura / 2.0f) / PIXEL, altura, (y - mapa.altura / 2.0f) / PIXEL};
}

Rectangle retanguloPlanalto() {
    float lado = mapa.grade * mapa.lote + 6.0f;
    return {mapa.gradeX - 3.0f, mapa.gradeY - 3.0f, lado, lado};
}

float alturaDoChao(float x, float y) {
    return CheckCollisionPointRec({x, y}, retanguloPlanalto()) ? ALTURA_PLANALTO : 0.0f;
}

// desenha uma figura nova numa textura (fundo transparente)
RenderTexture2D assar(int largura, int altura) {
    RenderTexture2D rt = LoadRenderTexture(largura * FATOR, altura * FATOR);
    SetTextureFilter(rt.texture, TEXTURE_FILTER_POINT);
    return rt;
}

void comecarAssado(RenderTexture2D& rt) {
    BeginTextureMode(rt);
    ClearBackground(BLANK);
    comecarZoom();
}

void terminarAssado() {
    terminarZoom();
    EndTextureMode();
}

void assarArvores() {
    for (int tipo = 0; tipo < TOTAL_TIPOS_ARVORE; tipo++)
        for (int tam = 0; tam < 3; tam++)
            for (int tom = 0; tom < 4; tom++)
                for (int q = 0; q < 3; q++) {
                    RenderTexture2D& rt = texArvore[tipo][tam][tom][q] = assar(24, 30);
                    comecarAssado(rt);
                    desenharArvoreVariada(12, 28, tipo, tam, tom, q - 1.0f);
                    terminarAssado();
                }
}

void assarPredios() {
    for (int i = 0; i < TOTAL_TIPOS_EDIFICIO; i++) {
        texPredio[i] = assar(24, 30);
        comecarAssado(texPredio[i]);
        desenharComContorno(mapa.predios[i], 1, 7, 1);
        terminarAssado();
    }
    texCastelo = assar(24, 30);
    comecarAssado(texCastelo);
    desenharComContorno(desenharCastelo, 1, 7, 1);
    terminarAssado();
}

void assarGalinhas() {
    for (int d = 0; d < 2; d++)
        for (int p = 0; p < 2; p++)
            for (int c = 0; c < 2; c++) {
                RenderTexture2D& rt = texGalinha[d][p][c] = assar(9, 8);
                comecarAssado(rt);
                desenharGalinhaEm(4, 6, d == 0 ? -1 : 1, p, c == 1);
                terminarAssado();
            }
}

// figura em pé, sempre virada pra câmera (só gira em volta do eixo y)
// (x, y) = pé da figura no mapa 2D; largura/altura em pixels
void figura(const Texture2D& tex, float x, float y, float afundar, Color tinta, float cresce = 1.0f) {
    float w = tex.width / (PIXEL * FATOR) * (2.0f - cresce), h = tex.height / (PIXEL * FATOR) * cresce;
    Vector3 base = noMundo(x, y, alturaDoChao(x, y) - afundar / PIXEL);
    Vector3 frente = {camera.target.x - camera.position.x, 0, camera.target.z - camera.position.z};
    float tamanho = sqrtf(frente.x * frente.x + frente.z * frente.z);
    Vector3 direita = {-frente.z / tamanho * w / 2, 0, frente.x / tamanho * w / 2};

    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);
    rlColor4ub(tinta.r, tinta.g, tinta.b, tinta.a);
    rlTexCoord2f(0, 0); rlVertex3f(base.x - direita.x, base.y, base.z - direita.z);
    rlTexCoord2f(1, 0); rlVertex3f(base.x + direita.x, base.y, base.z + direita.z);
    rlTexCoord2f(1, 1); rlVertex3f(base.x + direita.x, base.y + h, base.z + direita.z);
    rlTexCoord2f(0, 1); rlVertex3f(base.x - direita.x, base.y + h, base.z - direita.z);
    rlEnd();
    rlSetTexture(0);
}

// pedaço do chão (retângulo do mapa 2D) deitado na altura y
void pedacoDoChao(Rectangle r, float y) {
    float u0 = r.x / mapa.largura, u1 = (r.x + r.width) / mapa.largura;
    float v0 = 1 - r.y / mapa.altura, v1 = 1 - (r.y + r.height) / mapa.altura;
    Vector3 a = noMundo(r.x, r.y, y), b = noMundo(r.x + r.width, r.y + r.height, y);
    rlSetTexture(chao.texture.id);
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);
    rlTexCoord2f(u0, v0); rlVertex3f(a.x, y, a.z);
    rlTexCoord2f(u0, v1); rlVertex3f(a.x, y, b.z);
    rlTexCoord2f(u1, v1); rlVertex3f(b.x, y, b.z);
    rlTexCoord2f(u1, v0); rlVertex3f(b.x, y, a.z);
    rlEnd();
    rlSetTexture(0);
}

// a ilha: chão em volta, barranco embaixo e o planalto do reino em relevo
void desenharTerreno() {
    float L = mapa.largura / PIXEL, A = mapa.altura / PIXEL;
    DrawCube({0, -0.62f, 0}, L, 1.2f, A, TERRA_ESCURA);   // um pouco abaixo do chão, sem brigar com ele
    DrawCube({0, -1.25f, 0}, L - 0.4f, 0.1f, A - 0.4f, Fade(CONTORNO, 0.6f));
    pedacoDoChao({0, 0, static_cast<float>(mapa.largura), static_cast<float>(mapa.altura)}, 0.001f);

    Rectangle p = retanguloPlanalto();
    Vector3 centro = noMundo(p.x + p.width / 2, p.y + p.height / 2, ALTURA_PLANALTO / 2);
    DrawCube(centro, p.width / PIXEL, ALTURA_PLANALTO - 0.01f, p.height / PIXEL, TERRA);
    DrawCubeWires(centro, p.width / PIXEL, ALTURA_PLANALTO - 0.01f, p.height / PIXEL, TERRA_ESCURA);
    pedacoDoChao(p, ALTURA_PLANALTO);
}

void desenharArvoresDoCenario(const Cenario& c, float tempo) {
    for (int i = 0; i < c.total; i++) {
        const ObjetoCenario& o = c.objetos[i];
        if (o.tipo >= TOTAL_TIPOS_ARVORE) continue;
        float vento = ventoNaArvore(tempo, o.fase);
        int q = vento < -0.5f ? 0 : vento > 0.5f ? 2 : 1;
        figura(texArvore[o.tipo][o.tamanho][o.tom % 4][q].texture, o.x, o.y, 2, WHITE);
    }
}

void desenharFlorestaDosLotes(const Reino& reino, float tempo) {
    for (int l = 0; l < mapa.grade; l++) {
        for (int c = 0; c < mapa.grade; c++) {
            if (loteLiberado(reino, l, c)) continue;
            int semente = l * 7 + c * 13 + 5;
            for (int i = 0; i < 3; i++) {
                int dx, dy, tipo, tamanho, tom;
                arvoreDoLote(semente, i, dx, dy, tipo, tamanho, tom);
                float vento = ventoNaArvore(tempo, semente + i * 2.0f);
                int q = vento < -0.5f ? 0 : vento > 0.5f ? 2 : 1;
                float x = mapa.gradeX + c * mapa.lote + dx, y = mapa.gradeY + l * mapa.lote + dy;
                figura(texArvore[tipo][tamanho][tom][q].texture, x, y, 2, WHITE);
            }
        }
    }
}

Vector2 peDoLote(int linha, int coluna) {
    return {mapa.gradeX + coluna * mapa.lote + 11.0f, mapa.gradeY + linha * mapa.lote + 17.0f};
}

void desenharPredios(const Reino& reino, const Previa3D& previa) {
    Vector2 castelo = peDoLote(2, 2);
    figura(texCastelo.texture, castelo.x, castelo.y, 0, WHITE);
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        Vector2 pe = peDoLote(no->dados.linha, no->dados.coluna);
        float cresce = previa.crescimento[no->dados.linha][no->dados.coluna];
        figura(texPredio[no->dados.tipo].texture, pe.x, pe.y, 0, WHITE, cresce);
    }
}

void desenharGalinhas3D(float tempo) {
    EstadoGalinha galinhas[10];
    int total = obterGalinhas(galinhas, 10, tempo);
    for (int i = 0; i < total; i++) {
        const EstadoGalinha& g = galinhas[i];
        const Texture2D& tex = texGalinha[g.direcao > 0][g.passo][g.cabecaBaixa].texture;
        figura(tex, g.x, g.y + 2, 1, WHITE);
    }
}

// fumaça vira cubinhos subindo
void desenharFumaca3D() {
    EstadoFumaca fumacas[80];
    int total = obterFumacas(fumacas, 80);
    for (int i = 0; i < total; i++) {
        const EstadoFumaca& f = fumacas[i];
        float pe = f.origemY + 17;
        float altura = ALTURA_PLANALTO + (pe - f.y) / PIXEL;
        Vector3 pos = noMundo(f.x, pe, altura);
        float lado = f.tamanho / PIXEL;
        DrawCube(pos, lado, lado, lado, Fade({206, 200, 192, 255}, f.vida / 2.6f * 0.8f));
    }
}

void desenharFantasmas(const Reino& reino, const Previa3D& previa, float tempo) {
    float pulso = 0.45f + 0.25f * sinf(tempo * 6.0f);
    for (NoFila* no = reino.ordens.inicio; no != nullptr; no = no->proximo) {
        Vector2 pe = peDoLote(no->ordem.linha, no->ordem.coluna);
        figura(texPredio[no->ordem.tipoEdificio].texture, pe.x, pe.y, 0, Fade(WHITE, 0.45f));
    }
    if (previa.tipo >= 0) {
        Vector2 pe = peDoLote(previa.linha, previa.coluna);
        figura(texPredio[previa.tipo].texture, pe.x, pe.y, 0, Fade(WHITE, pulso + 0.2f));
    }
}

// número da posição na fila, em cima de cada fantasma
void desenharNumerosDaFila(const Reino& reino) {
    int posicao = 1;
    for (NoFila* no = reino.ordens.inicio; no != nullptr; no = no->proximo, posicao++) {
        Vector2 pe = peDoLote(no->ordem.linha, no->ordem.coluna);
        Vector3 topo = noMundo(pe.x, pe.y, ALTURA_PLANALTO + 1.9f);
        Vector2 t = GetWorldToScreenEx(topo, camera, mapa.largura, mapa.altura);
        DrawRectangle(static_cast<int>(t.x) - 3, static_cast<int>(t.y) - 1, 7, 9, CONTORNO);
        desenharTexto(TextFormat("%d", posicao), static_cast<int>(t.x) - 2, static_cast<int>(t.y), 1, DOURADO);
    }
}

// bolinhas douradas = nível do prédio
void desenharNiveis(const Reino& reino) {
    for (NoEdificio* no = reino.edificios.primeiro; no != nullptr; no = no->proximo) {
        const Edificio& e = no->dados;
        if (e.nivel < 2) continue;
        Vector2 pe = peDoLote(e.linha, e.coluna);
        Vector3 topo = noMundo(pe.x, pe.y, ALTURA_PLANALTO + 1.75f);
        Vector2 t = GetWorldToScreenEx(topo, camera, mapa.largura, mapa.altura);
        int x = static_cast<int>(t.x) - e.nivel * 2, y = static_cast<int>(t.y);
        DrawRectangle(x - 1, y - 1, e.nivel * 4 + 1, 5, CONTORNO);
        for (int i = 0; i < e.nivel; i++) DrawRectangle(x + i * 4, y, 3, 3, DOURADO);
    }
}

void posicionarCamera() {
    Rectangle p = retanguloPlanalto();
    camera.target = noMundo(p.x + p.width / 2, p.y + p.height / 2 + 6, ALTURA_PLANALTO);
    float horizontal = distancia * cosf(inclinacao);
    camera.position = {camera.target.x + horizontal * sinf(giro),
                       camera.target.y + distancia * sinf(inclinacao),
                       camera.target.z + horizontal * cosf(giro)};
}

}

void iniciarVisao3D(const InfoMapa& info) {
    mapa = info;
    chao = LoadRenderTexture(mapa.largura * FATOR, mapa.altura * FATOR);
    SetTextureFilter(chao.texture, TEXTURE_FILTER_POINT);
    descarte = LoadShaderFromMemory(nullptr, SHADER_DESCARTE);
    camera.up = {0, 1, 0};
    camera.fovy = 40;
    camera.projection = CAMERA_PERSPECTIVE;
    resetarCamera3D();
    assarArvores();
    assarPredios();
    assarGalinhas();
}

void liberarVisao3D() {
    for (auto& tipo : texArvore)
        for (auto& tam : tipo)
            for (auto& tom : tam)
                for (RenderTexture2D& rt : tom) UnloadRenderTexture(rt);
    for (RenderTexture2D& rt : texPredio) UnloadRenderTexture(rt);
    for (auto& d : texGalinha)
        for (auto& p : d)
            for (RenderTexture2D& rt : p) UnloadRenderTexture(rt);
    UnloadRenderTexture(texCastelo);
    UnloadRenderTexture(chao);
    UnloadShader(descarte);
}

RenderTexture2D& texturaDoChao() {
    return chao;
}

void resetarCamera3D() {
    giro = 0.0f;
    inclinacao = 0.8f;
    distancia = 13.0f;
    posicionarCamera();
}

// Q/E ou setas giram, W/S ou setas inclinam, roda dá zoom, botão do meio arrasta
void atualizarCamera3D(float dt) {
    if (IsKeyDown(KEY_Q) || IsKeyDown(KEY_LEFT)) giro -= 1.6f * dt;
    if (IsKeyDown(KEY_E) || IsKeyDown(KEY_RIGHT)) giro += 1.6f * dt;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) inclinacao += 0.9f * dt;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) inclinacao -= 0.9f * dt;
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) giro -= GetMouseDelta().x * 0.01f;
    distancia -= GetMouseWheelMove() * 0.8f;
    if (IsKeyPressed(KEY_R)) resetarCamera3D();
    inclinacao = fminf(fmaxf(inclinacao, 0.45f), 1.4f);
    distancia = fminf(fmaxf(distancia, 5.0f), 22.0f);
    posicionarCamera();
}

bool mouseNoMapa3D(Vector2 mouse, Vector2& ponto) {
    Ray raio = GetScreenToWorldRayEx(mouse, camera, mapa.largura, mapa.altura);
    if (raio.direction.y >= 0) return false;
    float t = (ALTURA_PLANALTO - raio.position.y) / raio.direction.y;
    float x = raio.position.x + raio.direction.x * t;
    float z = raio.position.z + raio.direction.z * t;
    ponto = {x * PIXEL + mapa.largura / 2.0f, z * PIXEL + mapa.altura / 2.0f};
    return true;
}

void desenharCena3D(const Cenario& cenario, const Reino& reino, const Previa3D& previa, float tempo) {
    ClearBackground(CEU);                    // limpa a profundidade do quadro anterior também
    DrawRectangleGradientV(0, 0, mapa.largura, mapa.altura, CEU, CEU_HORIZONTE);
    terminarZoom();                          // o 3D não usa a ampliação do 2D
    BeginMode3D(camera);
    rlDisableBackfaceCulling();
    desenharTerreno();

    BeginShaderMode(descarte);
    desenharArvoresDoCenario(cenario, tempo);
    desenharFlorestaDosLotes(reino, tempo);
    desenharPredios(reino, previa);
    desenharGalinhas3D(tempo);
    EndShaderMode();

    desenharFantasmas(reino, previa, tempo);
    desenharFumaca3D();
    rlEnableBackfaceCulling();
    EndMode3D();
    comecarZoom();
    desenharNumerosDaFila(reino);
    desenharNiveis(reino);
}
