#include "inimigos.h"
#include "aleatorio.h"

namespace {

// nome, vida, ataque, defesa, de longe, custo
const ReinoInimigo REINOS[TOTAL_REINOS] = {
    {"Clã Goblin", "Grizzak, o Ladrão", "Tudo que brilha é nosso!",
        {{"Goblin", 5, 2, 0, false, 1}, {"Goblin fundeiro", 4, 3, 0, true, 2}, {"Troll do pântano", 26, 5, 1, false, 7}}, "Clã\nGoblin"},
    {"Horda Orc", "Murg Quebra-Ossos", "Sangue pro Vale Rubro!",
        {{"Orc", 12, 4, 1, false, 3}, {"Orc lanceiro", 10, 5, 1, false, 3}, {"Chefe orc", 30, 6, 2, false, 8}}, "Horda\nOrc"},
    {"Necrópole", "Vessa, a Sem-Fim", "A morte não descansa.",
        {{"Esqueleto", 6, 3, 0, false, 2}, {"Arqueiro ossudo", 5, 3, 0, true, 2}, {"Cavaleiro da morte", 24, 6, 3, false, 8}}, "Necró-\npole"},
    {"Império das Areias", "Imperatriz Saresh", "O deserto engole reinos.",
        {{"Lanceiro de bronze", 9, 3, 2, false, 2}, {"Arqueiro do sol", 6, 4, 0, true, 2}, {"Guarda de bronze", 22, 5, 4, false, 7}}, "Império\ndas Areias"},
    {"Elfos Prateados", "Lorde Aelith", "A floresta cobra o que é dela.",
        {{"Batedor élfico", 7, 3, 1, false, 2}, {"Arqueiro élfico", 7, 5, 0, true, 3}, {"Druida antigo", 20, 7, 1, true, 8}}, "Elfos\nPrateados"},
    {"Senhores das Cinzas", "Pyrax, o Dragão", "Tudo vira cinza.",
        {{"Cultista", 7, 3, 0, false, 2}, {"Mago de fogo", 6, 5, 0, true, 3}, {"Draco das cinzas", 40, 7, 2, false, 10}}, "Senhores\ndas Cinzas"},
};

}

const ReinoInimigo& reinoInimigo(int id) {
    return REINOS[id];
}

int poderDaOnda(int turno) {
    return 3 + turno * 3 + turno * turno / 10;
}

bool turnoDeChefe(int turno) {
    return turno % 6 == 0;
}

void sortearOrdemDosReinos(int ordem[TOTAL_REINOS]) {
    for (int i = 0; i < TOTAL_REINOS; i++) ordem[i] = i;
    for (int i = TOTAL_REINOS - 1; i > 0; i--) {
        int j = sortear(0, i);
        int troca = ordem[i];
        ordem[i] = ordem[j];
        ordem[j] = troca;
    }
}
