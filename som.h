#ifndef SOM_H
#define SOM_H

// Efeitos sonoros. Não tem arquivo de áudio: cada som é uma onda
// (quadrada ou ruído) montada no código quando o jogo abre.

enum Som { SOM_CLIQUE, SOM_CARTA, SOM_CONSTRUIR, SOM_GOLPE, SOM_VITORIA, SOM_DERROTA, SOM_ERRO, TOTAL_SONS };

void iniciarSom();
void fecharSom();
void tocar(int som);

// liga/desliga (tecla M). devolve true se ficou ligado
bool alternarSom();

#endif
