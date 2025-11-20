#include "jogo.h"

// Variáveis globais
EstadoJogo *jogo = NULL;
int shmid = 0;
pid_t pid_monstro = 0, pid_jogador = 0;

// --- Funções de Limpeza e Sinais ---

void limpar_recursos() {
    if (jogo != NULL) {
        sem_destroy(&jogo->mutex);
        shmdt(jogo);
    }
    if (shmid > 0) {
        shmctl(shmid, IPC_RMID, NULL);
        printf("\n[SISTEMA] Memoria partilhada limpa.\n");
    }
}

void handle_sigint(int sig) {
    printf("\n\n[SISTEMA] A encerrar forcadamente...\n");
    if (pid_monstro > 0) kill(pid_monstro, SIGKILL);
    if (pid_jogador > 0) kill(pid_jogador, SIGKILL);
    limpar_recursos();
    exit(0);
}

// --- Funções de Carregamento ---

void carregar_objetos() {
    strcpy(jogo->objetos[0].nome, "Espada de Aco");
    jogo->objetos[0].eficacia = 20;
    
    strcpy(jogo->objetos[1].nome, "Machado Duplo");
    jogo->objetos[1].eficacia = 35;
    
    jogo->num_objetos = 2;
}

void carregar_mapa(const char *ficheiro) {
    FILE *f = fopen(ficheiro, "r");
    if (!f) { 
        printf("ERRO: Nao foi possivel abrir %s.\n", ficheiro);
        exit(1); 
    }

    int i = 0;
    char buffer[MAX_DESC];
    
    while (!feof(f) && i < MAX_SALAS) {
        int n = fscanf(f, "%d %d %d %d %d %d %d %d\n", 
               &jogo->salas[i].direcoes[0], &jogo->salas[i].direcoes[1],
               &jogo->salas[i].direcoes[2], &jogo->salas[i].direcoes[3],
               &jogo->salas[i].direcoes[4], &jogo->salas[i].direcoes[5],
               &jogo->salas[i].id_objeto, &jogo->salas[i].tem_tesouro);
        
        if (n != 8) break;

        if (fgets(buffer, MAX_DESC, f)) {
            buffer[strcspn(buffer, "\n")] = 0;
            strcpy(jogo->salas[i].descricao, buffer);
        }
        fgets(buffer, MAX_DESC, f); 
        i++;
    }
    jogo->num_salas = i;
    fclose(f);
}

void gravar_jogo() {
    FILE *f = fopen("savegame.dat", "wb");
    if (f) {
        fwrite(jogo, sizeof(EstadoJogo), 1, f);
        fclose(f);
        strcpy(jogo->ultima_mensagem, ">> Jogo gravado com sucesso!");
    } else {
        strcpy(jogo->ultima_mensagem, ">> Erro ao gravar jogo.");
    }
}

void carregar_save() {
    FILE *f = fopen("savegame.dat", "rb");
    if (f) {
        EstadoJogo temp;
        fread(&temp, sizeof(EstadoJogo), 1, f);
        memcpy(jogo->salas, temp.salas, sizeof(temp.salas));
        jogo->jogador = temp.jogador;
        jogo->monstro = temp.monstro;
        strcpy(jogo->ultima_mensagem, ">> Jogo carregado!");
        fclose(f);
    } else {
        strcpy(jogo->ultima_mensagem, ">> Savegame nao encontrado.");
    }
}

// --- Processo: MONSTRO ---

void processo_monstro() {
    srand(time(NULL) ^ getpid());
    
    while (jogo->jogo_a_correr) {
        int tempo = (rand() % 10) + 5; 
        sleep(tempo); 

        sem_wait(&jogo->mutex);
        if (!jogo->em_combate && jogo->jogo_a_correr && jogo->monstro.energia > 0) {
            int dir = rand() % 6;
            int prox = jogo->salas[jogo->monstro.local].direcoes[dir];
            if (prox != -1) {
                jogo->monstro.local = prox;
            }
        }
        sem_post(&jogo->mutex);
    }
    exit(0);
}

// --- Processo: JOGADOR (Interface) ---

void desenhar_interface() {
    // REMOVIDO: system("clear"); 
    // Adicionado quebras de linha para separar visualmente os turnos
    printf("\n\n"); 
    printf("==================================================\n");
    
    // MODO COMBATE
    if (jogo->em_combate) {
        printf("       /!\\ RELATORIO DE COMBATE /!\\   \n");
        printf("--------------------------------------------------\n");
        printf("%s", jogo->log_combate);
        printf("--------------------------------------------------\n");
        return;
    }

    // MODO EXPLORAÇÃO
    Sala *s = &jogo->salas[jogo->jogador.local];
    printf("LOCAL: %s (Sala %d)\n", s->descricao, jogo->jogador.local);
    
    if (jogo->monstro.energia > 0)
        printf("[INFO] Monstro detetado na sala: %d (HP: %d)\n", jogo->monstro.local, jogo->monstro.energia);
    else
        printf("[INFO] O Monstro foi derrotado.\n");

    printf("--------------------------------------------------\n");
    
    if (s->id_objeto != -1) 
        printf("CHAO: Ves um(a) [%s] aqui.\n", jogo->objetos[s->id_objeto].nome);
    
    if (s->tem_tesouro == 1)
        printf("\n*** O TESOURO ESTA AQUI! ***\n");

    printf("--------------------------------------------------\n");
    printf("JOGADOR: Energia: %d/100 | MAO: %s\n", 
           jogo->jogador.energia, 
           (jogo->jogador.id_objeto_mao == -1) ? "Vazia" : jogo->objetos[jogo->jogador.id_objeto_mao].nome);
    printf("--------------------------------------------------\n");
    printf("MENSAGEM: %s\n", jogo->ultima_mensagem);
    printf("==================================================\n");
    printf("COMANDOS: norte, sul, este, oeste, apanhar, tesouro, gravar, carregar, sair\n");
    printf(">> ");
}

void processo_jogador() {
    char cmd[50];
    
    while (jogo->jogo_a_correr) {
        sem_wait(&jogo->mutex);
        
        if (jogo->em_combate) {
            desenhar_interface();
            sem_post(&jogo->mutex);
            sleep(2); // Espera um pouco para não spammar o log de combate muito rápido
            continue;
        }
        
        desenhar_interface();
        sem_post(&jogo->mutex);

        // Input do utilizador
        scanf("%s", cmd);

        sem_wait(&jogo->mutex);
        strcpy(jogo->ultima_mensagem, "..."); 

        int nova_sala = -1;
        int dir = -1;
        Sala *s = &jogo->salas[jogo->jogador.local];

        if (strcmp(cmd, "norte") == 0) dir = 0;
        else if (strcmp(cmd, "sul") == 0) dir = 1;
        else if (strcmp(cmd, "oeste") == 0) dir = 2;
        else if (strcmp(cmd, "este") == 0) dir = 3;
        else if (strcmp(cmd, "cima") == 0) dir = 4;
        else if (strcmp(cmd, "baixo") == 0) dir = 5;
        else if (strcmp(cmd, "apanhar") == 0) {
            if (s->id_objeto != -1) {
                jogo->jogador.id_objeto_mao = s->id_objeto;
                s->id_objeto = -1;
                sprintf(jogo->ultima_mensagem, "Apanhaste: %s", jogo->objetos[jogo->jogador.id_objeto_mao].nome);
            } else {
                strcpy(jogo->ultima_mensagem, "Nada para apanhar.");
            }
        }
        else if (strcmp(cmd, "tesouro") == 0) {
            if (s->tem_tesouro == 1) {
                jogo->jogador.tem_tesouro = 1;
                s->tem_tesouro = 0;
                strcpy(jogo->ultima_mensagem, "TESOURO APANHADO! Foge para a Sala 0!");
            } else {
                strcpy(jogo->ultima_mensagem, "Sem tesouro aqui.");
            }
        }
        else if (strcmp(cmd, "gravar") == 0) gravar_jogo();
        else if (strcmp(cmd, "carregar") == 0) carregar_save();
        else if (strcmp(cmd, "sair") == 0) jogo->jogo_a_correr = 0;
        else strcpy(jogo->ultima_mensagem, "Comando invalido.");

        if (dir != -1) {
            nova_sala = s->direcoes[dir];
            if (nova_sala != -1) {
                jogo->jogador.local = nova_sala;
            } else {
                strcpy(jogo->ultima_mensagem, "Caminho bloqueado.");
            }
        }
        sem_post(&jogo->mutex);
    }
    exit(0);
}

// --- Main (Motor de Jogo) ---

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL); 
    signal(SIGINT, handle_sigint);

    // 1. Memória Partilhada
    shmid = shmget(SHM_KEY, sizeof(EstadoJogo), IPC_CREAT | 0666);
    if (shmid < 0) { perror("Erro shmget"); exit(1); }
    
    jogo = (EstadoJogo *)shmat(shmid, NULL, 0);
    if (jogo == (void *)-1) { perror("Erro shmat"); exit(1); }

    sem_init(&jogo->mutex, 1, 1);

    // 2. Inicialização
    jogo->jogo_a_correr = 1;
    jogo->em_combate = 0;
    strcpy(jogo->ultima_mensagem, "Bem-vindo!");
    strcpy(jogo->log_combate, "");
    carregar_objetos();
    carregar_mapa("mapa.txt");

    // Valores Default
    jogo->jogador.energia = 100;
    jogo->jogador.local = 0;
    jogo->jogador.id_objeto_mao = -1;
    jogo->jogador.tem_tesouro = 0;
    
    jogo->monstro.energia = 100;
    jogo->monstro.local = 2; 

    // Modo Super User
    if (argc == 5 && strcmp(argv[1], "1234") == 0) {
        jogo->jogador.energia = atoi(argv[2]);
        int sala = atoi(argv[3]);
        if (sala < jogo->num_salas) jogo->jogador.local = sala;
        jogo->jogador.id_objeto_mao = atoi(argv[4]);
    }

    // 3. Criar Processos
    pid_monstro = fork();
    if (pid_monstro == 0) processo_monstro();

    pid_jogador = fork();
    if (pid_jogador == 0) processo_jogador();

    // 4. Loop Principal
    while (jogo->jogo_a_correr) {
        sem_wait(&jogo->mutex);
        
        // Vitória
        if (jogo->jogador.tem_tesouro == 1 && jogo->jogador.local == 0) {
            jogo->jogo_a_correr = 0;
            sem_post(&jogo->mutex);
            printf("\n\n*** PARABENS! VITORIA! ***\n");
            break;
        }

        // Combate
        if (jogo->jogador.local == jogo->monstro.local && jogo->monstro.energia > 0) {
            jogo->em_combate = 1;
            
            // Cálculos
            int dano_base = 5;
            int dano_arma = 0;
            char nome_arma[50] = "Punhos";
            
            if (jogo->jogador.id_objeto_mao != -1) {
                dano_arma = jogo->objetos[jogo->jogador.id_objeto_mao].eficacia;
                strcpy(nome_arma, jogo->objetos[jogo->jogador.id_objeto_mao].nome);
            }
            int total_dano_jog = dano_base + dano_arma;
            int dano_monstro = (rand() % 15) + 5; 
            
            int hp_mon_antes = jogo->monstro.energia;
            int hp_jog_antes = jogo->jogador.energia;
            
            jogo->monstro.energia -= total_dano_jog;
            jogo->jogador.energia -= dano_monstro;
            
            // Log Detalhado
            sprintf(jogo->log_combate, 
                "JOGADOR ATACA:\n"
                " > Base (5) + %s (%d) = %d Dano\n"
                " > Monstro HP: %d -> %d\n\n"
                "MONSTRO ATACA:\n"
                " > Ataque Furioso (Random 5-20): %d Dano\n"
                " > Jogador HP: %d -> %d\n",
                nome_arma, dano_arma, total_dano_jog,
                hp_mon_antes, jogo->monstro.energia,
                dano_monstro,
                hp_jog_antes, jogo->jogador.energia
            );

            if (jogo->monstro.energia <= 0) {
                strcat(jogo->log_combate, "\n>>> O MONSTRO FOI DERROTADO! <<<");
                jogo->monstro.local = -1; 
            }
            
            if (jogo->jogador.energia <= 0) {
                jogo->jogo_a_correr = 0;
                strcat(jogo->log_combate, "\n>>> MORRESTE... <<<");
            }
            
            sem_post(&jogo->mutex);
            sleep(2); // Pausa para dar ritmo ao combate
            
            sem_wait(&jogo->mutex);
            if (jogo->monstro.energia <= 0) jogo->em_combate = 0;
            sem_post(&jogo->mutex);
            
        } else {
            if (jogo->em_combate == 1) jogo->em_combate = 0;
            sem_post(&jogo->mutex);
        }
        
        usleep(200000);
    }

    kill(pid_monstro, SIGKILL);
    kill(pid_jogador, SIGKILL);
    wait(NULL);
    wait(NULL);
    limpar_recursos();
    
    return 0;
}