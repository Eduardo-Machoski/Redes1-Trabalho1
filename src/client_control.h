#ifndef __CLIENT__
#define __CLIENT__

	#include "struct_board.h"
	#include "connection_protocol.h"

	typedef unsigned char uchar;

	// Inicializa o jogo e seus componente
      // - Tabuleiro
		// - Pacotes de mensagens
   	// - Conexao com o servidor
   void client_init_game();

	// Imprime o estado atual do tabuleiro
   void client_print_board();

   // Obtem um comando valido do usuario
   // Comandos validos:
      // - 'A', 'a'
      // - 'S', 's'
      // - 'D', 'd'
      // - 'W', 'w'
   void client_get_valid_command();

   // Envia o command_request ao servidor
   // Recebe a resposta do servidor
   // Caso encontre um tesouro recebe o tesouro e salva seu path em Treasure_path
   // Respostas:
       // - OK : Posicao no tabuleiro sem tesouro
       // - VIDEO/TEXTO/IMAGEM : Posicao no tabuleiro com tesouro
       // - ACK :  Player fica parado
   uchar client_send_command_request();

	// Anda no tabuleiro
		// Marca a casa atual como X se tinha um tesouro, se nao marca com um espaco
   void client_walk(bool *treasure);


	// Recebe o tesouro do servidor na seginte ordem:
		// TAMANHO - Verifica se ha espaco em disco suficiente
		// DADOS - Repete recepcao de dados
		// FIM_FILE - Fim da recepcao de dados e do tesouro
	void client_recieve_treasure(uchar type);

	// Abre o tesouro com base em:
		//type : tipo do tesouro
		//Treasure_path : caminho para o tesouro recebido por ultimo
	void client_open_treasure(uchar type);

#endif
