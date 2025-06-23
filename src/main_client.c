#include <stdlib.h>
#include <stdio.h>

#include "client_control.h"

int main(){

	client_init_game();

	client_print_board();

	uchar command_response;

	int treasures_left = TREASURES;
	bool treasure_found = false;

	while(treasures_left > 0){
		client_get_valid_command();

		command_response = client_send_command_request();

		switch(command_response){
			
			// Player andou para uma casa vazia
			case ACK:
				
				client_walk(&treasure_found);

				client_print_board();
				break;

			// Player andou para uma casa com tesouro
			case VIDEO:
			case IMAGEM:
			case TEXTO:
				
				char *path;

				// Anda para a casa com tesouro
				client_walk(&treasure_found);
				
				// Na proxima vez que o player andar a casa será marcada como tesouro
				treasure_found = true;
				client_print_board();

				// Abre o tesouro conforme o seu tipo
				// Cliente recebe o tesouro do servidor
				path = client_receive_treasure(command_response);

				treasures_left--;
				if (path){
					client_open_treasure(command_response, path);
					free(path);
				}
				break;

			// Player nao se move
			case OK:
				client_print_board();
				break;

			// Caso tenha alguma falha no tipo de resposta do servidor
			default:
				perror("Falha resposta servidor invalida!");
				exit(1);
				break;

		}
	}

	printf("FIM DE JOGO\n");

	return 0;
}
