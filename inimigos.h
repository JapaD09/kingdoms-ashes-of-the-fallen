#ifndef INIMIGOS_H
#define INIMIGOS_H

// Os 6 reinos que atacam Ashenfall. Cada um tem 3 tropas:
// 0 fraca, 1 média e 2 forte (a forte é o chefe nos turnos múltiplos de 6).

const int TOTAL_REINOS = 6;

struct TropaInimiga {
    const char* nome;
    int vida, ataque, defesa;
    bool distancia;
    int custo;          // quanto "gasta" do poder da onda
};

struct ReinoInimigo {
    const char* nome;
    const char* rei;
    const char* lema;
    TropaInimiga tropas[3];
    const char* placa;      // nome quebrado em 2 linhas pro painel
};

const ReinoInimigo& reinoInimigo(int id);

// poder da onda cresce com o turno
int poderDaOnda(int turno);
bool turnoDeChefe(int turno);

// embaralha a ordem dos 6 reinos (um por turno, todo ciclo muda)
void sortearOrdemDosReinos(int ordem[TOTAL_REINOS]);

#endif
