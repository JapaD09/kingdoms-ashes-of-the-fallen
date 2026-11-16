#include "som.h"
#include "raylib.h"
#include <cmath>
#include <cstdlib>

namespace {

const int TAXA = 22050;          // amostras por segundo
const int MAX_NOTAS = 4;

struct Nota {
    float frequencia;            // 0 = ruído (batida)
    float duracao;               // segundos
};

Sound sons[TOTAL_SONS];
bool pronto = false;
bool ligado = true;

// monta as notas uma atrás da outra; cada uma começa forte e some (envelope)
Sound montar(const Nota notas[], int total, float volume) {
    float duracao = 0;
    for (int i = 0; i < total; i++) duracao += notas[i].duracao;
    int amostras = static_cast<int>(duracao * TAXA);
    short* dados = static_cast<short*>(MemAlloc(amostras * sizeof(short)));

    int pos = 0;
    for (int n = 0; n < total; n++) {
        int tamanho = static_cast<int>(notas[n].duracao * TAXA);
        for (int i = 0; i < tamanho && pos < amostras; i++, pos++) {
            float t = static_cast<float>(i) / TAXA;
            float some = 1.0f - static_cast<float>(i) / tamanho;
            float onda;
            if (notas[n].frequencia <= 0) {
                onda = (rand() % 2001 - 1000) / 1000.0f;                    // ruído
            } else {
                onda = sinf(2 * PI * notas[n].frequencia * t) > 0 ? 1.0f : -1.0f;   // onda quadrada
            }
            dados[pos] = static_cast<short>(onda * some * some * volume * 32000);
        }
    }
    Wave onda = {static_cast<unsigned int>(amostras), TAXA, 16, 1, dados};
    Sound som = LoadSoundFromWave(onda);
    UnloadWave(onda);
    return som;
}

}

// Liga o áudio e prepara os efeitos. Se não tiver placa de som, o jogo segue mudo.
void iniciarSom() {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) return;       // sem placa de som: o jogo segue mudo
    SetMasterVolume(0.5f);

    const Nota clique[] = {{700, 0.035f}};
    const Nota carta[] = {{520, 0.05f}, {780, 0.07f}};
    const Nota construir[] = {{0, 0.07f}, {196, 0.06f}, {294, 0.10f}};
    const Nota golpe[] = {{0, 0.06f}, {110, 0.04f}};
    const Nota vitoria[] = {{523, 0.11f}, {659, 0.11f}, {784, 0.11f}, {1047, 0.28f}};
    const Nota derrota[] = {{392, 0.18f}, {330, 0.18f}, {262, 0.18f}, {196, 0.35f}};
    const Nota erro[] = {{160, 0.09f}};
    sons[SOM_CLIQUE] = montar(clique, 1, 0.25f);
    sons[SOM_CARTA] = montar(carta, 2, 0.25f);
    sons[SOM_CONSTRUIR] = montar(construir, 3, 0.4f);
    sons[SOM_GOLPE] = montar(golpe, 2, 0.35f);
    sons[SOM_VITORIA] = montar(vitoria, 4, 0.3f);
    sons[SOM_DERROTA] = montar(derrota, 4, 0.3f);
    sons[SOM_ERRO] = montar(erro, 1, 0.25f);
    pronto = true;
}

// Descarrega os sons e fecha o áudio.
void fecharSom() {
    if (pronto) {
        for (Sound& s : sons) UnloadSound(s);
    }
    if (IsAudioDeviceReady()) CloseAudioDevice();
    pronto = false;
}

// Toca um efeito (SOM_CLIQUE, SOM_GOLPE...), se o som estiver ligado.
void tocar(int som) {
    if (!pronto || !ligado || som < 0 || som >= TOTAL_SONS) return;
    PlaySound(sons[som]);
}

// Liga/desliga o som. Retorno: true se ficou ligado.
bool alternarSom() {
    ligado = !ligado;
    return ligado;
}
