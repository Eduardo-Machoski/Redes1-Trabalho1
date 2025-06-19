#include "server_control.h"

#include <string.h>

//=====================VARIAVEIS GLOBAIS==============================    ==

board g_board;						// Estado atual do tabuleiro

package_t send_package;			// Pacote a ser enviado 
package_t last_package;			// Ultimo pacote enviado 

package_t recieved_package;	// Ultimo pacote recebido

char *g_aux_string = "teste";

treasure_t treasures[TREASURES];      // Vetor com os tesouros e suas informacoes

//===========================FUNCOES INTERNAS===================================

void local_build_send_package(uchar type, char *path){
	#ifdef DEBUG
	printf("Local_build_send_package build: %u\n", type);
	#endif

	switch (type){
		// Tipos sem dados enviados
		case ACK:
		case NACK:
		case OK:
		case FIM_FILE:
			send_package.type = type;
			send_package.size = 0;
		break;

		// Envia o nome do arquivo alem do tipo dele
		case TREASURE_FOUND:
			send_package.type = VIDEO;

			// Tamanho padrao dos nomes de arquivos do jogo
			send_package.size = 5;
			
			// Inicio do nome do arquivo do tesouro
			char *aux = &path[strlen(path) - 5];

			// Coloca o nome do tesouro no send_package
			for(int i = 0; i < 5; i++)
				send_package.data[i] = aux[i];

		break;


		case TAMANHO:
			send_package.type = type;
			

			// Tem que fazer o sistema de arquivos e dai atualizar aqui
			send_package.data[0] = '1';
			send_package.size = 1;
		break;

		default:
			perror("Erro type local_build_send_package");
			exit (1);
		break;
	}
}

// Verifica se a posicao atual do player possui um tesouro
// Retorno:
	// NULL - Posicao atual nao tem tesouro
	// Path do tesouro - Posicao atual tem um tesouro
char *local_treasure_found(){
	for(int i = 0; i < TREASURES; i++){
		if(!treasures[i].found)
			if(g_board.player_x == treasures[i].x && g_board.player_y == treasures[i].y)
			{
				treasures[i].found = true;
				return treasures[i].path;
			}
	}

	return NULL;
}

// Cria um tesouro em uma posicao vazia do tabuleiro
void local_init_treasure(int index){
	bool valid;

	// Garante que a posicao criada pelo tesouro eh vazia
	do{
		valid = true;

		treasures[index].x = rand() % BOARDSIZE;
		treasures[index].y = rand() % BOARDSIZE;

		// Posicao 0-0 nao eh valida
		if(treasures[index].x == 0 && treasures[index].y == 0)
			valid = false;
			
		// Verifica se nao ha outro tesouro nessa posicao
		for(int i = 0; i < index; i++)
			if(treasures[index].x == treasures[i].x && treasures[index].y == treasures[i].y)
				valid = false;
			
	} while (!valid);


	// Inicializa o resto do tesouro
	treasures[index].found = false;
	treasures[index].path = g_aux_string;
}

//===========================FUNCOES EXTERNAS=========================    ==========

// Inicializa o servidor e seus componentes
	// - Tabuleiro
	// - Tesouros
	// - Pacotes de mensagens
	// - Conexao com o cliente
void server_init(){
	
	// Inicializa o tabuleiro
	for(int i = 0; i < BOARDSIZE; i++)
		for(int j = 0; j < BOARDSIZE; j++)
			g_board.board[i][j] = ' ';

	// Posicao inicial do player
	g_board.board[0][0] = '0';

	g_board.player_x = 0;
	g_board.player_y = 0;

	// Inicializa os tesouros
	for(int i = 0; i < TREASURES; i++){
		local_init_treasure(i);
		
		// Insere as posicoes de cada tesouro no tabuleiro
		g_board.board[treasures[i].y][treasures[i].x] = i + '1';
	}
	
	// Configura o socket de comunicacao
	protocol_init(PATH_INTERFACE);

}

// Espera passivamente por um pacoteenviado do cliente
// Retorno:
	// Tipo do pacote recebido
uchar server_recieve_command(){
	protocol_recieve_passive(&recieved_package, &last_package);

	return recieved_package.type;
}

// Recebe um commando e tenta andar no board
// Situacoes:
	// Andou para local vazio
		// Anda no board
		// Seta a resposta para ok
	// Andou para local com tesouro
		// Anda no board
		// Seta a resposta para o tipo do tesouro
	// Andou para local invalido
		// Seta a resposta para ack
// Retorno:
	// Path do tesouro
	// Se nao encontrou tesouro
		// Retorna NULL
char *server_walk(uchar command){

	bool walk_fail = false;

	g_board.board[g_board.player_y][g_board.player_x] = ' ';

	switch (command){
		case CIMA:
				if(g_board.player_y == BOARDSIZE - 1)
					walk_fail = true;
				else
					g_board.player_y += 1;
			break;
		case BAIXO:
				if(g_board.player_y == 0)
					walk_fail = true;
				else
					g_board.player_y -= 1;
			break;
		case ESQUERDA:
				if(g_board.player_x == 0)
					walk_fail = true;
				else
					g_board.player_x -= 1;
			break;	
		case DIREITA:
				if(g_board.player_x == BOARDSIZE - 1)
					walk_fail = true;
				else
					g_board.player_x += 1;
			break;
		default:
			perror("Erro comando server_walk");
			exit(1);
			break;
	}

	g_board.board[g_board.player_y][g_board.player_x] = '0';

	// Player tentou andar para fora do tabuleiro
	if(walk_fail){
		local_build_send_package(OK, NULL);
		return NULL;
	}

	// Verifica se encontrou um tesouro
	char *path = local_treasure_found();

	if(!path)
		local_build_send_package(ACK, path);
	else
		local_build_send_package(TREASURE_FOUND, path);

	return path;
}

// Imprime o board no estado atual
void server_print_board(){
	// Tabuleiro 'desconhecido'
   for(int i = BOARDSIZE - 1; i >= 0; i--){
      for(int j = 0; j < BOARDSIZE; j++)
         printf("%c", g_board.board[i][j]);
      printf("\n");
   }
}

// Imprime a sequencia de passos validos do cliente
void server_print_seq_events(){

}

// Envia ao cliente a resposta montada anteriormente
void server_send_answer(char *path){
	//FAZER muita coisa ainda
	protocol_send_package(&send_package, true);	
}
