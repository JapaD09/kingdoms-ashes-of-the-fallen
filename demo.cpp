// Demonstração no terminal: mostra as estruturas e os algoritmos do jogo
// funcionando sem abrir a janela. Usa as mesmas funções do jogo de verdade.
// Compilar: make demo   (ou g++ -std=c++17 demo.cpp + os .cpp da lógica)

#include "jogo.h"
#include "reino.h"
#include "estruturas.h"
#include "algoritmos.h"
#include <chrono>
#include <iostream>
#include <string>

using namespace std;

// Escreve um título de seção pra separar as partes da demonstração.
static void titulo(const string& texto) {
    cout << "\n=== " << texto << " ===\n";
}

// Imprime a lista encadeada de edifícios do primeiro ao último nó.
static void mostrarLista(const ListaEdificios& lista) {
    cout << "  lista (" << lista.tamanho << "): ";
    for (const NoEdificio* no = lista.primeiro; no != nullptr; no = no->proximo) {
        cout << modeloEdificio(no->dados.tipo).nome << "(" << no->dados.linha << "," << no->dados.coluna << ")";
        if (no->proximo != nullptr) cout << " -> ";
    }
    cout << "\n";
}

// Procura um lote onde dá pra construir o tipo pedido.
// Retorno: true se achou; linha e coluna voltam pelas referências.
static bool loteLivre(const Reino& reino, int tipo, int& linha, int& coluna) {
    for (linha = 0; linha < TAMANHO_GRADE; linha++) {
        for (coluna = 0; coluna < TAMANHO_GRADE; coluna++) {
            if (validarConstrucao(reino, tipo, linha, coluna).empty()) return true;
        }
    }
    return false;
}

// Constrói o tipo pedido no primeiro lote livre e mostra o que o jogo respondeu.
static void construirNoPrimeiroLote(Reino& reino, int tipo) {
    int linha, coluna;
    if (!loteLivre(reino, tipo, linha, coluna)) {
        cout << "  sem lote pra " << modeloEdificio(tipo).nome << "\n";
        return;
    }
    string resposta = construirEdificio(reino, tipo, linha, coluna);
    cout << "  construir " << modeloEdificio(tipo).nome << ": " << (resposta.empty() ? "ok" : resposta) << "\n";
}

// Lista encadeada (R5) e pilha do desfazer (R6): constrói três prédios
// e desfaz o último, que sai da pilha e da lista.
static void demoListaEPilha(Reino& reino) {
    titulo("Lista encadeada + pilha (desfazer)");
    reino.estoque = {2000, 2000, 2000, 500, 200};
    construirNoPrimeiroLote(reino, FAZENDA);
    construirNoPrimeiroLote(reino, SERRARIA);
    construirNoPrimeiroLote(reino, MINA);
    mostrarLista(reino.edificios);
    cout << "  desfazer: " << desfazerUltimaAcao(reino) << "\n";
    mostrarLista(reino.edificios);
}

// Fila de construção (R7): sem recurso, as ordens esperam na fila
// e saem na ordem em que entraram quando o ouro chega.
static void demoFila(Reino& reino) {
    titulo("Fila de construcao");
    int linha, coluna;
    for (int tipo : {CASA, MERCADO}) {
        reino.estoque = {2000, 2000, 2000, 500, 200};      // só pra achar o lote
        if (!loteLivre(reino, tipo, linha, coluna)) continue;
        reino.estoque = {0, 0, 0, 500, 0};                 // agora sem ouro: vai pra fila
        cout << "  agendar " << modeloEdificio(tipo).nome << ": " << agendarConstrucao(reino, tipo, linha, coluna) << "\n";
    }
    reino.estoque = {2000, 2000, 2000, 500, 200};
    cout << "  chegou recurso -> " << processarFila(reino) << "\n";
    mostrarLista(reino.edificios);
}

// Imprime os itens do vetor (id e preço) numa linha só.
static void mostrarItens(const Item itens[], int total) {
    cout << "  ";
    for (int i = 0; i < total; i++) cout << itens[i].id << ":" << itens[i].valor << " ";
    cout << "\n";
}

// Árvore do catálogo (R8) e insertion sort por preço (R9).
// copiarEmOrdem já devolve ordenado por id (percurso em ordem).
static void demoArvoreEOrdenacao(const NoArvore* catalogo) {
    titulo("Arvore binaria de busca + insertion sort");
    Item itens[20];
    int total = 0;
    copiarEmOrdem(catalogo, itens, total, 20);
    cout << "  " << contarItens(catalogo) << " itens, altura " << alturaArvore(catalogo) << "\n";
    cout << "  em ordem (id:preco):\n";
    mostrarItens(itens, total);
    int comparacoes = 0;
    ordenarPorValor(itens, total, comparacoes);
    cout << "  por preco, " << comparacoes << " comparacoes:\n";
    mostrarItens(itens, total);
}

// Compara as três buscas (R10) procurando o mesmo id no catálogo.
static void demoBuscas(const NoArvore* catalogo, int id) {
    titulo("Buscas no catalogo (id " + to_string(id) + ")");
    Item itens[20];
    int total = 0, sequencial = 0, binaria = 0, arvore = 0;
    copiarEmOrdem(catalogo, itens, total, 20);
    buscaSequencial(itens, total, id, sequencial);
    buscaBinaria(itens, total, id, binaria);
    const Item* achou = buscarItem(catalogo, id, arvore);
    cout << "  item: " << (achou ? achou->nome : "nao existe") << "\n";
    cout << "  sequencial " << sequencial << " | binaria " << binaria << " | arvore " << arvore << " comparacoes\n";
}

// Mede o tempo de muitas buscas num vetor grande, porque com 20 itens
// tudo termina rápido demais pra medir. O que procura é sempre o pior caso
// da sequencial (o último id). Retorno: microssegundos gastos.
static long long medir(const Item itens[], int total, bool binaria, int repeticoes) {
    auto inicio = chrono::steady_clock::now();
    int comparacoes = 0;
    for (int r = 0; r < repeticoes; r++) {
        int id = itens[total - 1].id;
        if (binaria) buscaBinaria(itens, total, id, comparacoes);
        else buscaSequencial(itens, total, id, comparacoes);
    }
    auto fim = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::microseconds>(fim - inicio).count();
}

// Tempo das buscas com 10 mil itens (R10 com tempo, além das comparações).
// O vetor fica no heap com new[] e é apagado no fim.
static void demoTempo() {
    titulo("Tempo: 1000 buscas em 10000 itens");
    const int total = 10000;
    Item* itens = new Item[total];
    for (int i = 0; i < total; i++) itens[i].id = i * 2 + 1;     // já ordenado por id
    int compSeq = 0, compBin = 0;
    buscaSequencial(itens, total, itens[total - 1].id, compSeq);
    buscaBinaria(itens, total, itens[total - 1].id, compBin);
    cout << "  sequencial: " << compSeq << " comparacoes por busca, " << medir(itens, total, false, 1000) << " us\n";
    cout << "  binaria:    " << compBin << " comparacoes por busca, " << medir(itens, total, true, 1000) << " us\n";
    delete[] itens;
    itens = nullptr;
}

// Roda um turno completo: produção, ataque inimigo e batalha automática.
static void demoTurno(Jogo& jogo) {
    titulo("Um turno do jogo");
    cout << "  " << encerrarTurno(jogo) << "\n";
    cout << "  " << concluirBatalha(jogo) << "\n";
    cout << "  castelo com " << jogo.reino.vidaCastelo << " de vida, turno " << jogo.turno << "\n";
}

// Monta uma partida e chama cada parte da demonstração em ordem.
int main() {
    Jogo jogo;
    iniciarJogo(jogo);
    cout << "Kingdoms: Ashes of the Fallen - demonstracao no terminal\n";
    demoListaEPilha(jogo.reino);
    demoFila(jogo.reino);
    demoArvoreEOrdenacao(jogo.catalogo);
    demoBuscas(jogo.catalogo, 43);
    demoTempo();
    demoTurno(jogo);
    encerrarJogo(jogo);
    return 0;
}
