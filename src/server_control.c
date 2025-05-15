#include "server_control.h"

void server_start(){

}

// Espera passivamente por um comando enviado do cliente
// Ao receber uma mensagem valida verifica:
//		- Sequencia
//    - Checksum
uchar server_receive_command(){
	return 'C';
}

// Recebe um commando e tenta andar no board
// Situacoes:
//		- Andou para local vazio
//			- Anda no board
//			- Seta a resposta para ok
//		- Andou para local com tesouro
//			- Anda no board
//			- Seta a resposta para o tipo do tesouro
//		-Andou para local invalido
//			-Seta a resposta para ack
// Retorno:
// 	0 - Se nao encontrou tesouro
//    1 - Se encontrou um tesouro
int server_walk(uchar command){

	return 0;
}

// Imprime o board no estado atual
void server_print_board(){

}

// Imprime a sequencia de passos validos do cliente
void server_print_seq_events(){

}

// Envia ao cliente a resposta montada anteriormente
void server_send_answer(){

}
