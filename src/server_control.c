#include "server_control.h"

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>

//=====================VARIAVEIS GLOBAIS==============================    ==

board g_board;								// Estado atual do tabuleiro

package_t send_package;					// Pacote a ser enviado 

package_t received_package;			// Ultimo pacote recebido

treasure_t treasures[TREASURES];    // Vetor com os tesouros e suas informacoes

char *extensions[3] = {"jpg", "mp4", "txt"};// Vetor com toda as extensões possiveis

uchar plays[1024];						// Vetor com a sequencia de jogadas do player
int num_plays;								//	Numero de jogadas que o player realizou

//===========================FUNCOES INTERNAS===================================

// Salva uma jogada do player na ultima posicao
void local_save_play(uchar type){
	plays[num_plays] = type;
	num_plays++;
}

// Imprime todas as jogadas do player em sequencia
void local_print_play_sequence(){
	for(int i = 0; i < num_plays ; i++){
		switch (plays[i]){
			case ESQUERDA:
				printf("a");
				break;
				
			case DIREITA:
				printf("d");
				break;
				
			case CIMA:
				printf("w");
				break;
				
			case BAIXO:
				printf("s");
				break;
				
			default:
				break;
		}

		if(i == num_plays - 1)
			printf("\n");
		else
			printf(" ");
	}
	printf("\n");
}

void local_build_send_package(uchar type, char *data){
	#ifdef DEBUG
	printf("Local_build_send_package build: %u\n", type);
	#endif

	switch (type){
		// Tipos sem dados enviados
		case ACK:
		case NACK:
		case OK:
		case FIM_FILE:
			send_package.type = type;
			send_package.size = 0;
		break;

		// Envia o nome do arquivo alem do tipo dele
		case TREASURE_FOUND:
			if (!strcmp(data+10, "jpg")) send_package.type = IMAGEM;
			else if (!strcmp(data+10, "mp4")) send_package.type = VIDEO;
			else if (!strcmp(data+10, "txt")) send_package.type = TEXTO;

			// Tamanho padrao dos nomes de arquivos do jogo
			send_package.size = 17;

			// Coloca o nome do tesouro no send_package
			for(int i = 0; i < 17; i++)
				send_package.data[i] = data[i];

		break;


		case TAMANHO:
			send_package.type = type;
			

			// Tem que fazer o sistema de arquivos e dai atualizar aqui
			send_package.data[0] = '1';
			send_package.size = 1;
		break;

		case ERRO_CONEXAO:
			send_package.type = type;

			send_package.data[0] = ERRO_TIPO;
			send_package.size = 1;
		break;

		default:
			perror("Erro type local_build_send_package");
			exit (1);
		break;
	}
}

// Verifica se a posicao atual do player possui um tesouro
// Retorno:
	// NULL - Posicao atual nao tem tesouro
	// Path do tesouro - Posicao atual tem um tesouro
char *local_treasure_found(){
	for(int i = 0; i < TREASURES; i++){
		if(!treasures[i].found)
			if(g_board.player_x == treasures[i].x && g_board.player_y == treasures[i].y)
			{
				treasures[i].found = true;
				return treasures[i].path;
			}
	}

	return NULL;
}

// Descobre o caminho para cada tesouro 
int local_get_file_path(int index, char *s) {
    FILE *file = NULL;

    for (int i = 0; i < 3; i++) {
        snprintf(s, 14, "objetos/%d.%s", index, extensions[i]);
		file = fopen(s, "r");
        if (file != NULL) {
            fclose(file);
            return 1;  // Encontrou
        }
    }

    s[0] = '\0';  // Não encontrou
    return 0;
}

// Cria um tesouro em uma posicao vazia do tabuleiro
void local_init_treasure(int index){
	bool valid;

	// Garante que a posicao criada pelo tesouro eh vazia
	do{
		valid = true;

		treasures[index].x = rand() % BOARDSIZE;
		treasures[index].y = rand() % BOARDSIZE;

		// Posicao 0-0 nao eh valida
		if(treasures[index].x == 0 && treasures[index].y == 0)
			valid = false;
			
		// Verifica se nao ha outro tesouro nessa posicao
		for(int i = 0; i < index; i++)
			if(treasures[index].x == treasures[i].x && treasures[index].y == treasures[i].y)
				valid = false;
			
	} while (!valid);


	// Inicializa o resto do tesouro
	treasures[index].found = false;

	// Acha o caminho do tesouro
	if (!local_get_file_path(index + 1, treasures[index].path)){
		printf("Devem ter tesouros nomeados de 1 a 8 com extensão .txt, .jpg ou .mp4 no diretório objetos\n");
		exit(1);
	}
}

//===========================FUNCOES EXTERNAS=========================    ==========

// Inicializa o servidor e seus componentes
	// - Tabuleiro
	// - Tesouros
	// - Pacotes de mensagens
	// - Conexao com o cliente
void server_init(){
	
	// Inicializa o tabuleiro
	for(int i = 0; i < BOARDSIZE; i++)
		for(int j = 0; j < BOARDSIZE; j++)
			g_board.board[i][j] = ' ';

	// Posicao inicial do player
	g_board.board[0][0] = '0';

	g_board.player_x = 0;
	g_board.player_y = 0;

	// Numero de jogadas inicial do player
	num_plays = 0;

	// Inicializa os tesouros
	for(int i = 0; i < TREASURES; i++){
		local_init_treasure(i);
		
		// Insere as posicoes de cada tesouro no tabuleiro
		g_board.board[treasures[i].y][treasures[i].x] = i + '1';
	}
	
	// Configura o socket de comunicacao
	protocol_init(PATH_INTERFACE);
}

// Espera passivamente por um pacoteenviado do cliente
// Retorno:
	// Tipo do pacote recebido
uchar server_receive_command(){
	protocol_receive_passive(&received_package);

	switch (received_package.type){
		// Valores validos
		case ESQUERDA:
		case DIREITA:
		case CIMA:
		case BAIXO:
			local_save_play(received_package.type);
			local_print_play_sequence();
			break;

		// Tipo invalido
		default:
			local_build_send_package(ERRO_CONEXAO, NULL);
			exit(ERRO_TIPO);
			break;
	}

	return received_package.type;
}

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
char *server_walk(uchar command){

	bool walk_fail = false;

	g_board.board[g_board.player_y][g_board.player_x] = ' ';

	switch (command){
		case CIMA:
				if(g_board.player_y == BOARDSIZE - 1)
					walk_fail = true;
				else
					g_board.player_y += 1;
			break;
		case BAIXO:
				if(g_board.player_y == 0)
					walk_fail = true;
				else
					g_board.player_y -= 1;
			break;
		case ESQUERDA:
				if(g_board.player_x == 0)
					walk_fail = true;
				else
					g_board.player_x -= 1;
			break;	
		case DIREITA:
				if(g_board.player_x == BOARDSIZE - 1)
					walk_fail = true;
				else
					g_board.player_x += 1;
			break;
		default:
			perror("Erro comando server_walk");
			exit(1);
			break;
	}

	g_board.board[g_board.player_y][g_board.player_x] = '0';

	// Player tentou andar para fora do tabuleiro
	if(walk_fail){
		local_build_send_package(OK, NULL);
		return NULL;
	}

	// Verifica se encontrou um tesouro
	char *path = local_treasure_found();

	if(!path)
		local_build_send_package(ACK, path);
	else
		local_build_send_package(TREASURE_FOUND, path);

	return path;
}

// Imprime o board no estado atual
void server_print_board(){
	// Tabuleiro 'desconhecido'
   for(int i = BOARDSIZE - 1; i >= 0; i--){
      for(int j = 0; j < BOARDSIZE; j++)
         printf("%c", g_board.board[i][j]);
      printf("\n");
   }
   printf("\n");
}

// Imprime a sequencia de passos validos do cliente
void server_print_seq_events(){

}

// Envia arquivo
void local_send_file(char *path){
	struct stat st;

	FILE *file = fopen(path, "r");

	// Espera ACK
	protocol_receive_active(&received_package);

	// Descobre informações do arquivo
	stat(path, &st);

	// Monta tamanho como unsigned e coloca em 8 bytes de dados
	uint64_t size = (uint64_t) st.st_size;
	for (int i=0; i<8; i++)
		send_package.data[i] = (size >> (56 - i * 8)) & 0xFF;
	send_package.size = 8;
	send_package.type = TAMANHO;

	// Envia tamanho  e espera ACK ou ERRO
	protocol_send_package(&send_package, true);
	protocol_receive_active(&received_package);

	// Caso receba erro
	if (received_package.type == ERRO){
		return;
	}

	// Envia arquivo
	size_t bytes_read;
	while ((bytes_read = fread(send_package.data, 1, 127, file)) > 0){
		send_package.size = bytes_read;
		send_package.type = DADOS;
		protocol_send_package(&send_package, true);
		protocol_receive_active(&received_package);
	}

	// Envia FIM_FILE
	send_package.size = 0;
	send_package.type = FIM_FILE;
	protocol_send_package(&send_package, true);
}

// Envia resposta ao cliente
void server_send_answer(char *path){

	// Manda ultimo pacote montado
	protocol_send_package(&send_package, true);

	// Manda arquivo
	if (path) local_send_file(path);
}
