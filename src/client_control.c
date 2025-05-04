#include "client_control.h"
#include "connection_protocol.h"

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

board g_board;
int connection_socket;

//
//======================================================================
//

// Inicializa o tabuleiro e seus componentes
// Configura a conexao com o servidor
void client_set_up_game(){
    
	// Inicializa o tabuleiro e seus componentes
   // Tabuleiro 'desconhecido'
   for(int i = 0; i < BOARDSIZE; i++)
      for(int j = 0; j < BOARDSIZE; j++)
         g_board.board[i][j] = '#';

   // Posicao inicial do player
   g_board.board[0][0] = '0';

   g_board.player_x = 0;
   g_board.player_y = 0;

   if(!(g_board.Treasure_path = (uchar**)malloc(sizeof(uchar*) * TREASURES))){
      perror("Erro malloc treasure path");
      exit(1);    
   }

	// Configura a conexao com o servidor
	connection_socket = protocol_create_raw_socket(PATH_INTERFACE);

}

//
//======================================================================
//

// Imprime o estado atual do tabuleiro
void client_print_board(){

   // Tabuleiro 'desconhecido'
   for(int i = BOARDSIZE - 1; i >= 0; i--){
      for(int j = 0; j < BOARDSIZE; j++)
         printf("%c", g_board.board[i][j]);
      printf("\n");        
   }
}

//
//======================================================================
//

// Obtem um comando valido do usuario e retorna
// Comandos validos:
	// - 'A', 'a'
	// - 'S', 's' 
	// - 'D', 'd' 
	// - 'W', 'w' 
uchar client_get_valid_command(){
	uchar c = '\0';

	while((c != 'A') && (c != 'S') && (c != 'W') && (c != 'D')){
		c = getchar();
		c = toupper(c);
	}

	switch (c){
		case 'A': 
			c = ESQUERDA;
			break;
		case 'S':
			c = BAIXO;
			break;
		case 'D': 
			c = DIREITA;
			break;
		case 'W': 
			c = CIMA;
			break;
		default:
			perror("COMO?");
			exit(1);
			break;
	}

	return c;
}

//
//======================================================================
//

// Envia o comando desejado ao servidor
// Recebe a resposta do servidor e retorna o codigo de resposta
// Caso encontre um tesouro recebe o tesouro e salva seu path em Treasure_path
// Respostas:
    // - EMPTY : Sucesso no comando, mas posicao no tabuleiro sem tesouro
    // - TREASURE : Sucesso no comando, posicao no tabuleiro com tesouro
    // - FAIL : Falha no comando (player fica parado)
unsigned int client_send_command_request(uchar command, uchar *treasure_type){

	// Envia a mensagem de comando para o servidor
	protocol_send_message(connection_socket, 0, 1, command, NULL);

	uchar buffer[131];
	uchar size;
	uchar seq;
	uchar type;
	uchar data[127];

	// Recebe a resposta do servidor
	protocol_recieve_message(connection_socket, buffer, &size, &seq, &type, data);

	switch (type){
		// Andou para uma casa vazia
		case (ACK): return command;

		// Andou e encontrou um tesouro
		case (TEXTO || VIDEO || IMAGEM): 
			*treasure_type = type;
			return TREASURE;

		// Nao andou
		case (OK): return FAIL;
		default:
			perror("TIPO DE MENSAGEM INCORRETO");
			exit(1);
			break;
	}
}

//
//======================================================================
//

// Anda no tabuleiro para uma casa vazia
void client_walk_empty(uchar command){
    
   // marca a casa atual do player como ja visitada
   g_board.board[g_board.player_y][g_board.player_x] = ' ';
    
   switch (command){
      case CIMA:
         g_board.player_y += 1;
         break;
      case ESQUERDA:
         g_board.player_x -= 1;
         break;
      case BAIXO:
         g_board.player_y -= 1;
         break;
      case DIREITA:
         g_board.player_x += 1;
         break;
   }

   g_board.board[g_board.player_y][g_board.player_x] = '0';
}

//
//======================================================================
//

// Anda no tabuleiro para uma casa com um tesouro
// Realiza a acao do tesouro (abre txt, da play no mp4 e abre o jpg)
void client_walk_treasure(uchar command, uchar treasure_type){

}
