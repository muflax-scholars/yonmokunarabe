/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#include <stdio.h>
#include "ai.h"
#include "board.h"
#include "common.h"
#include "hash.h"

unsigned int ai_counter = 0; /* Steps the AI took to solve a board. */

/* Solves board from scratch, prints result. */
board_state solve(board *board)
{
    board_state res;
        
    printf("Solving %dx%d board now.\n", board->size->x, board->size->y);
    print_board(board);
    
    ai_counter = 0;
    init_hash();
    
    printf("Solving...\n");
    res = alpha_beta(board, LOSE, WIN);
    printf("Done. Took %d steps.\n", ai_counter);

    printf("Result: ");
    switch (res) {
        case LOSE:
            printf("lose");
            break;
        case DRAW:
            printf("draw");
            break;
        case WIN:
            printf("win");
            break;
        default:
            printf("shit blew up :<");
            break;
    }
    printf(".\n");
    return res;
}

/* Alpha-beta search, returns result. */
board_state alpha_beta(board *board, board_state alpha, board_state beta)
{
    board_state hash, res;
    int threat = -1;
    int possible_moves = 0;
    int i;

    ai_counter += 1;

    /* Check if the game is already over. */
    if (board->turn >= board->max_turns) {
        return DRAW;
    }

    /* Check if a solution is available in the hash. */
    hash = get_hash(board);
    switch (hash) {
        /* The hash may not be accurate. If it is not, use it to improve our
         * boundaries. If that already allows us to make an accurate assessment,
         * use it. */
        case WIN:
        case LOSE:
        case DRAW:
            return hash;
        case MAYBE_LOSE:
            beta = DRAW;
            if (alpha >= beta) {
                return hash;
            }
            break;
        case MAYBE_WIN:
            alpha = DRAW;
            if (alpha >= beta) {
                return hash;
            }
            break;
        case UNKNOWN:
        default:
            /* do nothing */
            break;
    }
    
    /* Detect all threats and winning moves. If there is more than 1 threat, 
     * the board is lost. */
    for (i = 0; i < board->size->x; i++) {
        if (column_free(board, i)) {
            /* Note number of available moves for later. */
            possible_moves += 1;

            /* threat? */
            fast_move(board, i, board->player^1);
            if (has_won(board, board->player^1)) {
                if (threat == -1) {
                    threat = i;
                } else { /* already another threat */
                    fast_undo(board, i, board->player^1);
                    return set_hash(board, LOSE);
                }
            }
            fast_undo(board, i, board->player^1);

            /* winning move? */
            fast_move(board, i, board->player);
            if (has_won(board, board->player)) {
                fast_undo(board, i, board->player);
                return set_hash(board, WIN);
            }
            fast_undo(board, i, board->player);
        }
    }

    /* There is a threat, so act against it. */
    if (threat != -1) {
        move(board, threat);
        res = -alpha_beta(board, -beta, -alpha);
        undo(board, 1);

        if (res >= beta) {
            return set_hash(board, beta);
        } else {
            return set_hash(board, res);
        }
    } else { /* No threat, so try all possible moves. */
        for (i = 0; i < board->size->x; i++) {
            if (column_free(board, i)) {
                move(board, i);
                res = -alpha_beta(board, -beta, -alpha);
                undo(board, 1);
                possible_moves -= 1;

                if (res > alpha) {
                    if (res >= beta) { /* cut-off */
                        /* It may get better, but this is irrelevant now. */
                        if (res == DRAW && possible_moves > 0) {
                            res = MAYBE_WIN;
                        }
                        return set_hash(board, res);
                    }
                    alpha = res;
                }
            }
        }
        return set_hash(board, alpha);
    }
}

/* Recommend the next move. 
 * Returns the column or -1 if no good move was found. */
int recommend_move(board *board)
{
    int i;
    int best_move     = -1;
    board_state alpha = LOSE;
    board_state beta  = WIN;
    board_state res   = UNKNOWN; 

    for (i = 0; i < board->size->x; i++) {
        if (column_free(board, i)) {
            move(board, i);
            if (has_won(board, board->player^1)) {
                    undo(board, 1);
                    return i;
            }

            res = -alpha_beta(board, -beta, -alpha);
            undo(board, 1);

            if (res >= beta) {
                return i;
            }

            if (res > alpha) {
                alpha     = res;
                best_move = i;
            }
        }
    }
    return i;
}
