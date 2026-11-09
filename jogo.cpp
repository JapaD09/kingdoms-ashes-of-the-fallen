#include "jogo.h"
#include <iostream>
#include <limits>

using namespace std;

static void mostrarTitulo() {
    cout << "\n";
    cout << "  ==============================================\n";
    cout << "       |>                              <|\n";
    cout << "      /^^\\    K I N G D O M S        /^^\\\n";
    cout << "      |[]|                            |[]|\n";
    cout << "      |  |   ~ Ashes of the Fallen ~  |  |\n";
    cout << "  ==============================================\n";
}

static void mostrarCabecalho(const Jogo& jogo) {
    mostrarTitulo();
    cout << "  Reino: " << jogo.nomeReino
         << "\t\tTurno: " << jogo.turno << "\n";
    cout << "  ----------------------------------------------\n";
    mostrarRecursos(jogo.reino);
    cout << "  ----------------------------------------------\n";
}

static void mostrarMenuPrincipal() {
    cout << "  [1] Gerenciar edificios\n";
    cout << "  [2] Recrutar soldados\n";
    cout << "  [3] Consultar recursos\n";
    cout << "  [4] Preparar expedicao\n";
    cout << "  [5] Consultar catalogo\n";
    cout << "  [6] Encerrar turno\n";
    cout << "  [0] Sair\n";
}

int lerOpcao(int minimo, int maximo) {
    int opcao;
    while (true) {
        cout << "  Escolha: ";
        if (cin >> opcao && opcao >= minimo && opcao <= maximo) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return opcao;
        }
        if (cin.eof()) {
            // ctrl+d / fim da entrada: sai em vez de ficar em loop eterno
            return 0;
        }
        cout << "  Opcao invalida, digite um numero de "
             << minimo << " a " << maximo << ".\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void pausar() {
    cout << "\n  (Enter para continuar)";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void iniciarJogo(Jogo& jogo) {
    jogo.turno = 1;
    jogo.rodando = true;
    iniciarReino(jogo.reino);
}

// Por enquanto o turno so conta. Producao e consumo entram na etapa 3.
static void encerrarTurno(Jogo& jogo) {
    jogo.turno++;
    cout << "\n  O sol se pos sobre " << jogo.nomeReino
         << ". Comeca o turno " << jogo.turno << ".\n";
}

static void avisoEmBreve() {
    cout << "\n  Ainda nao implementado.\n";
}

void loopPrincipal(Jogo& jogo) {
    while (jogo.rodando) {
        mostrarCabecalho(jogo);
        mostrarMenuPrincipal();

        int opcao = lerOpcao(0, 6);
        switch (opcao) {
            case 3:
                cout << "\n";
                mostrarRecursos(jogo.reino);
                pausar();
                break;
            case 6:
                encerrarTurno(jogo);
                pausar();
                break;
            case 0:
                jogo.rodando = false;
                break;
            default:
                avisoEmBreve();
                pausar();
                break;
        }
    }
    cout << "\n  Ate a proxima, governante.\n";
}
