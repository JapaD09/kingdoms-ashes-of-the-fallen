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
void desenharMina(int x, int y, int s);
void desenharMercado(int x, int y, int s);
void desenharCasa(int x, int y, int s);
void desenharTorre(int x, int y, int s);
void desenharArquearia(int x, int y, int s);
void desenharEstabulo(int x, int y, int s);
void desenharTorreArcana(int x, int y, int s);

// desenho de cada TipoEdificio
Desenho desenhoDoEdificio(int tipo);

// arte das cartas de encanto e especiais (22x22)
void artLaminas(int x, int y, int s);
void artArmaduras(int x, int y, int s);
void artEstandarte(int x, int y, int s);
void artBencao(int x, int y, int s);
void artExpansao(int x, int y, int s);
void artCaravana(int x, int y, int s);

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
void iconeDesfazer(int x, int y, int s);
void iconeVista(int x, int y, int s);
void iconeCoracao(int x, int y, int s);
void iconeCristal(int x, int y, int s);
void iconeEspadas(int x, int y, int s);
void iconeEstrela(int x, int y, int s);

#endif
