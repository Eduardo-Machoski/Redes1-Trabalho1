#include "client_control.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

//=====================VARIAVEIS GLOBAIS================================

board g_board;					// Estado atual do tabuleiro

char *Treasure_path;			// Path para o ultimo tesouro recebido

package_t send_package; 		// Pacote de envio 
package_t recieved_package;		// Pacote de recebimento



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

	send_package.size = 0;

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
// Monta o send_package com base no comando
void client_get_valid_command(){
	uchar c = '\0';

   while((c != 'A') && (c != 'S') && (c != 'W') && (c != 'D')){
      c = getchar();
      c = toupper(c);
   }

   switch (c){
      case 'A':
			send_package.type = ESQUERDA;
         break;
      case 'S':
			send_package.type = BAIXO;
         break;
      case 'D':
			send_package.type = DIREITA;
         break;
      case 'W':
			send_package.type = CIMA;
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

// Envia o send_package ao servidor
// Recebe a resposta do servidor
// Caso encontre um tesouro recebe o tesouro e salva seu path em Treasure_path
// Respostas:
	 // - OK : Posicao no tabuleiro sem tesouro
	 // - VIDEO/TEXTO/IMAGEM : Posicao no tabuleiro com tesouro
	 // - ACK :  Player fica parado
uchar client_send_command_request(){

	// Envia o command_request
	protocol_send_package(&send_package, true);

	// Recebe a resposta do servidor
	protocol_recieve_active(&recieved_package);

	return recieved_package.type;
}

//======================================================================

// Anda no tabuleiro
void client_walk(){
	
	// marca a casa atual do player como ja visitada
   g_board.board[g_board.player_y][g_board.player_x] = ' ';

	// Move o player com base no ultimo comando enviado ao servidor
   switch (send_package.type){
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

// Retorna o caminho para o tesouro caso tenha sido baixado corretamente
// Retorna NULL caso contrario
char *client_recieve_treasure(uchar type){
	char *path = recieved_package.data;

	printf("TESOURO ENCONTRADO\n");

	#ifdef DEBUG
		printf("path: %s\n", path);
	#endif

	// Fala que entrou na função de receber arquivo
	send_package.type = ACK;
	send_package.size = 0;

	// Manda ACK
	protocol_send_package(&send_package, true);
	
	// Espera TAMANHO
	protocol_recieve_passive(&recieved_package);
	while(recieved_package.type != TAMANHO){
		protocol_send_package(&send_package, false);
		protocol_recieve_passive(&recieved_package);
	}

	// Guarda tamanho
	uint64_t size = 0;

    for (int i = 0; i < 8; i++) {
        size |= ((uint64_t)recieved_package.data[i]) << (56 - i * 8);
    }

	#ifdef DEBUG
		printf("tamanho do arquivo: %lu\n", size);
	#endif

	return path;
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
