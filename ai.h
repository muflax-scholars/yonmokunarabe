/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#ifndef YONMOKUNARABE_AI_H
#define YONMOKUNARABE_AI_H

#include "board.h"
                              
#define AI_DEBUG 0 /* print AI debug info */
#define DEBUG_DEPTH 10 /* don't print info after that depth */

/* Currently, there can only be at most 16 columns and thus 60 turns. */
#define MAX_COLS  16
#define MAX_TURNS 60

#define REORDER_DEPTH 10 /* Moves are only reordered until this depth. 
							Set to 0 to (kinda) disable reordering. Typically,
							the last turns aren't worth the sorting trouble. */

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

