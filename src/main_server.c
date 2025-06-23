#include "server_control.h"

int main() {

	//Inicializa o servidor e comunicacao com o cliente
	server_init();

	server_print_board();

	uchar command;

	char *treasure_path;
	int treasures_remaining = TREASURES;

	while(treasures_remaining > 0){

		// Recebe um comando do cliente
		command = server_recieve_command();

		// Realiza o comando do cliente
		// Retorna o path do tesouro (se encontrado)
		treasure_path = server_walk(command);

		server_print_board();
		server_print_seq_events();

		if(treasure_path)
			treasures_remaining--;

		// Envia o pacote de resposta ao cliente
		server_send_answer(treasure_path);
	}

	return 0;
}
