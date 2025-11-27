#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Estrutura do Jogador
struct Player{
	char name[20];
	int energy;
	int cell;
	int object;
	int treasure;
};

// Estrutura da Célula/Sala
struct Cell{
	int north;
	int south;
	int west;
	int east;
	int up;
	int down;
	int object;
	int treasure;
	char description[100];
};

// Estrutura do Objeto
struct Object{
	char name[30];
	int effectiveness;
};

// Estrutura do Monstro
struct Monster{
	int energy;
	int cell;
};

// Variáveis globais
struct Cell map[100];
struct Object objects[10];
int nCells;
int nObjects;

// ============================================
// FUNÇÕES DE INICIALIZAÇÃO
// ============================================

void InitializePlayer(struct Player* pPlayer){
	printf("Olá aventureiro! Como te chamas?\n");
	scanf("%s", pPlayer->name);
	pPlayer->energy = 100;
	pPlayer->cell = 0;
	pPlayer->object = -1;
	pPlayer->treasure = -1;
}

void InitializeObjects(){
	nObjects = 3;

	// Objeto 0
	strcpy(objects[0].name, "Chave de Tanoeiro");
	objects[0].effectiveness = 15;

	// Objeto 1
	strcpy(objects[1].name, "Lamparina de Azeite");
	objects[1].effectiveness = 10;

	// Objeto 2
	strcpy(objects[2].name, "Espada Enferrujada");
	objects[2].effectiveness = 20;
}

void InitializeMap(){
	// Cell 0 - Tanoaria da Mouraria
	map[0].north = 1;
	map[0].south = -1;
	map[0].west = -1;
	map[0].east = -1;
	map[0].up = -1;
	map[0].down = -1;
	map[0].object = 0;
	map[0].treasure = -1;
	strcpy(map[0].description, "Tanoaria da tua família. Cheira a madeira e vinho.");

	// Cell 1 - Rua da Mouraria
	map[1].north = 2;
	map[1].south = 0;
	map[1].west = -1;
	map[1].east = -1;
	map[1].up = -1;
	map[1].down = -1;
	map[1].object = -1;
	map[1].treasure = -1;
	strcpy(map[1].description, "Rua estreita da Mouraria. Crianças brincam ao longe.");

	// Cell 2 - Largo do Farol
	map[2].north = -1;
	map[2].south = 1;
	map[2].west = 4;
	map[2].east = 3;
	map[2].up = -1;
	map[2].down = 5;
	map[2].object = 1;
	map[2].treasure = -1;
	strcpy(map[2].description, "Largo abandonado. Uma escadaria desce para as galerias.");

	// Cell 3 - Beco do Eco
	map[3].north = -1;
	map[3].south = -1;
	map[3].west = 2;
	map[3].east = -1;
	map[3].up = -1;
	map[3].down = -1;
	map[3].object = -1;
	map[3].treasure = -1;
	strcpy(map[3].description, "Beco sem saída. Os sons ecoam de forma estranha...");

	// Cell 4 - Travessa das Sombras
	map[4].north = -1;
	map[4].south = -1;
	map[4].west = -1;
	map[4].east = 2;
	map[4].up = -1;
	map[4].down = -1;
	map[4].object = 2;
	map[4].treasure = -1;
	strcpy(map[4].description, "Travessa escura. Algo te observa das sombras...");

	// Cell 5 - Galerias Subterrâneas
	map[5].north = -1;
	map[5].south = -1;
	map[5].west = -1;
	map[5].east = -1;
	map[5].up = 2;
	map[5].down = -1;
	map[5].object = -1;
	map[5].treasure = 1;
	strcpy(map[5].description, "Galerias antigas. O Barril das Almas está aqui!");

	nCells = 6;
}

void InitializeMonster(struct Monster* pMonster){
	pMonster->energy = 60;
	pMonster->cell = 4;
}

// ============================================
// FUNÇÕES DE IMPRESSÃO/DEBUG
// ============================================

void PrintPlayer(struct Player player){
	printf("\n=== ESTADO DO JOGADOR ===\n");
	printf("Nome: %s\n", player.name);
	printf("Energia: %d\n", player.energy);
	printf("Célula: %d\n", player.cell);
	if(player.object != -1){
		printf("Objeto: %s\n", objects[player.object].name);
	} else {
		printf("Objeto: Nenhum\n");
	}
	if(player.treasure == 1){
		printf("Tesouro: Barril das Almas\n");
	} else {
		printf("Tesouro: Não encontrado\n");
	}
}

void PrintMonster(struct Monster monster){
	printf("\n=== ESTADO DO MONSTRO ===\n");
	printf("Energia: %d\n", monster.energy);
	printf("Célula: %d\n", monster.cell);
}

void PrintMap(){
	printf("\n=== MAPA DO JOGO ===\n");
	for(int i = 0; i < nCells; i++){
		printf("\nCélula %d:\n", i);
		printf("  Norte: %d | Sul: %d | Este: %d | Oeste: %d\n",
			map[i].north, map[i].south, map[i].east, map[i].west);
		printf("  Cima: %d | Baixo: %d\n", map[i].up, map[i].down);
		printf("  Descrição: %s\n", map[i].description);
		if(map[i].object != -1){
			printf("  Objeto: %s\n", objects[map[i].object].name);
		}
		if(map[i].treasure == 1){
			printf("  TESOURO ESTÁ AQUI!\n");
		}
	}
}

// ============================================
// FUNÇÕES DO JOGO
// ============================================

void DescribeLocation(struct Player player){
	printf("\n----------------------------------------\n");
	printf("LOCALIZAÇÃO: Célula %d\n", player.cell);
	printf("----------------------------------------\n");
	printf("%s\n", map[player.cell].description);

	// Mostrar objeto disponível
	if(map[player.cell].object != -1){
		printf("\n>> Vês aqui: %s\n", objects[map[player.cell].object].name);
	}

	// Mostrar tesouro
	if(map[player.cell].treasure == 1){
		printf("\n*** O BARRIL DAS ALMAS ESTÁ AQUI! ***\n");
	}

	// Mostrar saídas
	printf("\nSaídas disponíveis: ");
	if(map[player.cell].north != -1) printf("[N]orte ");
	if(map[player.cell].south != -1) printf("[S]ul ");
	if(map[player.cell].east != -1) printf("[E]ste ");
	if(map[player.cell].west != -1) printf("[O]este ");
	if(map[player.cell].up != -1) printf("[C]ima ");
	if(map[player.cell].down != -1) printf("[B]aixo ");
	printf("\n");
}

void AcceptCommand(struct Player* pPlayer, char* command){
	printf("\nComandos: [N/S/E/O/C/B] Mover | [P]egar objeto | [I]nfo | [Q]uit\n");
	printf("Comando: ");
	scanf(" %c", command);
}

void MovePlayer(struct Player* pPlayer, char command){
	int destination = -1;

	switch(command){
		case 'N': case 'n':
			destination = map[pPlayer->cell].north;
			break;
		case 'S': case 's':
			destination = map[pPlayer->cell].south;
			break;
		case 'E': case 'e':
			destination = map[pPlayer->cell].east;
			break;
		case 'O': case 'o':
			destination = map[pPlayer->cell].west;
			break;
		case 'C': case 'c':
			destination = map[pPlayer->cell].up;
			break;
		case 'B': case 'b':
			destination = map[pPlayer->cell].down;
			break;
		case 'P': case 'p':
			// Pegar objeto
			if(map[pPlayer->cell].object != -1){
				pPlayer->object = map[pPlayer->cell].object;
				printf("\nApanhaste: %s!\n", objects[pPlayer->object].name);
				map[pPlayer->cell].object = -1;
			} else {
				printf("\nNão há nada para apanhar aqui.\n");
			}
			return;
		case 'I': case 'i':
			PrintPlayer(*pPlayer);
			return;
		default:
			printf("\nComando inválido!\n");
			return;
	}

	if(destination == -1){
		printf("\nNão podes ir nessa direção!\n");
	} else {
		pPlayer->cell = destination;
		printf("\nMoveste-te para a célula %d.\n", pPlayer->cell);

		// Apanhar tesouro automaticamente
		if(map[pPlayer->cell].treasure == 1 && pPlayer->treasure == -1){
			pPlayer->treasure = 1;
			map[pPlayer->cell].treasure = -1;
			printf("\n*** ENCONTRASTE O BARRIL DAS ALMAS! ***\n");
			printf("Agora tens que voltar à Tanoaria (célula 0)!\n");
		}
	}
}

void MoveMonster(struct Monster* pMonster){
	// Movimento aleatório do monstro
	if(pMonster->cell == -1) return; // Monstro morto

	int directions[6] = {
		map[pMonster->cell].north,
		map[pMonster->cell].south,
		map[pMonster->cell].east,
		map[pMonster->cell].west,
		map[pMonster->cell].up,
		map[pMonster->cell].down
	};

	int attempts = 0;
	while(attempts < 10){
		int dir = rand() % 6;
		if(directions[dir] != -1){
			pMonster->cell = directions[dir];
			break;
		}
		attempts++;
	}
}

void Fight(struct Player* pPlayer, struct Monster* pMonster){
	printf("\n========================================\n");
	printf("   ENCONTRASTE A SOMBRA SÓNICA!\n");
	printf("========================================\n");

	while(pPlayer->energy > 0 && pMonster->energy > 0){
		printf("\n--- COMBATE ---\n");
		printf("Tua energia: %d | Sombra Sónica: %d\n",
			pPlayer->energy, pMonster->energy);

		printf("\n[A]tacar | [F]ugir\n");
		printf("Ação: ");
		char action;
		scanf(" %c", &action);

		if(action == 'A' || action == 'a'){
			// Jogador ataca
			int damage = (pPlayer->object != -1) ?
				objects[pPlayer->object].effectiveness : 5;
			pMonster->energy -= damage;

			if(pPlayer->object != -1){
				printf("Atacaste com %s! Causaste %d de dano!\n",
					objects[pPlayer->object].name, damage);
			} else {
				printf("Atacaste com os punhos! Causaste %d de dano!\n", damage);
			}

			// Monstro contra-ataca
			if(pMonster->energy > 0){
				int monsterDamage = 8 + (rand() % 5);
				pPlayer->energy -= monsterDamage;
				printf("A Sombra Sónica contra-ataca! Perdeste %d de energia!\n",
					monsterDamage);
			}
		} else if(action == 'F' || action == 'f'){
			printf("\nFugiste do combate!\n");

			// Mover jogador para célula aleatória adjacente
			int directions[6] = {
				map[pPlayer->cell].north,
				map[pPlayer->cell].south,
				map[pPlayer->cell].east,
				map[pPlayer->cell].west,
				map[pPlayer->cell].up,
				map[pPlayer->cell].down
			};

			for(int i = 0; i < 6; i++){
				if(directions[i] != -1){
					pPlayer->cell = directions[i];
					printf("Fugiste para a célula %d!\n", pPlayer->cell);
					break;
				}
			}
			return;
		} else {
			printf("Comando inválido! Perdeste a vez...\n");
		}
	}

	if(pMonster->energy <= 0){
		printf("\n*** DERROTASTE A SOMBRA SÓNICA! ***\n");
		pMonster->cell = -1; // Monstro morre
	}
}

// ============================================
// FUNÇÃO PRINCIPAL
// ============================================

int main(){
	struct Player player;
	struct Monster monster;
	char command;
	int gameOver = 0;

	// Inicializar random
	srand(time(NULL));

	// Título do jogo
	printf("========================================\n");
	printf("   O ÚLTIMO FAROL DE ALFAMA\n");
	printf("========================================\n");
	printf("\nO teu irmão Miguel desapareceu nas galerias de Alfama.\n");
	printf("Encontra o Barril das Almas antes que a Sombra Sónica\n");
	printf("consuma a cidade!\n\n");

	// Inicializar Player
	InitializePlayer(&player);

	// Inicializar Objects
	InitializeObjects();

	// Inicializar Map
	InitializeMap();

	// Inicializar Monster
	InitializeMonster(&monster);

	printf("\nPrima Enter para começar a aventura...");
	getchar();
	getchar();

	// Loop principal do jogo
	while(!gameOver && player.energy > 0){
		// Movimentar monstro
		MoveMonster(&monster);

		// Descrever localização do jogador
		DescribeLocation(player);

		// Mostrar status
		printf("\nEnergia: %d | ", player.energy);
		if(player.object != -1){
			printf("Arma: %s | ", objects[player.object].name);
		} else {
			printf("Arma: Nenhuma | ");
		}
		if(player.treasure == 1){
			printf("Tesouro: SIM");
		} else {
			printf("Tesouro: NÃO");
		}
		printf("\n");

		// Aceitar comando do jogador
		AcceptCommand(&player, &command);

		// Verificar comando de saída
		if(command == 'Q' || command == 'q'){
			printf("\nA sair do jogo...\n");
			gameOver = 1;
			continue;
		}

		// Movimentar jogador
		MovePlayer(&player, command);

		// Verificar vitória
		if(player.treasure == 1 && player.cell == 0){
			printf("\n========================================\n");
			printf("   MISSÃO CUMPRIDA!\n");
			printf("========================================\n");
			printf("Regressaste à tanoaria com o Barril das Almas!\n");
			printf("Alfama está salva... por agora.\n");
			gameOver = 1;
			continue;
		}

		// Se jogador e monstro estão na mesma célula
		if(player.cell == monster.cell && monster.energy > 0){
			Fight(&player, &monster);
		}

		// Consumo de energia por turno
		player.energy -= 1;

		printf("\nPrima Enter para continuar...");
		getchar();
	}

	// Apresentar resultado final
	printf("\n========================================\n");
	if(player.energy <= 0){
		printf("   GAME OVER\n");
		printf("========================================\n");
		printf("A Sombra Sónica consumiu-te...\n");
		printf("Alfama está perdida.\n");
	} else if(player.treasure == 1 && player.cell == 0){
		printf("   VITÓRIA!\n");
		printf("========================================\n");
		printf("Pontuação final: %d\n", player.energy * 10);
	}
	printf("\nObrigado por jogares!\n");

	return 0;
}
