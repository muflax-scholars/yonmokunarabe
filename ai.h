/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#ifndef YONMOKUNARABE_AI_H
#define YONMOKUNARABE_AI_H

#include "board.h"
                              
/* Currently, there can only be at most 16 columns and thus 60 turns. */
#define MAX_COLS  16
#define MAX_TURNS 60

typedef enum { 
    UNKNOWN    = -3,
    LOSE       = -2,
    MAYBE_LOSE = -1,
    DRAW       = 0,
    MAYBE_WIN  = 1,
    WIN        = 2
} board_state;

board_state solve(board *board);
int recommend_move(board *board);
void init_ai(board *board);
void init_reorder(board_size *size);
board_state alpha_beta(board *board, board_state alpha, board_state beta);
void reorder_moves(board *board, int moves[]);
void score_move(board *board, int col);

#endif /* end of include guard: YONMOKUNARABE_AI_H */

