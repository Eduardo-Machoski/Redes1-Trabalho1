#include "client_control.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

//=====================VARIAVEIS GLOBAIS================================

board g_board;					// Estado atual do tabuleiro

char *Treasure_path;			// Path para o ultimo tesouro recebido

uchar seq_package;			// Sequencia do pacote a ser enviada
uchar expected_package;		// Sequencia esperada da resposta

package_t command_request; // Pacote com o comando enviado
package_t last_package;		// Pacote com a ultima mensagem enviada
package_t answer_package;	// Ultima resposta do servidor

int connection_socket;		// Numero do socket para envio de pacotes


//===========================FUNCOES INTERNAS===================================

// Incrementa o seq_package (vai de 0 a 31 circular)
void local_inc_seq(){
	seq_package += 1;

	if(seq_package == 32)
		seq_package = 0;
}

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

	// Configura os dados de pacotes de mensagens
	seq_package = 0;
	expected_package = 0;
	
	command_request.size = 0;

	// Configura a conexao com o servidor
	connection_socket = protocol_create_raw_socket(PATH_INTERFACE);
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

	// Atribui a sequencia do comando atual
	command_request.seq = seq_package;
	
	// Incrementa seq_package
	local_inc_seq();

	// Recebe a resposta do servidor
	protocol_recieve_active(connection_socket, expected_package, &answer_package);

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
