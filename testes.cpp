// Testes do reino e da lista (sem janela)
// g++ -std=c++17 testes.cpp jogo.cpp reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp
//     expedicao.cpp estruturas.cpp algoritmos.cpp aleatorio.cpp -o testes

#include <iostream>
#include <string>
#include "reino.h"
#include "algoritmos.h"
#include "jogo.h"
#include "aleatorio.h"
#include "economia.h"

using namespace std;

static int falhas = 0;

static bool silenciosoOk = true;

// pra conferir dentro de laço sem imprimir mil linhas
static void conferir_silencioso(bool condicao) {
    silenciosoOk = silenciosoOk && condicao;
}

// Imprime ok ou FALHA com a descrição e conta as falhas.
static void conferir(bool condicao, const string& descricao) {
    cout << (condicao ? "  ok    " : "  FALHA ") << descricao << "\n";
    if (!condicao) falhas++;
}

// Retorno: uma fazenda com o id e o lote dados, pra montar listas de teste.
static Edificio edificio(int id, int linha, int coluna) {
    Edificio e;
    e.id = id;
    e.tipo = FAZENDA;
    e.linha = linha;
    e.coluna = coluna;
    return e;
}

// Testa a lista encadeada: inserir, buscar, remover do começo, meio e fim.
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

// Testa construir e demolir: custo exato, sem recurso, lote ocupado, castelo e floresta.
static void testarConstrucao() {
    cout << "Construcao\n";
    Reino reino;
    iniciarReino(reino);

    reino.estoque = modeloEdificio(FAZENDA).custo;   // custo exato da fazenda
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

// Testa a produção do turno: comida, madeira, imposto e o que acontece com fome.
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

// Testa a pilha (LIFO) e a fila (FIFO) isoladas.
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

// Testa o desfazer (pilha) e a fila de ordens de construção juntos.
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

// Árvore do catálogo: busca no vazio, id repetido, achar e não achar.
static void testarArvore(NoArvore*& raiz) {
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
}

// Insertion sort: ordena por preço, melhor caso com n-1 comparações e vetor vazio.
static void testarOrdenacao(Item v[], int n) {
    int comps = 0;
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
}

// In-ordem crescente e as duas buscas no vetor ordenado por id.
static void testarCatalogo() {
    cout << "Arvore, ordenacao e buscas\n";
    NoArvore* raiz = nullptr;
    testarArvore(raiz);

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
    testarOrdenacao(v, n);

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

// Testa a troca de mão e o preço de subir o nível dos prédios.
static void testarTrocaDeMao() {
    cout << "Troca de mao e preco do nivel\n";
    semearAleatorio(4);
    Reino reino;
    iniciarReino(reino);
    int cartas = totalDeCartas(reino.baralho);
    reino.estoque.ouro = 10;
    trocarMao(reino);
    conferir(reino.estoque.ouro == 10 && reino.trocasDeMao == 0, "sem ouro nao troca a mao");
    reino.estoque.ouro = 100;
    trocarMao(reino);
    conferir(reino.estoque.ouro == 85 && reino.baralho.naMao == 5, "troca custa 15 e vem mao cheia");
    conferir(totalDeCartas(reino.baralho) == cartas, "nenhuma carta some na troca");
    conferir(precoDaTroca(reino) == 25, "a segunda troca do turno e mais cara");
    processarTurnoReino(reino);
    conferir(precoDaTroca(reino) == 15, "no turno seguinte o preco volta");

    economia().nivelPct = 150;
    Recursos base = modeloEdificio(QUARTEL).custo;
    conferir(custoDeSubir(QUARTEL, 1).ouro == base.ouro * 250 / 100, "subir pro nivel 2 custa mais que construir");
    conferir(custoDeSubir(QUARTEL, 2).ouro > custoDeSubir(QUARTEL, 1).ouro, "nivel 3 custa mais que o 2");
    economia().nivelPct = 0;
    liberarReino(reino);
}

// Encantos: Lâminas no quartel, desfazer devolve a carta, e não vale em fazenda.
static void testarEncantos(Reino& reino, const Edificio* quartel) {
    reino.baralho.mao[reino.baralho.naMao++] = 12;   // Lâminas
    jogarCarta(reino, reino.baralho.naMao - 1, 1, 1);
    conferir(quartel->bonusAtaque == 2, "laminas dao +2 de ataque");
    desfazerUltimaAcao(reino);
    conferir(quartel->bonusAtaque == 0 && naMao(reino, 12) >= 0, "desfazer encanto devolve a carta");

    construirEdificio(reino, FAZENDA, 1, 2);
    reino.baralho.mao[0] = 12;
    string msg = jogarCarta(reino, 0, 1, 2);
    conferir(msg.find("tropa") != string::npos, "laminas nao vale em fazenda");
}

// Expansão só abre lote colado no reino, e desfazer fecha de novo.
static void testarExpansao(Reino& reino) {
    reino.baralho.mao[0] = 16;                       // Expansão
    jogarCarta(reino, 0, 0, 0);
    conferir(!loteLiberado(reino, 0, 0), "expansao longe do reino e recusada");
    jogarCarta(reino, 0, 0, 2);
    conferir(loteLiberado(reino, 0, 2), "expansao abre lote colado");
    desfazerUltimaAcao(reino);
    conferir(!loteLiberado(reino, 0, 2), "desfazer fecha o lote de novo");
}

// Baralho de 44 cartas, mão de 5, cartas de tropa e virada de turno sem perder carta.
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
    testarEncantos(reino, quartel);
    testarExpansao(reino);

    int antes = totalDeCartas(reino.baralho);
    descartarTudo(reino.baralho);
    comprarMao(reino.baralho);
    conferir(reino.baralho.naMao == 5 && totalDeCartas(reino.baralho) == antes, "turno novo: mao nova, nenhuma carta some");
    liberarReino(reino);
}

// Gacha: chances somam 100%, garantia de épico a cada 10 e equipe de até 3.
static void testarInvocacao() {
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
}

// Heróis: invocação, estrelas e o custo em cristais.
static void testarHerois() {
    cout << "Herois e invocacao\n";
    semearAleatorio(11);
    testarInvocacao();

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

// Sem defensor, os inimigos batem direto no castelo.
static void testarCasteloSemDefesa() {
    Batalha b;
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

// Dano mínimo, montagem do exército, vitória e recompensa.
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
    testarCasteloSemDefesa();
}

// Testa o ciclo de turnos: batalha todo turno, os 6 reinos atacando, derrota e reinício.
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

// mapa da expedição, saque e combate por ações
// 200 mapas sorteados, todos com caminho até o chefe; mapa vazio não tem caminho.
static void testarMapas() {
    semearAleatorio(21);
    bool sempreValido = true;
    for (int i = 0; i < 200; i++) {
        MapaExpedicao mapa;
        gerarMapa(mapa);
        sempreValido = sempreValido && mapaValido(mapa) && mapa.total <= MAX_NOS;
        liberarMapa(mapa);
    }
    conferir(sempreValido, "200 mapas: sempre existe caminho ate o chefe");

    MapaExpedicao vazio;
    bool visitados[MAX_NOS] = {};
    conferir(!existeCaminho(vazio.entrada, vazio.chefe, visitados) && !mapaValido(vazio), "mapa vazio nao tem caminho");
    liberarMapa(vazio);
}

// Partir: precisa de herói, gasta comida e só vale uma vez por turno.
static void testarPartida(Jogo& jogo) {
    conferir(!partirEmExpedicao(jogo).empty(), "sem heroi nao sai em expedicao");
    jogo.reino.herois.estrelas[0] = jogo.reino.herois.estrelas[1] = 5;
    jogo.reino.herois.naEquipe[0] = jogo.reino.herois.naEquipe[1] = true;
    int comida = jogo.reino.estoque.comida;
    conferir(partirEmExpedicao(jogo).empty() && jogo.expedicao.ativa, "com herois a expedicao comeca");
    conferir(jogo.reino.estoque.comida == comida - CUSTO_EXPEDICAO, "expedicao gasta comida");
    conferir(!partirEmExpedicao(jogo).empty(), "so uma expedicao por turno");
}

// Combate por ações dentro da expedição e a volta pra casa com o saque.
static void testarCombateEVolta(Jogo& jogo) {
    Expedicao& exp = jogo.expedicao;
    int ouro = jogo.reino.estoque.ouro;
    conferir(!avancar(exp, 9).empty(), "caminho que nao existe e recusado");
    exp.saque.ouro = 40;
    conferir(jogo.reino.estoque.ouro == ouro, "saque ainda nao e do reino");

    exp.atual->caminhos[0]->tipo = NO_COMBATE;          // força um combate pra testar as ações
    avancar(exp, 0);
    conferir(exp.fase == EXP_COMBATE && inimigosVivos(exp) > 0, "no de combate comeca a luta");
    int vida = exp.grupo[0].vida + exp.grupo[1].vida;
    agirNoCombate(exp, 99, 0);
    conferir(exp.rodada == 0, "acao invalida nao gasta a rodada");
    exp.pocoes = 0;
    agirNoCombate(exp, USAR_POCAO, 0);
    conferir(exp.rodada == 0, "sem pocao nao gasta a rodada");
    agirNoCombate(exp, DEFENDER, 0);
    int perdeuDefendendo = vida - exp.grupo[0].vida - exp.grupo[1].vida;
    conferir(exp.rodada == 1 && perdeuDefendendo >= 0, "defender passa a rodada");
    while (exp.fase == EXP_COMBATE) agirNoCombate(exp, ATACAR, 0);
    conferir(exp.fase == EXP_AVISO && exp.saque.ouro > 40, "vencer o combate da saque");
    continuarExpedicao(exp);
    voltarAoReino(exp);
    conferir(exp.fase == EXP_FIM && exp.desfecho == VOLTOU, "da pra voltar pelo mapa");
    int saque = exp.saque.ouro;
    concluirExpedicao(jogo);
    conferir(jogo.reino.estoque.ouro >= ouro + saque && !exp.ativa, "voltando, o saque entra no reino");
    conferir(exp.mapa.todos == nullptr, "mapa liberado no fim");
}

// Derrota na expedição: perde o saque e os heróis ficam feridos.
static void testarDerrotaNaExpedicao(Jogo& jogo) {
    Expedicao& exp = jogo.expedicao;
    jogo.reino.explorouNoTurno = false;
    partirEmExpedicao(jogo);
    exp.saque.ouro = 500;
    int ouro = jogo.reino.estoque.ouro;
    exp.atual->caminhos[0]->tipo = NO_COMBATE;
    avancar(exp, 0);
    for (int i = 0; i < exp.tamanho; i++) exp.grupo[i].vida = 1;
    for (int i = 0; i < exp.totalInimigos; i++) exp.inimigos[i].ataque = 50;
    agirNoCombate(exp, DEFENDER, 0);
    conferir(exp.desfecho == DERROTADA, "grupo sem vida perde a expedicao");
    concluirExpedicao(jogo);
    conferir(jogo.reino.estoque.ouro == ouro && jogo.reino.feridos == 2, "derrota: saque perdido e herois feridos");
}

// Carta de recompensa depois da defesa e vitória da campanha com os 6 chefes.
static void testarRecompensaEVitoria(Jogo& jogo) {
    Expedicao& exp = jogo.expedicao;
    jogo.batalha.vitoria = true;
    jogo.emBatalha = true;
    jogo.batalha.acabou = true;
    int cartasAntes = totalDeCartas(jogo.reino.baralho);
    concluirBatalha(jogo);
    conferir(jogo.escolhendoCarta, "vencer a defesa oferece 3 cartas");
    conferir(jogo.recompensa[0] != jogo.recompensa[1] && jogo.recompensa[1] != jogo.recompensa[2], "as cartas oferecidas sao diferentes");
    escolherRecompensa(jogo, 1);
    conferir(totalDeCartas(jogo.reino.baralho) == cartasAntes + 1 && !jogo.escolhendoCarta, "a carta escolhida entra no baralho");

    // vitória da campanha
    for (int r = 0; r < TOTAL_REINOS - 1; r++) jogo.chefeDerrotado[r] = true;
    conferir(proximaRegiao(jogo) == TOTAL_REINOS - 1 && !jogo.vitoria, "falta so o ultimo reino");
    jogo.reino.feridos = 0;
    jogo.reino.explorouNoTurno = false;
    jogo.reino.estoque.comida = 100;
    partirEmExpedicao(jogo);
    exp.desfecho = CHEFE_VENCIDO;
    concluirExpedicao(jogo);
    conferir(jogo.vitoria && chefesDerrotados(jogo) == TOTAL_REINOS, "6 chefes derrotados = vitoria");
    conferir(pontuacao(jogo) >= 3000, "pontuacao conta os chefes");
}

// Expedição inteira: mapas, partida, combate, derrota e vitória da campanha.
static void testarExpedicao() {
    cout << "Expedicao\n";
    testarMapas();
    Jogo jogo;
    iniciarJogo(jogo, 4);
    testarPartida(jogo);
    testarCombateEVolta(jogo);
    testarDerrotaNaExpedicao(jogo);
    testarRecompensaEVitoria(jogo);
    encerrarJogo(jogo);
}

// Os testes usam os preços de tabela, sem o ajuste de balanceamento,
// pra que os números conferidos não mudem quando a economia for recalibrada.
static void usarEconomiaDeTabela() {
    Economia deTeste;
    deTeste.custoPct = 100;
    deTeste.ondaPorTurno = 3;
    deTeste.ondaDivisor = 10;
    deTeste.materiaisPct = 100;
    deTeste.producaoPct = 100;
    deTeste.forcaBase = 0;
    deTeste.rendaMadeira = 0;
    deTeste.saquePct = 100;
    deTeste.vendaPct = 100;
    deTeste.ouroPorInimigo = 3;
    deTeste.nivelPct = 0;
    economia() = deTeste;
    aplicarEconomia();
}

// Roda todos os testes. Retorno: 0 se passou tudo, 1 se algum falhou.
int main() {
    usarEconomiaDeTabela();
    testarCatalogo();
    testarPilhaEFila();
    testarDesfazerEOrdens();
    testarLista();
    testarConstrucao();
    testarTurno();
    testarCartas();
    testarTrocaDeMao();
    silenciosoOk = true;
    testarHerois();
    testarBatalha();
    silenciosoOk = true;
    testarCicloDoJogo();
    testarExpedicao();
    cout << (falhas == 0 ? "\nTudo certo!\n" : "\nTem falha!\n");
    return falhas == 0 ? 0 : 1;
}
