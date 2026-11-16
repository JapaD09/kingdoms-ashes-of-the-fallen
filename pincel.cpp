#include "pincel.h"
#include "rlgl.h"

namespace {

bool silhueta = false;
float deslocX = 0, deslocY = 0;
Color corSilhueta = BLACK;

}

// Liga a câmera 2D que aumenta tudo pelo FATOR (desenhamos em 320x180 e a textura é maior).
void comecarZoom() {
    Camera2D camera = {};
    camera.zoom = static_cast<float>(FATOR);
    BeginMode2D(camera);
}

// Desliga a câmera do comecarZoom.
void terminarZoom() {
    EndMode2D();
}

// Retângulo base de todo desenho. No modo silhueta pinta tudo com a cor do contorno, deslocado.
void ret(float x, float y, float w, float h, Color cor) {
    if (silhueta) {
        if (cor.a < 200) return;         // sombra, brilho e fumaça não têm contorno
        DrawRectangleRec({x + deslocX, y + deslocY, w, h}, corSilhueta);
        return;
    }
    DrawRectangleRec({x, y, w, h}, cor);
}

// Os 4 primeiros passos desenham a silhueta deslocada pra cada lado; o 5º desenha normal.
void passoDoContorno(int passo, Color cor) {
    const float lados[4][2] = {{-0.5f, 0}, {0.5f, 0}, {0, -0.5f}, {0, 0.5f}};
    silhueta = passo < 4;
    if (!silhueta) return;
    deslocX = lados[passo][0];
    deslocY = lados[passo][1];
    corSilhueta = cor;
}

// Estica o que for desenhado em volta do ponto pivô (usado na animação do prédio sendo construído).
void comecarEscala(float pivoX, float pivoY, float escalaX, float escalaY) {
    rlPushMatrix();
    rlTranslatef(pivoX, pivoY, 0);
    rlScalef(escalaX, escalaY, 1);
    rlTranslatef(-pivoX, -pivoY, 0);
}

// Desfaz o comecarEscala.
void terminarEscala() {
    rlPopMatrix();
}
