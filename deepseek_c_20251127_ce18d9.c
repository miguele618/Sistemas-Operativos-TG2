#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// Estruturas (mantidas do código anterior)
typedef struct {
    char nome[50];
    int energia;
    int local;
    int objeto;
    int tesouro;
} Jogador;

typedef struct {
    int norte, sul, este, oeste, cima, baixo;
    char descricao[200];
    int objeto;
    int tesouro;
} Sala;

typedef struct {
    char nome[50];
    int eficacia;
} Objeto;

typedef struct {
    int energia;
    int local;
} Monstro;

// Variáveis globais
Jogador jogador;
Sala *mapa;
Objeto *objetos;
Monstro monstro;
int nSalas = 0;
int nObjetos = 0;

// Novas funções para interface RPG
void mostrarTitulo() {
    printf("\n=========================================\n");
    printf("    O ÚLTIMO FAROL DE ALFAMA\n");
    printf("=========================================\n");
}

void mostrarStatus() {
    printf("\n=== STATUS ===\n");
    printf("Saúde: %d/100\n", jogador.energia);
    printf("Local: %d\n", jogador.local);
    if (jogador.objeto != -1) {
        printf("Arma: %s\n", objetos[jogador.objeto].nome);
    } else {
        printf("Arma: Mãos vazias\n");
    }
    printf("Tesouro: %s\n", jogador.tesouro == 1 ? "Barril das Almas" : "Não encontrado");
}

void mostrarComandos() {
    printf("\n=== COMANDOS ===\n");
    printf("[N] Norte  [S] Sul  [E] Este  [O] Oeste\n");
    printf("[C] Subir  [B] Descer  [I] Inventário\n");
    printf("[M] Mapa local  [Q] Sair do jogo\n");
    printf("[A] Atacar  [F] Fugir (quando em combate)\n");
}

void mostrarMapaLocal(int local) {
    printf("\n--- MAPA LOCAL ---\n");
    printf("Estás na sala: %d\n", local);
    printf("Saídas disponíveis:\n");
    if (mapa[local].norte != -1) printf("  Norte → Sala %d\n", mapa[local].norte);
    if (mapa[local].sul != -1) printf("  Sul → Sala %d\n", mapa[local].sul);
    if (mapa[local].este != -1) printf("  Este → Sala %d\n", mapa[local].este);
    if (mapa[local].oeste != -1) printf("  Oeste → Sala %d\n", mapa[local].oeste);
    if (mapa[local].cima != -1) printf("  Escadas acima → Sala %d\n", mapa[local].cima);
    if (mapa[local].baixo != -1) printf("  Escadas abaixo → Sala %d\n", mapa[local].baixo);
}

void combateRPG() {
    printf("\n⚔️  ENCONTRASTE A SOMBRA SÓNICA! ⚔️\n");
    
    while (jogador.energia > 0 && monstro.energia > 0) {
        printf("\n--- COMBATE ---\n");
        printf("Tua saúde: %d | Sombra Sónica: %d\n", jogador.energia, monstro.energia);
        printf("[A] Atacar  [F] Fugir\n");
        printf("Escolhe: ");
        
        char acao;
        scanf(" %c", &acao);
        
        if (acao == 'A' || acao == 'a') {
            int dano = (jogador.objeto != -1) ? objetos[jogador.objeto].eficacia : 5;
            monstro.energia -= dano;
            printf("Atacaste com %s! Causaste %d de dano!\n", 
                   (jogador.objeto != -1) ? objetos[jogador.objeto].nome : "punhos", dano);
            
            if (monstro.energia > 0) {
                int danoMonstro = 8 + (rand() % 5);
                jogador.energia -= danoMonstro;
                printf("A Sombra Sónica contra-ataca! Perdeste %d de saúde!\n", danoMonstro);
            }
        } else if (acao == 'F' || acao == 'f') {
            printf("Fugiste do combate! A Sombra fica mais forte...\n");
            monstro.energia += 10;
            break;
        } else {
            printf("Comando inválido! Perdes a vez...\n");
        }
    }
    
    if (monstro.energia <= 0) {
        printf("\n🎉 DERROTASTE A SOMBRA SÓNICA! 🎉\n");
        printf("Encontraste o Barril das Almas!\n");
        jogador.tesouro = 1;
        monstro.local = -1;
    }
}

// Funções originais (adaptadas)
void inicializarJogador() {
    strcpy(jogador.nome, "D. Afonso");
    jogador.energia = 100;
    jogador.local = 0;
    jogador.objeto = -1;
    jogador.tesouro = -1;
}

void inicializarMapa() {
    nSalas = 6;
    mapa = malloc(nSalas * sizeof(Sala));

    // Sala 0 - Tanoaria da Mouraria
    mapa[0].norte = 1;
    mapa[0].sul = -1;
    mapa[0].este = -1;
    mapa[0].oeste = -1;
    mapa[0].cima = -1;
    mapa[0].baixo = -1;
    mapa[0].objeto = 0;
    mapa[0].tesouro = -1;
    strcpy(mapa[0].descricao, "Tanoaria da tua família. Cheira a madeira e vinho. À norte, a Rua da Mouraria.");

    // Sala 1 - Rua da Mouraria
    mapa[1].norte = 2;
    mapa[1].sul = 0;
    mapa[1].este = -1;
    mapa[1].oeste = -1;
    mapa[1].cima = -1;
    mapa[1].baixo = -1;
    mapa[1].objeto = -1;
    mapa[1].tesouro = -1;
    strcpy(mapa[1].descricao, "Rua estreita da Mouraria. Crianças brincam ao longe. A norte, o Largo do Farol.");

    // Sala 2 - Largo do Farol
    mapa[2].norte = -1;
    mapa[2].sul = 1;
    mapa[2].este = 3;
    mapa[2].oeste = 4;
    mapa[2].cima = -1;
    mapa[2].baixo = 5;
    mapa[2].objeto = 1;
    mapa[2].tesouro = -1;
    strcpy(mapa[2].descricao, "Largo abandonado. Uma escadaria de pedra desce para as galerias subterrâneas.");

    // Sala 3 - Beco do Eco
    mapa[3].norte = -1;
    mapa[3].sul = -1;
    mapa[3].este = -1;
    mapa[3].oeste = 2;
    mapa[3].cima = -1;
    mapa[3].baixo = -1;
    mapa[3].objeto = -1;
    mapa[3].tesouro = -1;
    strcpy(mapa[3].descricao, "Beco sem saída. Os sons ecoam de forma estranha... Algo não está bem aqui.");

    // Sala 4 - Travessa das Sombras
    mapa[4].norte = -1;
    mapa[4].sul = -1;
    mapa[4].este = 2;
    mapa[4].oeste = -1;
    mapa[4].cima = -1;
    mapa[4].baixo = -1;
    mapa[4].objeto = -1;
    mapa[4].tesouro = -1;
    strcpy(mapa[4].descricao, "Travessa escura. Sentes arrepios na espinha. Algo te observa das sombras...");

    // Sala 5 - Galerias Subterrâneas
    mapa[5].norte = -1;
    mapa[5].sul = -1;
    mapa[5].este = -1;
    mapa[5].oeste = -1;
    mapa[5].cima = 2;
    mapa[5].baixo = -1;
    mapa[5].objeto = -1;
    mapa[5].tesouro = 1;
    strcpy(mapa[5].descricao, "Galerias antigas sob Alfama. O ar é pesado e o silêncio é quase absoluto.");
}

void inicializarObjetos() {
    nObjetos = 2;
    objetos = malloc(nObjetos * sizeof(Objeto));
    
    strcpy(objetos[0].nome, "Chave de Tanoeiro");
    objetos[0].eficacia = 15;
    
    strcpy(objetos[1].nome, "Lamparina de Azeite");
    objetos[1].eficacia = 10;
}

void inicializarMonstro() {
    monstro.energia = 60;
    monstro.local = 4; // Começa na Travessa das Sombras
}

void descreverLocal(int local) {
    printf("\n--- %s ---\n", (local == 5) ? "GALERIAS SUBTERRÂNEAS" : "ALFAMA");
    printf("%s\n", mapa[local].descricao);
    
    if (mapa[local].objeto != -1) {
        printf("🔦 Vês um objeto: %s\n", objetos[mapa[local].objeto].nome);
        printf("Queres apanhá-lo? [S/N]: ");
        char resposta;
        scanf(" %c", &resposta);
        if (resposta == 'S' || resposta == 's') {
            jogador.objeto = mapa[local].objeto;
            mapa[local].objeto = -1;
            printf("Apanhaste %s!\n", objetos[jogador.objeto].nome);
        }
    }
}

void moverJogador(char comando) {
    int destino = -1;
    
    switch (comando) {
        case 'N': case 'n': destino = mapa[jogador.local].norte; break;
        case 'S': case 's': destino = mapa[jogador.local].sul; break;
        case 'E': case 'e': destino = mapa[jogador.local].este; break;
        case 'O': case 'o': destino = mapa[jogador.local].oeste; break;
        case 'C': case 'c': destino = mapa[jogador.local].cima; break;
        case 'B': case 'b': destino = mapa[jogador.local].baixo; break;
        default: return; // Comando tratado no main
    }

    if (destino == -1) {
        printf("\n❌ Não podes ir nessa direção!\n");
    } else {
        jogador.local = destino;
        printf("\n🏃 Moveste-te para uma nova área...\n");
    }
}

void moverMonstro() {
    // Movimento mais inteligente - persegue o jogador
    if (monstro.local == -1) return;
    
    int direcoes[6] = {
        mapa[monstro.local].norte,
        mapa[monstro.local].sul,
        mapa[monstro.local].este,
        mapa[monstro.local].oeste,
        mapa[monstro.local].cima,
        mapa[monstro.local].baixo
    };
    
    // Tenta mover-se para mais perto do jogador
    for (int i = 0; i < 6; i++) {
        if (direcoes[i] != -1 && abs(direcoes[i] - jogador.local) < abs(monstro.local - jogador.local)) {
            monstro.local = direcoes[i];
            return;
        }
    }
    
    // Movimento aleatório se não conseguir aproximar
    int tentativas = 0;
    while (tentativas < 10) {
        int dir = rand() % 6;
        if (direcoes[dir] != -1) {
            monstro.local = direcoes[dir];
            break;
        }
        tentativas++;
    }
}

int main() {
    srand(time(NULL));
    mostrarTitulo();
    
    printf("\n📖 HISTÓRIA:\n");
    printf("O teu irmão Miguel desapareceu nas galerias de Alfama.\n");
    printf("Encontra o Barril das Almas antes que a Sombra Sónica consuma a cidade!\n\n");
    
    inicializarJogador();
    inicializarMapa();
    inicializarObjetos();
    inicializarMonstro();
    
    printf("Pressiona Enter para começar...");
    getchar();
    
    char comando;
    int fimDeJogo = 0;
    
    while (!fimDeJogo && jogador.energia > 0) {
        system("clear"); // Limpa o ecrã (Linux/Mac)
        
        mostrarTitulo();
        mostrarStatus();
        
        // Movimento do monstro
        moverMonstro();
        
        // Descrição do local
        descreverLocal(jogador.local);
        
        // Verificar combate
        if (jogador.local == monstro.local && monstro.energia > 0) {
            combateRPG();
        }
        
        // Verificar vitória
        if (jogador.tesouro == 1 && jogador.local == 0) {
            printf("\n🎉🎉🎉 MISSÃO CUMPRIDA! 🎉🎉🎉\n");
            printf("Regressaste à tanoaria com o Barril das Almas!\n");
            printf("Alfama está salva... por agora.\n");
            break;
        }
        
        // Comandos do jogador
        mostrarComandos();
        printf("\nComando: ");
        scanf(" %c", &comando);
        
        switch (comando) {
            case 'I': case 'i': 
                printf("\n--- INVENTÁRIO ---\n");
                if (jogador.objeto != -1) 
                    printf("Arma: %s (+%d dano)\n", objetos[jogador.objeto].nome, objetos[jogador.objeto].eficacia);
                else
                    printf("Arma: Nenhuma\n");
                printf("Tesouro: %s\n", jogador.tesouro == 1 ? "Barril das Almas ✓" : "Não encontrado");
                printf("Pressiona Enter para continuar...");
                getchar(); getchar();
                break;
                
            case 'M': case 'm':
                mostrarMapaLocal(jogador.local);
                printf("Pressiona Enter para continuar...");
                getchar(); getchar();
                break;
                
            case 'Q': case 'q':
                printf("A sair do jogo...\n");
                fimDeJogo = 1;
                break;
                
            default:
                moverJogador(comando);
                break;
        }
        
        // Perda de energia por turno
        jogador.energia -= 1;
    }
    
    if (jogador.energia <= 0) {
        printf("\n💀 PERDESTE! 💀\n");
        printf("A Sombra Sónica consumiu-te... Alfama está perdida.\n");
    }
    
    free(mapa);
    free(objetos);
    
    return 0;
}