#ifndef __BOARD__
#define __BOARD__


	typedef unsigned char uchar;

	#define BOARDSIZE 8
	#define TREASURES 8

	typedef struct board{
		 char board[BOARDSIZE][BOARDSIZE] ; // Tabuleiro no seu estado atual
		 unsigned int player_x ;  			   // Posicao x atual do player
		 unsigned int player_y ;   			// Posicao y atual do player
	} board;

#endif
