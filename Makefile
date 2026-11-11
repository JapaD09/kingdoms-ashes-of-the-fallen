CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC = main.cpp jogo.cpp interface.cpp ui.cpp tela_batalha.cpp tela_herois.cpp figuras.cpp pincel.cpp natureza.cpp visao3d.cpp fonte.cpp sprites.cpp reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp expedicao.cpp estruturas.cpp algoritmos.cpp aleatorio.cpp

kingdoms: $(SRC) *.h
	$(CXX) $(CXXFLAGS) $(SRC) -o kingdoms $(LIBS)

clean:
	rm -f kingdoms kingdoms.exe
