#include "server_control.h"

int main (){


	// Inicializa as structs e a comunicacao com o cliente
	server_start();

	char command;

	server_print_board();

	int treasures_remaining = TREASURES;
	
	while(treasures_remaining > 1){

		// Recebe um comando do cliente
		command = server_receive_command();

		// Tenta realizar o comando do cliente
		// Verifica se encontrou um tesouro
		treasures_remaining -= server_walk(command);

		// Imprime o board e o historico de comandos do cliente
		server_print_board();
		server_print_seq_events();

		// Envia ao cliente a resposta montada apos o server_walk
		server_send_answer();
	}


	return 0;
}
