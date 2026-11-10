#ifndef SPRITES_H
#define SPRITES_H

// Desenhos em pixel art. s = tamanho do pixel.

typedef void (*Desenho)(int x, int y, int s);

// construções (22x22)
void desenharCastelo(int x, int y, int s);
void desenharFazenda(int x, int y, int s);
void desenharSerraria(int x, int y, int s);
void desenharQuartel(int x, int y, int s);
void desenharMuralha(int x, int y, int s);
void desenharOficina(int x, int y, int s);

// ícones (7x7)
void iconeOuro(int x, int y, int s);
void iconeMadeira(int x, int y, int s);
void iconePedra(int x, int y, int s);
void iconeComida(int x, int y, int s);
void iconeFerro(int x, int y, int s);
void iconePovo(int x, int y, int s);
void iconeSoldado(int x, int y, int s);
void iconeCoroa(int x, int y, int s);
void iconeMapa(int x, int y, int s);
void iconeLivro(int x, int y, int s);
void iconeBau(int x, int y, int s);
void iconePorta(int x, int y, int s);
void iconeAmpulheta(int x, int y, int s);

#endif
