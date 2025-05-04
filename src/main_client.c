#include <stdio.h>
#include <stdlib.h>

#include "client_control.h"

//fluxo principal do jogo por parte do cliente
int main(){	
	
   client_set_up_game();

   unsigned char next_command;
	unsigned char treasure_type;

   unsigned int command_response;

   // sem criterio de parada por enquanto (precisa fazer)
   while(1 == 1){
       
      client_print_board();
       
		next_command = client_get_valid_command();	

      command_response = client_send_command_request(next_command, &treasure_type);

      // Tratamento das respostas do servidor
      switch (command_response) {
			case EMPTY:
            client_walk_empty(next_command);
            break;
         case TREASURE:
            client_walk_treasure(next_command, treasure_type);
            break;
         case FAIL:
            break;
         default:
            perror("Falha commando invalido");
            exit(1);
            break;
      }
   }

   return 0;
}
