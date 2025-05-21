#ifndef __SERVER__
#define __SERVER__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "struct_board.h"
#include "connection_protocol.h"

#define TREASURE_FOUND -1

typedef struct treasure_t {
	unsigned int x;
	unsigned int y;
	char *path;
	bool found;
} treasure_t;


void server_init();

// Espera passivamente por um comando enviado do cliente
// Ao receber uma mensagem valida verifica:
//    - Sequencia
//    - Checksum
uchar server_recieve_command();

// Recebe um commando e tenta andar no board
// Situacoes:
	// Andou para local vazio
		// Anda no board
		// Seta a resposta para ok
	// Andou para local com tesouro
		// Anda no board
		// Seta a resposta para o tipo do tesouro
	// Andou para local invalido
		// Seta a resposta para ack
// Retorno:
	// Path do tesouro
	// Se nao encontrou tesouro
		// Retorna NULL
char *server_walk(uchar command);

// Imprime o board no estado atual
void server_print_board();

// Imprime a sequencia de passos validos do cliente
void server_print_seq_events();

// Envia ao cliente a resposta montada anteriormente
void server_send_answer(char* path);

#endif
