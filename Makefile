CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

SRC = main.cpp jogo.cpp reino.cpp expedicao.cpp combate.cpp estruturas.cpp algoritmos.cpp

kingdoms: $(SRC) *.h
	$(CXX) $(CXXFLAGS) $(SRC) -o kingdoms

clean:
	rm -f kingdoms kingdoms.exe
