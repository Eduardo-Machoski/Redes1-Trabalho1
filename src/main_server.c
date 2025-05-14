#include "server_control.h"

int main (){

	server_start();

	char command;

	while(Treasures_found > 1){
		command = server_receive_command();

		server_walk(command);

		server_print_board();
		
		server_print_seq_events();

		server_send_answer();
	}


	return 0;
}
