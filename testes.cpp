// Testes do reino e da lista (sem janela)
// g++ -std=c++17 testes.cpp jogo.cpp reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp
//     estruturas.cpp algoritmos.cpp aleatorio.cpp -o testes

#include <iostream>
#include <string>
#include "reino.h"
#include "algoritmos.h"
#include "jogo.h"
#include "aleatorio.h"

using namespace std;

static int falhas = 0;

static bool silenciosoOk = true;

// pra conferir dentro de laço sem imprimir mil linhas
static void conferir_silencioso(bool condicao) {
    silenciosoOk = silenciosoOk && condicao;
}

static void conferir(bool condicao, const string& descricao) {
    cout << (condicao ? "  ok    " : "  FALHA ") << descricao << "\n";
    if (!condicao) falhas++;
}

static Edificio edificio(int id, int linha, int coluna) {
    Edificio e;
    e.id = id;
    e.tipo = FAZENDA;
    e.linha = linha;
    e.coluna = coluna;
    return e;
}

static void testarLista() {
    cout << "Lista encadeada\n";
    ListaEdificios lista;
    conferir(!removerEdificio(lista, 1), "remover de lista vazia devolve false");
    conferir(buscarEdificioNoLote(lista, 1, 1) == nullptr, "buscar em lista vazia");

    inserirEdificio(lista, edificio(1, 1, 1));
    inserirEdificio(lista, edificio(2, 1, 2));
    inserirEdificio(lista, edificio(3, 1, 3));
    conferir(lista.tamanho == 3, "tres insercoes");
    conferir(lista.primeiro->dados.id == 1 && lista.ultimo->dados.id == 3, "mantem ordem de construcao");

    conferir(removerEdificio(lista, 1), "remove o primeiro");
    conferir(lista.primeiro->dados.id == 2, "segundo virou o primeiro");
    conferir(removerEdificio(lista, 3), "remove o ultimo");
    conferir(lista.ultimo->dados.id == 2, "ponteiro ultimo foi atualizado");
    conferir(!removerEdificio(lista, 99), "id inexistente devolve false");

    inserirEdificio(lista, edificio(4, 3, 3));
    conferir(lista.ultimo->dados.id == 4, "inserir depois de remover o ultimo");
    liberarEdificios(lista);
    conferir(lista.primeiro == nullptr && lista.tamanho == 0, "liberar zera a lista");
}

static void testarConstrucao() {
    cout << "Construcao\n";
    Reino reino;
    iniciarReino(reino);

    reino.estoque = {30, 20, 0, 0, 0};   // custo exato da fazenda
    construirEdificio(reino, FAZENDA, 1, 1);
    conferir(reino.edificios.tamanho == 1, "constroi com recursos exatos");
    conferir(reino.estoque.ouro == 0 && reino.estoque.madeira == 0, "cobra uma vez so");

    construirEdificio(reino, FAZENDA, 1, 2);
    conferir(reino.edificios.tamanho == 1, "nao constroi sem recursos");

    reino.estoque = {500, 500, 500, 500, 500};
    construirEdificio(reino, SERRARIA, 1, 1);
    conferir(reino.edificios.tamanho == 1, "nao constroi em lote ocupado");
    construirEdificio(reino, SERRARIA, 2, 2);
    conferir(reino.edificios.tamanho == 1, "nao constroi no castelo");
    construirEdificio(reino, SERRARIA, 0, 0);
    conferir(reino.edificios.tamanho == 1, "nao constroi na floresta");

    demolirEdificio(reino, 1, 1);
    conferir(reino.edificios.tamanho == 0 && reino.estoque.ouro == 515, "demolir devolve metade do ouro");
    liberarReino(reino);
}

static void testarTurno() {
    cout << "Turno\n";
    Reino reino;
    iniciarReino(reino);
    reino.estoque.ouro = 200;
    construirEdificio(reino, FAZENDA, 1, 1);
    construirEdificio(reino, SERRARIA, 1, 2);

    Recursos antes = reino.estoque;
    processarTurnoReino(reino);
    // +8 da fazenda - 6 de consumo = +2
    conferir(reino.estoque.comida == antes.comida + 2, "producao e consumo de comida");
    conferir(reino.estoque.madeira == antes.madeira + 6, "serraria produz madeira");
    conferir(reino.estoque.ouro == antes.ouro + 10, "imposto de 1 ouro por 2 habitantes");

    reino.estoque.comida = 0;
    demolirEdificio(reino, 1, 1);
    string msg = processarTurnoReino(reino);
    conferir(reino.estoque.comida == 0, "comida nao fica negativa");
    conferir(reino.populacao < 20, "fome faz gente ir embora");
    liberarReino(reino);
}

static void testarPilhaEFila() {
    cout << "Pilha e fila\n";
    PilhaAcoes pilha;
    Acao a;
    conferir(!desempilhar(pilha, a), "pilha vazia nao desempilha");
    for (int i = 1; i <= 3; i++) {
        a.edificio.id = i;
        empilhar(pilha, a);
    }
    conferir(topoDaPilha(pilha)->edificio.id == 3, "topo e o ultimo que entrou");
    desempilhar(pilha, a);
    conferir(a.edificio.id == 3 && pilha.tamanho == 2, "LIFO: sai o 3");
    limparPilha(pilha);
    conferir(pilhaVazia(pilha), "limpar esvazia a pilha");

    FilaConstrucao fila;
    OrdemConstrucao o;
    conferir(!desenfileirar(fila, o), "fila vazia nao desenfileira");
    for (int i = 0; i < 3; i++) {
        o.tipoEdificio = i;
        enfileirar(fila, o);
    }
    desenfileirar(fila, o);
    conferir(o.tipoEdificio == 0 && fila.tamanho == 2, "FIFO: sai o primeiro");
    desenfileirar(fila, o);
    desenfileirar(fila, o);
    conferir(filaVazia(fila) && fila.fim == nullptr, "fila esvaziou certinho");
}

static void testarDesfazerEOrdens() {
    cout << "Desfazer e ordens\n";
    Reino reino;
    iniciarReino(reino);
    Recursos antes = reino.estoque;
    construirEdificio(reino, FAZENDA, 1, 1);
    desfazerUltimaAcao(reino);
    conferir(reino.edificios.tamanho == 0 && reino.estoque.ouro == antes.ouro, "desfazer construcao devolve tudo");

    construirEdificio(reino, FAZENDA, 1, 1);
    demolirEdificio(reino, 1, 1);
    desfazerUltimaAcao(reino);
    conferir(reino.edificios.tamanho == 1, "desfazer demolicao traz de volta");

    processarTurnoReino(reino);
    conferir(pilhaVazia(reino.historico), "virou turno, pilha limpa");

    reino.estoque = {0, 0, 0, 100, 0};
    agendarConstrucao(reino, SERRARIA, 1, 2);
    agendarConstrucao(reino, FAZENDA, 1, 3);
    conferir(reino.ordens.tamanho == 2, "duas ordens na fila");
    processarFila(reino);
    conferir(reino.ordens.tamanho == 2, "sem recurso a fila espera");
    reino.estoque = {80, 40, 0, 100, 0};
    processarFila(reino);
    conferir(reino.ordens.tamanho == 0 && reino.edificios.tamanho == 3, "com recurso constroi na ordem");
    liberarReino(reino);
}

static void testarCatalogo() {
    cout << "Arvore, ordenacao e buscas\n";
    NoArvore* raiz = nullptr;
    int comps = 0;
    conferir(buscarItem(raiz, 5, comps) == nullptr, "buscar em arvore vazia");
    montarCatalogo(raiz);
    conferir(contarItens(raiz) == 14, "14 itens no catalogo");
    Item repetido;
    repetido.id = 50;
    conferir(!inserirItem(raiz, repetido), "id repetido e recusado");
    comps = 0;
    const Item* achou = buscarItem(raiz, 43, comps);
    conferir(achou && achou->nome == "Cota de malha", "acha id existente");
    comps = 0;
    conferir(buscarItem(raiz, 44, comps) == nullptr, "nao acha id inexistente");

    Item v[20];
    int n = 0;
    copiarEmOrdem(raiz, v, n, 20);
    bool crescente = true;
    for (int i = 1; i < n; i++) crescente = crescente && v[i - 1].id < v[i].id;
    conferir(n == 14 && crescente, "in-ordem sai crescente por id");

    int compSeq, compBin;
    conferir(buscaSequencial(v, n, 93, compSeq) == 13 && compSeq == 14, "sequencial olha todos ate o ultimo");
    conferir(buscaBinaria(v, n, 93, compBin) == 13 && compBin <= 4, "binaria acha em ate log2(n)+1");
    conferir(buscaBinaria(v, n, 94, compBin) == -1, "binaria nao acha inexistente");
    conferir(buscaSequencial(v, n, 1, compSeq) == -1, "sequencial nao acha inexistente");

    ordenarPorValor(v, n, comps);
    bool porPreco = true;
    for (int i = 1; i < n; i++) porPreco = porPreco && v[i - 1].valor <= v[i].valor;
    conferir(porPreco && v[0].nome == "Pão de viagem", "insertion sort por preco");
    int compOrdenado;
    ordenarPorValor(v, n, compOrdenado);
    conferir(compOrdenado == n - 1, "ja ordenado: n-1 comparacoes (melhor caso)");
    Item vazio[1];
    ordenarPorValor(vazio, 0, comps);
    conferir(comps == 0, "vetor vazio nao quebra");

    liberarArvore(raiz);
    conferir(raiz == nullptr, "liberar zera a raiz");
}

// acha a carta na mão (-1 se não veio)
static int naMao(const Reino& reino, int carta) {
    for (int i = 0; i < reino.baralho.naMao; i++) {
        if (reino.baralho.mao[i] == carta) return i;
    }
    return -1;
}

// força uma carta na mão pra testar
static int darCarta(Reino& reino, int carta) {
    reino.baralho.mao[0] = carta;
    return 0;
}

static void testarCartas() {
    cout << "Cartas e baralho\n";
    semearAleatorio(3);
    Reino reino;
    iniciarReino(reino);
    conferir(reino.baralho.naMao == 5, "comeca com 5 cartas na mao");
    conferir(totalDeCartas(reino.baralho) == 44, "baralho tem 44 cartas");

    reino.estoque = {500, 500, 500, 500, 500};
    int i = darCarta(reino, 8);                   // Quartel
    jogarCarta(reino, i, 1, 1);
    const Edificio* quartel = buscarEdificioNoLote(reino.edificios, 1, 1);
    conferir(quartel && quartel->tipo == QUARTEL && reino.baralho.naMao == 4, "carta de tropa ergue o quartel");

    reino.baralho.mao[reino.baralho.naMao++] = 8;
    jogarCarta(reino, reino.baralho.naMao - 1, 1, 1);
    conferir(quartel->nivel == 2, "mesma carta em cima sobe o nivel");

    reino.baralho.mao[reino.baralho.naMao++] = 12;   // Lâminas
    jogarCarta(reino, reino.baralho.naMao - 1, 1, 1);
    conferir(quartel->bonusAtaque == 2, "laminas dao +2 de ataque");
    desfazerUltimaAcao(reino);
    conferir(quartel->bonusAtaque == 0 && naMao(reino, 12) >= 0, "desfazer encanto devolve a carta");

    construirEdificio(reino, FAZENDA, 1, 2);
    reino.baralho.mao[0] = 12;
    string msg = jogarCarta(reino, 0, 1, 2);
    conferir(msg.find("tropa") != string::npos, "laminas nao vale em fazenda");

    reino.baralho.mao[0] = 16;                       // Expansão
    msg = jogarCarta(reino, 0, 0, 0);
    conferir(!loteLiberado(reino, 0, 0), "expansao longe do reino e recusada");
    jogarCarta(reino, 0, 0, 2);
    conferir(loteLiberado(reino, 0, 2), "expansao abre lote colado");
    desfazerUltimaAcao(reino);
    conferir(!loteLiberado(reino, 0, 2), "desfazer fecha o lote de novo");

    int antes = totalDeCartas(reino.baralho);
    descartarTudo(reino.baralho);
    comprarMao(reino.baralho);
    conferir(reino.baralho.naMao == 5 && totalDeCartas(reino.baralho) == antes, "turno novo: mao nova, nenhuma carta some");
    liberarReino(reino);
}

static void testarHerois() {
    cout << "Herois e invocacao\n";
    semearAleatorio(11);
    int soma = 0;
    for (int r = COMUM; r <= LENDARIO; r++) soma += chanceDaRaridade(r);
    conferir(soma == 100, "chances somam 100%");

    ColecaoHerois colecao;
    int epicos = 0;
    for (int i = 0; i < 200; i++) {
        int antes = colecao.semEpico;
        int id = invocarHeroi(colecao);
        if (modeloHeroi(id).raridade >= EPICO) epicos++;
        conferir_silencioso(antes < GARANTIA_EPICO);
    }
    conferir(silenciosoOk, "nunca passa de 10 sem epico (garantia)");
    conferir(epicos >= 200 / GARANTIA_EPICO, "pelo menos 1 epico a cada 10");
    conferir(tamanhoDaEquipe(colecao) == MAX_EQUIPE, "equipe enche ate 3");

    ColecaoHerois nova;
    conferir(string(alternarEquipe(nova, 0)).size() > 0, "nao escala heroi que nao tem");
    nova.estrelas[0] = 3;
    conferir(vidaDoHeroi(nova, 0) == 34 + 34 * 2 / 4, "estrelas aumentam a vida");

    Reino reino;
    iniciarReino(reino);
    int heroi;
    reino.cristais = 4;
    invocar(reino, heroi);
    conferir(heroi == -1 && reino.cristais == 4, "sem cristal nao invoca");
    reino.cristais = 5;
    invocar(reino, heroi);
    conferir(heroi >= 0 && reino.cristais == 0, "invocar gasta 5 cristais");
    liberarReino(reino);
}

static void testarBatalha() {
    cout << "Batalha\n";
    conferir(calcularDano(5, 2) == 3, "dano = ataque - defesa");
    conferir(calcularDano(2, 9) == 1, "dano minimo e 1");

    semearAleatorio(5);
    Reino reino;
    iniciarReino(reino);
    reino.estoque = {900, 900, 900, 900, 900};
    construirEdificio(reino, QUARTEL, 1, 1);
    construirEdificio(reino, ARQUEARIA, 1, 2);
    construirEdificio(reino, TORRE, 1, 3);

    Batalha b;
    montarBatalha(b, reino, 0, 1);
    conferir(vivosDoLado(b, ALIADO) == 3 + 2 + 6, "3 soldados + 2 arqueiros + 6 guardas");
    conferir(vivosDoLado(b, INIMIGO) > 0 && b.tirosDeTorre == 1, "goblins chegaram e a torre atira");
    while (!b.acabou) rodadaDeBatalha(b);
    conferir(b.vitoria, "reino armado vence a primeira onda");
    int ouroAntes = reino.estoque.ouro;
    aplicarResultado(b, reino);
    conferir(reino.estoque.ouro > ouroAntes && reino.cristais > 10, "vitoria da ouro e cristais");
    liberarReino(reino);

    Reino fraco;
    iniciarReino(fraco);
    fraco.soldados = 0;
    montarBatalha(b, fraco, 5, 12);
    rodadaDeBatalha(b);
    conferir(b.acabou && !b.vitoria && b.danoNoCastelo > 0, "sem defensor o castelo apanha");
    aplicarResultado(b, fraco);
    conferir(fraco.vidaCastelo == VIDA_CASTELO - b.danoNoCastelo, "dano sai da vida do castelo");
    liberarReino(fraco);
}

static void testarCicloDoJogo() {
    cout << "Turnos e reinos inimigos\n";
    Jogo jogo;
    iniciarJogo(jogo, 9);
    bool todos[TOTAL_REINOS] = {};
    for (int t = 0; t < TOTAL_REINOS; t++) {
        todos[reinoDaVez(jogo)] = true;
        jogo.reino.vidaCastelo = VIDA_CASTELO;      // pra não perder no meio do teste
        encerrarTurno(jogo);
        conferir_silencioso(jogo.emBatalha);
        concluirBatalha(jogo);
    }
    bool viuTodos = true;
    for (bool v : todos) viuTodos = viuTodos && v;
    conferir(silenciosoOk, "todo fim de turno tem batalha");
    conferir(viuTodos && jogo.turno == 7, "os 6 reinos atacam em 6 turnos");

    jogo.reino.vidaCastelo = 1;
    jogo.reino.soldados = 0;
    liberarEdificios(jogo.reino.edificios);
    encerrarTurno(jogo);
    concluirBatalha(jogo);
    conferir(jogo.derrota, "castelo sem vida = derrota");
    iniciarJogo(jogo, 9);
    conferir(!jogo.derrota && jogo.turno == 1 && jogo.reino.edificios.tamanho == 0, "reiniciar limpa a partida");
    encerrarJogo(jogo);
}

int main() {
    testarCatalogo();
    testarPilhaEFila();
    testarDesfazerEOrdens();
    testarLista();
    testarConstrucao();
    testarTurno();
    testarCartas();
    silenciosoOk = true;
    testarHerois();
    testarBatalha();
    silenciosoOk = true;
    testarCicloDoJogo();
    cout << (falhas == 0 ? "\nTudo certo!\n" : "\nTem falha!\n");
    return falhas == 0 ? 0 : 1;
}
