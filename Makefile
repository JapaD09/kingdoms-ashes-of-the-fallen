CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC = main.cpp jogo.cpp interface.cpp natureza.cpp fonte.cpp sprites.cpp reino.cpp expedicao.cpp combate.cpp estruturas.cpp algoritmos.cpp

kingdoms: $(SRC) *.h
	$(CXX) $(CXXFLAGS) $(SRC) -o kingdoms $(LIBS)

clean:
	rm -f kingdoms kingdoms.exe
