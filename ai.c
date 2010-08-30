/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#include <stdio.h>
#include "ai.h"
#include "board.h"
#include "common.h"
#include "hash.h"

unsigned int ai_counter = 0; /* Steps the AI took to solve a board. */

#define AI_DEBUG 0 /* print AI debug info */

int reordered_moves[16]; /* Contains columns to check in new order. */

/* Solves board from scratch, prints result. */
board_state solve(board *board)
{
    board_state res;
        
    printf("Solving %dx%d board now.\n", board->size->x, board->size->y);
    print_board(board);
    
    init_ai(board);
    
    printf("Solving...\n");
    res = alpha_beta(board, LOSE, WIN);
    printf("Done. Took %d steps.\n", ai_counter);
    print_hash_stats();

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
    int i, j;

    ai_counter += 1;
                
#if AI_DEBUG == 1
    printf("Starting alpha-beta #%d...\n", ai_counter);
    print_board(board);
#endif

    /* Check if the game is already over. */
    if (board->turn >= board->max_turns) {
        return DRAW;
    }

    /* Check if a solution is available in the hash. */
    hash = get_hash(board);
#if AI_DEBUG == 1
    printf("Hash: %d\n", hash);
#endif
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
    
#if AI_DEBUG == 1
    printf("Checking for threats and winning moves...\n");
#endif
    /* Detect all threats and winning moves. If there is more than 1 threat, 
     * the board is lost. */
    for (i = 0; i < board->size->x; i++) {
        if (column_free(board, i)) {
            /* Note number of available moves for later. */
            possible_moves += 1;

#if AI_DEBUG == 1
            printf("Threat on %d?\n", i);
#endif
            /* threat? */
            fast_move(board, i, board->player^1);
            if (has_won(board, board->player^1)) {
#if AI_DEBUG == 1
                printf("Threat found: %d\n", i);
                print_board(board);
#endif
                if (threat == -1) {
                    threat = i;
                } else { /* already another threat */
                    fast_undo(board, i, board->player^1);
                    return set_hash(board, LOSE);
                }
            }
            fast_undo(board, i, board->player^1);

#if AI_DEBUG == 1
            printf("Winning move on %d?\n", i);
#endif
            /* winning move? */
            fast_move(board, i, board->player);
            if (has_won(board, board->player)) {
#if AI_DEBUG == 1
                printf("Winning move found: %d\n", i);
#endif
                fast_undo(board, i, board->player);
                return set_hash(board, WIN);
            }
            fast_undo(board, i, board->player);
        }
    }

    /* There is a threat, so act against it. */
    if (threat != -1) {
#if AI_DEBUG == 1
        printf("Acting on threat...\n");
#endif
        move(board, threat);
        res = -alpha_beta(board, -beta, -alpha);
        undo(board, 1);

        if (res >= beta) {
            return set_hash(board, beta);
        } else {
            return set_hash(board, res);
        }
    } else { /* No threat, so try all possible moves. */
#if AI_DEBUG == 1
        printf("Testing all moves...\n");
#endif
        /* Iterate over re-ordered moves. */
        for (j = 0; j < board->size->x; j++) {
            i = reordered_moves[j];
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
#if AI_DEBUG == 1
                        printf("Cut-off: %d\n", res);
#endif
                        return set_hash(board, res);
                    }
                    alpha = res;
                }
            }
        }
#if AI_DEBUG == 1
        printf("Res: %d\n", alpha);
#endif
        return set_hash(board, alpha);
    }
}

/* Recommend the next move. 
 * Returns the column or -1 if no good move was found. */
int recommend_move(board *board)
{
    int i, j;
    int best_move     = -1;
    board_state alpha = LOSE;
    board_state beta  = WIN;
    board_state res   = UNKNOWN; 
    
    printf("Recommending move on %dx%d board now.\n", 
           board->size->x, board->size->y);
    print_board(board);
    
    init_ai(board);
    
    printf("Solving...\n");
    for (j = 0; j < board->size->x; j++) {
        /* Iterate over re-ordered moves. */
        i = reordered_moves[j];
        if (column_free(board, i)) {
            move(board, i);
            if (has_won(board, board->player^1)) {
                undo(board, 1);
                best_move = i;
#if AI_DEBUG == 1
                printf("Best move through winning: %d.\n", best_move);
#endif
                goto end;
            }

            res = -alpha_beta(board, -beta, -alpha);
            undo(board, 1);

            if (res >= beta) {
                best_move = i;
#if AI_DEBUG == 1
                printf("Best move through beta cut-off: %d.\n", best_move);
#endif
                goto end;
            }

            if (res > alpha) {
                alpha     = res;
                best_move = i;
            }
        }
    }
#if AI_DEBUG == 1
    printf("Best move through brute-forcing: %d.\n", best_move);
#endif
    end:
    printf("Done. Took %d steps.\n", ai_counter);
    print_hash_stats();
    printf("Result: %d\n", best_move);
    return best_move;
}

/* Initialize move reordering for given board size. */
void init_reorder(board_size *size)
{
    int i;

    printf("Initializing move order history...\n");
    /* #TODO: start from center */
    for (i = 0; i < size->x; i++) {
        reordered_moves[i] = i;
    }
}

    
/* Initialize everything needed for AI operation. */
void init_ai(board *board)
{
    ai_counter = 0;
    init_hash();
    init_reorder(board->size);
}
