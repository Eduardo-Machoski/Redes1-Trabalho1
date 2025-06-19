#include "client_control.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

//=====================VARIAVEIS GLOBAIS================================

board g_board;					// Estado atual do tabuleiro

char *Treasure_path;			// Path para o ultimo tesouro recebido

package_t command_request; // Pacote com o comando enviado
package_t answer_package;	// Ultima resposta do servidor



//===========================FUNCOES INTERNAS===================================


//===========================FUNCOES EXTERNAS===================================

// Inicializa o jogo e seus componente
	// - Tabuleiro
	// - Pacotes de mensagens
	// - Conexao com o servidor
void client_init_game(){
	
	// Inicializa tabuleiro
	for(int i = 0; i < BOARDSIZE; i++)
		for(int j = 0; j < BOARDSIZE; j++)
			g_board.board[i][j] = '#';

	// Posicao inicial do player
	g_board.board[0][0] = '0';

	g_board.player_x = 0;
	g_board.player_y = 0;

	command_request.size = 0;

	// Configura a conexao com o servidor
	 protocol_init(PATH_INTERFACE);
}

//======================================================================

// Imprime o estado atual do tabuleiro
void client_print_board(){
	// Tabuleiro 'desconhecido'
   for(int i = BOARDSIZE - 1; i >= 0; i--){
      for(int j = 0; j < BOARDSIZE; j++)
         printf("%c", g_board.board[i][j]);
      printf("\n");
   }
}

//======================================================================

// Obtem um comando valido do usuario
// Comandos validos:
	// - 'A', 'a'
	// - 'S', 's'
	// - 'D', 'd'
	// - 'W', 'w'
// Monta o command_request com base no comando
void client_get_valid_command(){
	uchar c = '\0';

   while((c != 'A') && (c != 'S') && (c != 'W') && (c != 'D')){
      c = getchar();
      c = toupper(c);
   }

   switch (c){
      case 'A':
			command_request.type = ESQUERDA;
         break;
      case 'S':
			command_request.type = BAIXO;
         break;
      case 'D':
			command_request.type = DIREITA;
         break;
      case 'W':
			command_request.type = CIMA;
         break;
      default:
			perror("Erro tipo comando!");
         exit(1);
         break;
   }

	#ifdef DEBUG
		printf("Comando: %c ", c);
	#endif
}

//======================================================================

// Envia o command_request ao servidor
// Recebe a resposta do servidor
// Caso encontre um tesouro recebe o tesouro e salva seu path em Treasure_path
// Respostas:
	 // - OK : Posicao no tabuleiro sem tesouro
	 // - VIDEO/TEXTO/IMAGEM : Posicao no tabuleiro com tesouro
	 // - ACK :  Player fica parado
uchar client_send_command_request(){

	// Envia o command_request
	protocol_send_package(&command_request, true);

	// Recebe a resposta do servidor
	protocol_recieve_active(&answer_package);

	return answer_package.type;
}

//======================================================================

// Anda no tabuleiro
void client_walk(){
	
	// marca a casa atual do player como ja visitada
   g_board.board[g_board.player_y][g_board.player_x] = ' ';

	// Move o player com base no ultimo comando enviado ao servidor
   switch (command_request.type){
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

//======================================================================

// Recebe o tesouro do servidor na seginte ordem:
	// TAMANHO - Verifica se ha espaco em disco suficiente
	// DADOS - Repete recepcao de dados
	// FIM_FILE - Fim da recepcao de dados e do tesouro
void client_recieve_treasure(uchar type){
}

//======================================================================

// Abre o tesouro com base em:
	//type : tipo do tesouro
	//Treasure_path : caminho para o tesouro recebido por ultimo
void client_open_treasure(uchar type){
	switch (type){
		case VIDEO:
			break;
		case TEXTO:
			break;
		case IMAGEM:
			break;
		default:
			perror("Erro tipo de tesouro");
			exit(1);
			break;
	}
}
