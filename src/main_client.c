#include <stdlib.h>
#include <stdio.h>

#include "client_control.h"

int main(){

	client_init_game();

	client_print_board();

	uchar command_response;

	while(1 == 1){
		client_get_valid_command();

		command_response = client_send_command_request();

		switch(command_response){
			
			// Player andou para uma casa vazia
			case ACK:
				
				client_walk();

				client_print_board();
				break;

			// Player andou para uma casa com tesouro
			case VIDEO:
			case IMAGEM:
			case TEXTO:
				
				// Cliente recebe o tesouro do servidor
				client_recieve_treasure(command_response);

				// Anda para a casa com tesouro
				client_walk();
				client_print_board();

				// Abre o tesouro conforme o seu tipo
				client_open_treasure(command_response);

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

	return 0;
}
