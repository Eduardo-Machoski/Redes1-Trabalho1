#ifndef __PROTOCOL__
#define __PROTOCOL__
	#include <stdbool.h>
	
	//===============================TIPOS DE MENSAGEM========================================
	#define ACK 			0
	#define NACK			1
	#define OK				2
	#define TAMANHO			4
	#define DADOS			5
	#define TEXTO			6
	#define VIDEO			7
	#define IMAGEM			8
	#define FIM_FILE		9
	#define DIREITA			10
	#define CIMA			11
	#define BAIXO			12
	#define ESQUERDA		13
	#define ERRO			15

	//================================CODIGOS DE ERRO=========================================

	#define NO_PERMISSION 	0
	#define NO_SPACE		1


	//=====================================DEMAIS=============================================

	typedef unsigned char uchar;
	#define INIT_SEQUENCE  	0b01111110
	#define PATH_INTERFACE 	"enp43s0"

	//================================STRUCT DE PACOTE========================================

	typedef struct package_t{
		uchar buffer[131];	
		uchar size;
		uchar seq;
		uchar type;
		uchar data[127];
		uchar buffer_size;
	} package_t;

	//==============================FUNÇÕES DO PROTOCOLO======================================

	void protocol_init(char* interface_name);

	int protocol_send_package(package_t *package, bool inc);

	void protocol_recieve_passive(package_t *package);
	
	void protocol_recieve_active(package_t *package);

#endif
