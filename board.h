/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#ifndef YONMOKUNARABE_BOARD_H
#define YONMOKUNARABE_BOARD_H

#include <stdint.h>

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
uint64_t board_hash(board *board);
void fast_move(board *board, int col, players player);
void fast_undo(board *board, int col, players player);
int column_free(board *board, int col);
void complex_move(board *board, char s[]);

#endif /* end of include guard: YONMOKUNARABE_BOARD_H */

