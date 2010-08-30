/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#ifndef YONMOKUNARABE_BOARD_H
#define YONMOKUNARABE_BOARD_H

#include <stdint.h>

#define USE_SYMMETRY 0 			/* Use additional symmetric hash? This is
								   generally very slow and not worth it. */
#define SYMMETRY_CUTOFF 20      /* Ignore symmetry hash after that many turns.
								   Set to -1 to turn off cut-off. */

typedef struct {
    unsigned int x;
    unsigned int y;
} board_size;

typedef struct {
    board_size *size;          /* size of the board */
    unsigned short player;     /* current player */
    unsigned int turn;         /* current turn */
    unsigned int max_turns;    /* maximal number of playable turns */
    uint64_t bitmap[2];        /* occupied positions for each player */
    unsigned int *height_map;  /* height of each column */
    int *history;              /* move history */
    uint64_t hash;             /* incremental hash */
#if USE_SYMMETRY == 1
    uint64_t sym_hash;         /* symmetrical hash */
#endif
} board;

typedef enum { 
    WHITE = 0,
    BLACK = 1
} players;

void init_board(board *board, board_size *size);
void destroy_board(board *board);
int blocked(board *board, int x, int y);
int blocked_by(board *board, int x, int y, players player);
void print_board(board *board);
int move(board *board, int col);
int has_won(board *board, players player);
int undo(board *board, int n);
int reset(board *board);
void fast_move(board *board, int col, players player);
void fast_undo(board *board, int col, players player);
int column_free(board *board, int col);
void complex_move(board *board, char s[]);
uint64_t zobrist_number(int x, int y, players player);
void init_zobrist();

#endif /* end of include guard: YONMOKUNARABE_BOARD_H */

