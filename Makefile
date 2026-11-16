CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# só a lógica, sem raylib (usada pelos testes e pela demo do terminal)
LOGICA = jogo.cpp reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp expedicao.cpp estruturas.cpp algoritmos.cpp aleatorio.cpp

TELA = main.cpp interface.cpp ui.cpp tela_batalha.cpp tela_herois.cpp tela_expedicao.cpp figuras.cpp pincel.cpp natureza.cpp visao3d.cpp modelos3d.cpp fonte.cpp sprites.cpp som.cpp

kingdoms: $(TELA) $(LOGICA) *.h
	$(CXX) $(CXXFLAGS) $(TELA) $(LOGICA) -o kingdoms $(LIBS)

testes: testes.cpp $(LOGICA) *.h
	$(CXX) $(CXXFLAGS) testes.cpp $(LOGICA) -o testes

demo: demo.cpp $(LOGICA) *.h
	$(CXX) $(CXXFLAGS) -O2 demo.cpp $(LOGICA) -o demo

clean:
	rm -f kingdoms kingdoms.exe testes demo
