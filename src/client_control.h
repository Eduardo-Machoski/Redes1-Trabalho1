#ifndef __CLIENT__
#define __CLIENT__

	#include "struct_board.h"

	#define EMPTY 0
	#define TREASURE 1
	#define FAIL 2

	typedef unsigned char uchar;


	// Inicializa o jogo e seus componente 
		// - Tabuleiro
		// - Conexao com o servidor
	void client_set_up_game();

	// Imprime o estado atual do tabuleiro
	void client_print_board();

	// Obtem um comando valido do usuario e retorna
	// Comandos validos:
		// - 'A', 'a' 
		// - 'S', 's'
		// - 'D', 'd'
		// - 'W', 'w'
	uchar client_get_valid_command();

	// Envia o comando desejado ao servidor
	// Recebe a resposta do servidor e retorna o codigo de resposta
	// Caso encontre um tesouro recebe o tesouro e salva seu path em Treasure_path
	// Respostas:
		 // - EMPTY : Sucesso no comando, mas posicao no tabuleiro sem tesouro
		 // - TREASURE : Sucesso no comando, posicao no tabuleiro com tesouro
		 // - FAIL : Falha no comando (player fica parado)
	unsigned int client_send_command_request(uchar command, uchar *treasure_type);

	// Anda no tabuleiro para uma casa vazia
	void client_walk_empty(uchar command);

	// Anda no tabuleiro para uma casa com um tesouro
	// Realiza a acao do tesouro (abre txt, da play no mp4 e abre o jpg)
	void client_walk_treasure(uchar command, uchar treasure_type);

#endif
