#ifndef JOGO_H
#define JOGO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>

// --- Constantes ---
#define MAX_NOME 50
#define MAX_DESC 256
#define MAX_SALAS 20
#define MAX_OBJETOS 10
#define SHM_KEY 12345 // Chave única para a memória partilhada

// --- Estruturas ---

typedef struct {
    char nome[MAX_NOME];
    int eficacia; 
} Objeto;

typedef struct {
    // 0:Norte, 1:Sul, 2:Oeste, 3:Este, 4:Cima, 5:Baixo
    int direcoes[6]; 
    char descricao[MAX_DESC];
    int id_objeto;    // -1 se não houver
    int tem_tesouro;  // 0: não, 1: sim
} Sala;

typedef struct {
    char nome[MAX_NOME];
    int energia;
    int local;        // Índice da sala
    int id_objeto_mao; // -1 se vazio
    int tem_tesouro;   // 0: não, 1: sim
} Jogador;

typedef struct {
    int energia;
    int local;
} Monstro;

// Estrutura Global partilhada
typedef struct {
    Sala salas[MAX_SALAS];
    Objeto objetos[MAX_OBJETOS];
    Jogador jogador;
    Monstro monstro;
    int num_salas;
    int num_objetos;
    
    // Controlos de Estado
    int jogo_a_correr; 
    int em_combate;
    char ultima_mensagem[200]; // Para feedback visual (ex: "Apanhaste a espada")
    
    // Sincronização
    sem_t mutex; 
} EstadoJogo;

#endif