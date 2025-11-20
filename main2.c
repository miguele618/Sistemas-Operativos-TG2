#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#define MAX_CELLS 100
#define MAX_OBJECTS 10

// ==========================================
// ESTRUTURAS DE DADOS
// ==========================================

struct Player {
    char name[50];
    int energy;
    int cell;       // Onde está
    int object_id;  // -1 se não tem objeto
    int has_treasure; // -1 (não), 1 (sim)
};

struct Monster {
    int energy;
    int cell;
};

struct Cell {
    int north, south, west, east, up, down; // IDs das salas vizinhas (-1 se parede)
    int object_id;  // ID do objeto aqui (-1 se vazio)
    int treasure;   // 1 se tem tesouro, -1 se não
    char description[200];
};

struct Object {
    char name[50];
    int effectiveness;
};

// Estrutura Gigante para Memória Partilhada
// Isto permite que todos os processos vejam os mesmos dados
struct GameState {
    struct Player player;
    struct Monster monster;
    struct Cell map[MAX_CELLS];
    struct Object objects[MAX_OBJECTS];
    int nCells;
    int game_running; // 1 = corre, 0 = termina
    char last_message[200]; // Para comunicação entre processos (opcional)
};

// ==========================================
// FUNÇÕES AUXILIARES
// ==========================================

void InitializeMap(struct GameState *state) {
    // --- Inicialização Hardcoded para Teste (Substituir por leitura de ficheiro depois) ---
    state->nCells = 3;

    // Sala 0: Quintal
    state->map[0].north = -1; state->map[0].south = -1; 
    state->map[0].west = -1; state->map[0].east = 1;
    state->map[0].object_id = -1; state->map[0].treasure = -1;
    strcpy(state->map[0].description, "Esta no quintal de um castelo abandonado.");

    // Sala 1: Hall (Monstro começa aqui)
    state->map[1].north = -1; state->map[1].south = 2; 
    state->map[1].west = 0; state->map[1].east = -1;
    state->map[1].object_id = 0; // Tem uma espada aqui
    state->map[1].treasure = -1;
    strcpy(state->map[1].description, "Esta no Hall de Entrada. Ve uma armadura.");

    // Sala 2: Sala do Tesouro
    state->map[2].north = 1; state->map[2].south = -1; 
    state->map[2].west = -1; state->map[2].east = -1;
    state->map[2].object_id = -1; 
    state->map[2].treasure = 1; // TESOURO AQUI
    strcpy(state->map[2].description, "Esta numa sala brilhante e dourada!");

    // Objetos
    strcpy(state->objects[0].name, "Espada Enferrujada");
    state->objects[0].effectiveness = 20;
}

void InitializeGame(struct GameState *state, int argc, char *argv[]) {
    InitializeMap(state);
    state->game_running = 1;
    
    // Inicialização padrão
    strcpy(state->player.name, "Aventureiro");
    state->player.energy = 100;
    state->player.cell = 0; // Começa na sala 0
    state->player.object_id = -1;
    state->player.has_treasure = -1;

    state->monster.energy = 100;
    state->monster.cell = 2; // Começa na sala 2

    // MODO SUPER USER (Requisito 4 do PDF)
    // Exemplo: ./ja 1234 5000 10 5
    if (argc == 5 && strcmp(argv[1], "1234") == 0) {
        printf(">>> MODO SUPER USER ATIVADO <<<\n");
        state->player.energy = atoi(argv[2]);
        int start_cell = atoi(argv[3]);
        if(start_cell < state->nCells) state->player.cell = start_cell;
        state->player.object_id = atoi(argv[4]); // Assume que o objeto existe
    } else if (argc == 1) {
        // Modo normal, pede nome
        printf("Ola aventureiro! Como te chamas? ");
        char nome[50];
        if (scanf("%s", nome)) strcpy(state->player.name, nome);
    }
}

// ==========================================
// PROCESSOS
// ==========================================

// Processo do Monstro: Move-se aleatoriamente
void MonsterProcess(struct GameState *state) {
    srand(time(NULL) + 1); // Seed diferente
    
    while (state->game_running) {
        sleep(5); // Espera 5 segundos (requisito: tempo aleatório ou fixo)
        
        if (!state->game_running) break;

        int current = state->monster.cell;
        int direction = rand() % 4; // 0:N, 1:S, 2:E, 3:W
        int next_cell = -1;

        switch(direction) {
            case 0: next_cell = state->map[current].north; break;
            case 1: next_cell = state->map[current].south; break;
            case 2: next_cell = state->map[current].east; break;
            case 3: next_cell = state->map[current].west; break;
        }

        if (next_cell != -1) {
            state->monster.cell = next_cell;
            // O PDF diz para visualizar a posição do monstro em modo debug, 
            // mas para o jogador normal não deve aparecer, vou colocar print oculto
            // printf("[DEBUG] Monstro moveu-se para sala %d\n", next_cell);
        }
    }
    exit(0);
}

// Processo do Jogador: Lê comandos e move
void PlayerProcess(struct GameState *state) {
    char command[20];
    
    while (state->game_running) {
        struct Cell *current_room = &state->map[state->player.cell];
        
        printf("\n------------------------------------------------\n");
        printf("Local: %s (Sala %d)\n", current_room->description, state->player.cell);
        printf("Energia: %d | Objeto: %s | Tesouro: %s\n", 
                state->player.energy, 
                (state->player.object_id == -1) ? "Nenhum" : state->objects[state->player.object_id].name,
                (state->player.has_treasure == 1) ? "SIM" : "Nao");
        
        // Mostra saídas possíveis
        printf("Saidas: ");
        if(current_room->north != -1) printf("[norte] ");
        if(current_room->south != -1) printf("[sul] ");
        if(current_room->east != -1) printf("[este] ");
        if(current_room->west != -1) printf("[oeste] ");
        printf("\nComando (norte, sul, este, oeste, apanhar, sair): ");
        
        scanf("%s", command);

        if (!state->game_running) break;

        int next_cell = -1;

        if (strcmp(command, "norte") == 0) next_cell = current_room->north;
        else if (strcmp(command, "sul") == 0) next_cell = current_room->south;
        else if (strcmp(command, "este") == 0) next_cell = current_room->east;
        else if (strcmp(command, "oeste") == 0) next_cell = current_room->west;
        else if (strcmp(command, "sair") == 0) {
            state->game_running = 0;
            break;
        }
        else if (strcmp(command, "apanhar") == 0) {
            // Lógica de apanhar objeto ou tesouro
            if (current_room->treasure == 1) {
                state->player.has_treasure = 1;
                current_room->treasure = -1;
                printf(">>> APANHASTE O TESOURO! CORRE PARA A SAIDA (SALA 0)! <<<\n");
            } else if (current_room->object_id != -1) {
                state->player.object_id = current_room->object_id;
                current_room->object_id = -1; // Remove da sala
                printf(">>> Apanhaste: %s <<<\n", state->objects[state->player.object_id].name);
            } else {
                printf("Nao ha nada para apanhar aqui.\n");
            }
            continue; // Salta o movimento
        }

        if (next_cell != -1) {
            state->player.cell = next_cell;
        } else {
            printf(">> Nao podes ir por ai!\n");
        }
    }
    exit(0);
}

// ==========================================
// MAIN (Processo Árbitro)
// ==========================================
int main(int argc, char *argv[]) {
    int shmid;
    struct GameState *state;

    // 1. Criar Memória Partilhada
    // IPC_PRIVATE cria uma chave única, 0666 dá permissão de leitura/escrita
    shmid = shmget(IPC_PRIVATE, sizeof(struct GameState), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Erro no shmget");
        exit(1);
    }

    // 2. Anexar Memória (Attach)
    state = (struct GameState *) shmat(shmid, NULL, 0);
    if (state == (struct GameState *) -1) {
        perror("Erro no shmat");
        exit(1);
    }

    // 3. Inicializar Dados do Jogo
    InitializeGame(state, argc, argv);

    // 4. Criar Processos
    pid_t pid_monster, pid_player;

    pid_monster = fork();
    if (pid_monster == 0) {
        MonsterProcess(state); // Código do filho Monstro
        return 0;
    }

    pid_player = fork();
    if (pid_player == 0) {
        PlayerProcess(state); // Código do filho Jogador
        return 0;
    }

    // 5. Loop Principal (Árbitro)
    // Verifica colisões e estado do jogo
    while (state->game_running) {
        // Verificar Vitória
        if (state->player.has_treasure == 1 && state->player.cell == 0) {
            printf("\n\n***************************************\n");
            printf("PARABENS %s! FUGISTE COM O TESOURO!\n", state->player.name);
            printf("***************************************\n");
            state->game_running = 0;
        }

        // Verificar Combate
        if (state->player.cell == state->monster.cell && state->game_running) {
            printf("\n\n!!! ENCONTRASTE O MONSTRO !!!\n");
            printf("A lutar...\n");
            
            // Lógica simples de combate
            int player_atk = 10; // Base
            if (state->player.object_id != -1) {
                player_atk += state->objects[state->player.object_id].effectiveness;
            }

            state->monster.energy -= player_atk;
            state->player.energy -= 15; // Monstro ataca

            printf("A tua energia: %d | Energia Monstro: %d\n", state->player.energy, state->monster.energy);

            if (state->player.energy <= 0) {
                printf("\n>>> MORRESTE... GAME OVER <<<\n");
                state->game_running = 0;
            } else if (state->monster.energy <= 0) {
                printf("\n>>> MATASTE O MONSTRO! <<<\n");
                // Move o monstro para o "limbo" ou renasce noutro lado
                state->monster.cell = -1; 
            } else {
                // Se ninguém morreu, o monstro foge para outra sala (para não ficar loop infinito de luta)
                sleep(1);
                printf("O monstro fugiu para o escuro...\n");
                state->monster.cell = (state->monster.cell + 1) % state->nCells; 
            }
            sleep(2); // Pausa dramática
        }

        usleep(500000); // Verifica a cada 0.5 segundos
    }

    // 6. Limpeza
    // Enviar sinal para garantir que filhos terminam se ficarem presos
    kill(pid_player, SIGKILL);
    kill(pid_monster, SIGKILL);
    
    wait(NULL); // Espera filhos terminarem
    wait(NULL);

    shmdt(state); // Detach
    shmctl(shmid, IPC_RMID, NULL); // Remove Memória Partilhada

    printf("Jogo Terminado.\n");
    return 0;
}