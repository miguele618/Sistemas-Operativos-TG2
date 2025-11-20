#include "jogo.h"

// Variável global para ponteiro da memória partilhada (para facilitar cleanup)
EstadoJogo *jogo = NULL;
int shmid = 0;

// --- Funções Auxiliares ---

void carregar_objetos() {
    // Hardcoded para simplificar, mas poderia vir de ficheiro
    strcpy(jogo->objetos[0].nome, "Espada Enferrujada");
    jogo->objetos[0].eficacia = 10;
    
    strcpy(jogo->objetos[1].nome, "Machado de Guerra");
    jogo->objetos[1].eficacia = 25;
    
    jogo->num_objetos = 2;
}

void carregar_mapa(const char *ficheiro) {
    FILE *f = fopen(ficheiro, "r");
    if (!f) { perror("Erro ao abrir mapa.txt"); exit(1); }

    int i = 0;
    char buffer[MAX_DESC];
    
    while (!feof(f) && i < MAX_SALAS) {
        // Ler direções, objeto e tesouro
        int n = fscanf(f, "%d %d %d %d %d %d %d %d\n", 
               &jogo->salas[i].direcoes[0], &jogo->salas[i].direcoes[1],
               &jogo->salas[i].direcoes[2], &jogo->salas[i].direcoes[3],
               &jogo->salas[i].direcoes[4], &jogo->salas[i].direcoes[5],
               &jogo->salas[i].id_objeto, &jogo->salas[i].tem_tesouro);
        
        if (n != 8) break; // Fim ou erro

        // Ler descrição
        if (fgets(buffer, MAX_DESC, f)) {
            // Remover newline no fim
            buffer[strcspn(buffer, "\n")] = 0;
            strcpy(jogo->salas[i].descricao, buffer);
        }
        
        // Consumir linha vazia separadora
        fgets(buffer, MAX_DESC, f);
        
        i++;
    }
    jogo->num_salas = i;
    fclose(f);
    printf(">> Mapa carregado com %d salas.\n", i);
}

void gravar_jogo() {
    FILE *f = fopen("savegame.dat", "wb");
    if (f) {
        fwrite(jogo, sizeof(EstadoJogo), 1, f);
        fclose(f);
        printf(">> Jogo gravado com sucesso!\n");
    } else {
        printf(">> Erro ao gravar jogo.\n");
    }
}

void carregar_save() {
    FILE *f = fopen("savegame.dat", "rb");
    if (f) {
        // Preservar o endereço do semáforo é complicado ao carregar binário bruto
        // Vamos carregar dados críticos apenas
        EstadoJogo temp;
        fread(&temp, sizeof(EstadoJogo), 1, f);
        
        // Copiar dados relevantes (evitando sobrescrever o semáforo mutex)
        memcpy(jogo->salas, temp.salas, sizeof(temp.salas));
        jogo->jogador = temp.jogador;
        jogo->monstro = temp.monstro;
        printf(">> Jogo carregado!\n");
        fclose(f);
    } else {
        printf(">> Savegame nao encontrado.\n");
    }
}

// --- Lógica dos Processos ---

void processo_monstro() {
    srand(time(NULL) ^ getpid()); // Seed aleatória
    
    while (jogo->jogo_a_correr) {
        int tempo_espera = (rand() % 10) + 5; // Espera 5 a 15 segundos
        sleep(tempo_espera);

        sem_wait(&jogo->mutex); // Bloqueia acesso
        
        if (!jogo->em_combate && jogo->jogo_a_correr) {
            // Tentar mover
            int direcao = rand() % 6;
            int prox_sala = jogo->salas[jogo->monstro.local].direcoes[direcao];
            
            if (prox_sala != -1) {
                jogo->monstro.local = prox_sala;
                // Opcional: Escrever mensagem para o jogador ver
                // sprintf(jogo->msg_sistema, "O monstro moveu-se para algures...");
            }
        }
        
        sem_post(&jogo->mutex); // Liberta acesso
    }
    exit(0);
}

void processo_jogador() {
    char cmd[50];
    
    while (jogo->jogo_a_correr) {
        // Mostrar estado
        sem_wait(&jogo->mutex);
        if (jogo->em_combate) {
            printf("\n!!! EM COMBATE !!! (Aguarde resolucao...)\n");
            sem_post(&jogo->mutex);
            sleep(1);
            continue;
        }
        
        Sala *s = &jogo->salas[jogo->jogador.local];
        printf("\n------------------------------------------------\n");
        printf("Local: %s (Sala %d)\n", s->descricao, jogo->jogador.local);
        printf("Energia: %d | Inventario: %s\n", jogo->jogador.energia, 
               (jogo->jogador.id_objeto_mao == -1) ? "Nada" : jogo->objetos[jogo->jogador.id_objeto_mao].nome);
        
        if (s->id_objeto != -1) 
            printf("Ves aqui: %s\n", jogo->objetos[s->id_objeto].nome);
        if (s->tem_tesouro == 1)
            printf("BRILHA ALGO DOURADO AQUI! (TESOURO)\n");
            
        // Mostrar posição do monstro (Requisito Pág 7)
        printf("[DEBUG] Monstro esta na sala: %d\n", jogo->monstro.local);
        
        sem_post(&jogo->mutex);

        // Ler comando
        printf("Comando (norte, sul, este, oeste, apanhar, tesouro, gravar, carregar, sair): ");
        scanf("%s", cmd);

        sem_wait(&jogo->mutex);
        
        int nova_sala = -1;
        int dir = -1;
        
        if (strcmp(cmd, "norte") == 0) dir = 0;
        else if (strcmp(cmd, "sul") == 0) dir = 1;
        else if (strcmp(cmd, "oeste") == 0) dir = 2;
        else if (strcmp(cmd, "este") == 0) dir = 3;
        else if (strcmp(cmd, "cima") == 0) dir = 4;
        else if (strcmp(cmd, "baixo") == 0) dir = 5;
        else if (strcmp(cmd, "apanhar") == 0) {
            if (s->id_objeto != -1) {
                jogo->jogador.id_objeto_mao = s->id_objeto;
                s->id_objeto = -1; // Remove da sala
                printf("Apanhaste o objeto!\n");
            } else {
                printf("Nao ha nada para apanhar.\n");
            }
        }
        else if (strcmp(cmd, "tesouro") == 0) {
            if (s->tem_tesouro == 1) {
                jogo->jogador.tem_tesouro = 1;
                s->tem_tesouro = 0;
                printf("PARABENS! APANHASTE O TESOURO!\n");
            }
        }
        else if (strcmp(cmd, "gravar") == 0) gravar_jogo();
        else if (strcmp(cmd, "carregar") == 0) carregar_save();
        else if (strcmp(cmd, "sair") == 0) jogo->jogo_a_correr = 0;

        // Processar movimento
        if (dir != -1) {
            nova_sala = s->direcoes[dir];
            if (nova_sala != -1) {
                jogo->jogador.local = nova_sala;
            } else {
                printf("Nao podes ir por ai.\n");
            }
        }
        
        sem_post(&jogo->mutex);
    }
    exit(0);
}

void resolver_combate() {
    printf("\n!!! ENCONTRO COM O MONSTRO NA SALA %d !!!\n", jogo->jogador.local);
    
    int dano_jogador = 5; // Dano base (soco)
    if (jogo->jogador.id_objeto_mao != -1) {
        dano_jogador += jogo->objetos[jogo->jogador.id_objeto_mao].eficacia;
        printf("Usas %s para atacar!\n", jogo->objetos[jogo->jogador.id_objeto_mao].nome);
    }
    
    int dano_monstro = (rand() % 15) + 5;
    
    // Troca de golpes
    jogo->monstro.energia -= dano_jogador;
    jogo->jogador.energia -= dano_monstro;
    
    printf("Deste %d de dano. Monstro tem %d HP.\n", dano_jogador, jogo->monstro.energia);
    printf("Recebeste %d de dano. Tens %d HP.\n", dano_monstro, jogo->jogador.energia);
    
    if (jogo->monstro.energia <= 0) {
        printf("VENCESTE O MONSTRO!\n");
        // Teleportar monstro para longe ou "matar" (resetar energia e mover)
        jogo->monstro.local = (jogo->monstro.local + 1) % jogo->num_salas; 
        jogo->monstro.energia = 100;
    }
    
    if (jogo->jogador.energia <= 0) {
        printf("MORRESTE...\n");
        jogo->jogo_a_correr = 0;
    }
    
    sleep(2); // Pausa dramática
}

// --- Main ---

int main(int argc, char *argv[]) {
    // 1. Configurar Memória Partilhada
    shmid = shmget(SHM_KEY, sizeof(EstadoJogo), IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }
    
    jogo = (EstadoJogo *)shmat(shmid, NULL, 0);
    if (jogo == (void *)-1) { perror("shmat"); exit(1); }

    // Inicializar Semáforo (partilhado entre processos, valor inicial 1)
    sem_init(&jogo->mutex, 1, 1);

    // 2. Inicialização de Dados
    jogo->jogo_a_correr = 1;
    jogo->em_combate = 0;
    carregar_objetos();
    carregar_mapa("mapa.txt");

    // Configuração Inicial (Default)
    jogo->jogador.energia = 100;
    jogo->jogador.local = 0;
    jogo->jogador.id_objeto_mao = -1;
    jogo->jogador.tem_tesouro = 0;
    
    jogo->monstro.energia = 100;
    jogo->monstro.local = 2; // Começa numa sala diferente

    // Modo Super User (Requisito Pág 6)
    // ./ja 1234 5000 10 5
    if (argc == 5 && strcmp(argv[1], "1234") == 0) {
        printf("--- MODO SUPER USER ---\n");
        jogo->jogador.energia = atoi(argv[2]);
        int sala_ini = atoi(argv[3]);
        if (sala_ini < jogo->num_salas) jogo->jogador.local = sala_ini;
        jogo->jogador.id_objeto_mao = atoi(argv[4]);
    }

    // 3. Criar Processos
    pid_t pid_monstro = fork();
    if (pid_monstro == 0) {
        processo_monstro(); // Código do filho (Monstro)
    }

    pid_t pid_jogador = fork();
    if (pid_jogador == 0) {
        processo_jogador(); // Código do filho (Jogador)
    }

    // 4. Processo Pai (Motor de Jogo / Árbitro)
    while (jogo->jogo_a_correr) {
        sem_wait(&jogo->mutex);
        
        // Verificar Vitória
        if (jogo->jogador.tem_tesouro == 1 && jogo->jogador.local == 0) {
            printf("\n\n*** PARABENS! SAIU DA CAVERNA COM O TESOURO! ***\n");
            jogo->jogo_a_correr = 0;
            sem_post(&jogo->mutex);
            break;
        }

        // Verificar Colisão (Combate)
        if (jogo->jogador.local == jogo->monstro.local && jogo->monstro.energia > 0) {
            jogo->em_combate = 1;
            resolver_combate();
            jogo->em_combate = 0;
        }
        
        sem_post(&jogo->mutex);
        usleep(500000); // Verifica a cada 0.5 segundos
    }

    // 5. Limpeza
    printf("A encerrar jogo...\n");
    kill(pid_monstro, SIGKILL);
    kill(pid_jogador, SIGKILL);
    wait(NULL);
    wait(NULL);
    
    sem_destroy(&jogo->mutex);
    shmdt(jogo);
    shmctl(shmid, IPC_RMID, NULL); // Marcar memória para destruição
    
    return 0;
}