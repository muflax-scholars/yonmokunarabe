/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#ifndef YONMOKUNARABE_AI_H
#define YONMOKUNARABE_AI_H

#include "board.h"

typedef enum { 
    UNKNOWN    = -3,
    LOSE       = -2,
    MAYBE_LOSE = -1,
    DRAW       = 0,
    MAYBE_WIN  = 1,
    WIN        = 2
} board_state;

board_state solve(board *board);
board_state alpha_beta(board *board, board_state alpha, board_state beta);
int recommend_move(board *board);

#endif /* end of include guard: YONMOKUNARABE_AI_H */

