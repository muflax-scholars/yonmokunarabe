/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#include <stdio.h>
#include <stdlib.h>
#include "ai.h"
#include "board.h"
#include "common.h"
#include "hash.h"

unsigned long ai_counter = 0; /* Steps the AI took to solve a board. */

long move_scores[MAX_TURNS][MAX_COLS]; /* Contains score for each column for 
                                          each depth. */ 

/* Solves board from scratch, prints result. */
board_state solve(board *board)
{
    board_state res;
        
    printf("Solving %dx%d board now.\n", board->size->x, board->size->y);
    print_board(board);
    
    init_ai(board);
    
    printf("Solving...\n");
    res = alpha_beta(board, LOSE, WIN);
    printf("Done. Took %lu steps.\n", ai_counter);
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
    board_state temp   = UNKNOWN;
    board_state res    = UNKNOWN;
    board_state hash   = UNKNOWN;
    int threat         = -1;
    int possible_moves = 0;
    int i, j;
    int reordered_moves[MAX_COLS]; /* Contains columns to check. */
#if AI_DEBUG == 1
    long n;
#endif

    ai_counter += 1;
                
#if AI_DEBUG == 1
    if (board->turn <= DEBUG_DEPTH) {
        n = ai_counter;
        printf("Starting alpha-beta #%d...\n", n);
        printf("Alpha: %d, beta: %d.\n", alpha, beta);
        print_board(board);
    }
#endif

    /* Check if the game is already over. */
    if (board->turn >= board->max_turns) {
        return DRAW;
    }

    /* Check if a solution is available in the hash. */
    hash = get_hash(board);
#if AI_DEBUG == 1
    if (board->turn <= DEBUG_DEPTH) {
        printf("Hash: %d\n", hash);
    }
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
    
    /* Prepare re-ordered moves. */
    for (i = 0; i < board->size->x; i++) {
        reordered_moves[i] = i;
    }
    /* Don't reorder moves near the end. This just introduces noise. */
    if (board->turn <= REORDER_DEPTH) {
        reorder_moves(board, reordered_moves);
    }
#if AI_DEBUG == 1
    if (board->turn <= DEBUG_DEPTH) {
        printf("Reordered: ");
        for (i = 0; i < board->size->x; i++) {
            printf("%d ", reordered_moves[i]);
        }
        printf("\n");
    }
#endif
    
#if AI_DEBUG == 1
    if (board->turn <= DEBUG_DEPTH) {
        printf("Checking for threats and winning moves...\n");
    }
#endif
    /* Detect all threats and winning moves. If there is more than 1 threat, 
     * the board is lost. */
    for (j = 0; j < board->size->x; j++) {
        i = reordered_moves[j];
        if (column_free(board, i)) {
            /* Note number of available moves for later. */
            possible_moves += 1;

#if AI_DEBUG == 1
            if (board->turn <= DEBUG_DEPTH) {
                printf("Threat on %d?\n", i);
            }
#endif
            /* Threat? Once there are already 2 threats, don't check for 
             * more. */
            if (threat != -2) {
                fast_move(board, i, board->player^1);
                if (has_won(board, board->player^1)) {
#if AI_DEBUG == 1
                    if (board->turn <= DEBUG_DEPTH) {
                        printf("Threat found: %d\n", i);
                        print_board(board);
                    }
#endif
                    if (threat == -1) {
                        threat = i;
                    } else { 
                        /* Found another threat, so unless there is still an instant
                         * victory, we lost. */
                        threat = -2;
                    }
                }
                fast_undo(board, i, board->player^1);
            }

#if AI_DEBUG == 1
            if (board->turn <= DEBUG_DEPTH) {
                printf("Winning move on %d?\n", i);
            }
#endif
            /* winning move? */
            fast_move(board, i, board->player);
            if (has_won(board, board->player)) {
#if AI_DEBUG == 1
                if (board->turn <= DEBUG_DEPTH) {
                    printf("Winning move found: %d\n", i);
                }
#endif
                fast_undo(board, i, board->player);
                return set_hash(board, WIN);
            }
            fast_undo(board, i, board->player);
        }
    }

    if (threat == -2) {
        /* More than 1 threat, so we lost. */
        res = LOSE;
    } else if (threat > -1) {
        /* There is a threat, so act against it. */
#if AI_DEBUG == 1
        if (board->turn <= DEBUG_DEPTH) {
            printf("Acting on threat...\n");
        }
#endif
        move(board, threat);
        temp = -alpha_beta(board, -beta, -alpha);
        /* Improve score. */
        res = max(res, temp);
        alpha = max(res, alpha);
#if AI_DEBUG == 1
        if (board->turn <= DEBUG_DEPTH) {
            printf("Got back in %d: %d (res: %d, alpha: %d)\n", 
                   n, temp, res, alpha); 
        }
#endif
        undo(board, 1);
    } else { 
        /* No threat, so try all possible moves. */
#if AI_DEBUG == 1
        if (board->turn <= DEBUG_DEPTH) {
            printf("Testing all %d moves...\n", possible_moves);
        }
#endif
        for (j = 0; j < board->size->x; j++) {
            i = reordered_moves[j];
            if (column_free(board, i)) {
                move(board, i);
                temp = -alpha_beta(board, -beta, -alpha);
                /* Improve score. */
                res = max(res, temp);
                alpha = max(res, alpha);
#if AI_DEBUG == 1
                if (board->turn <= DEBUG_DEPTH) {
                    printf("Got back in %d: %d (res: %d, alpha: %d)\n", 
                            n, temp, res, alpha); 
                }
#endif
                undo(board, 1);
                possible_moves -= 1;

                if (alpha >= beta) { /* cut-off */
                    /* A low beta may hide a successful WIN, which doesn't
                     * matter this time, but if we saved it like this, the hash
                     * would be wrong, so correct for this. */
                    if (possible_moves > 0) {
                        /* Reward columns with cut-offs, but only until a 
                         * certain depth. */
                        if (board->turn <= REORDER_DEPTH) {
                            score_move(board, i);
                        }
                        if (res == DRAW) {
                            res = MAYBE_WIN;
                        }
                    }
#if AI_DEBUG == 1
                    if (board->turn <= DEBUG_DEPTH) {
                        printf("Cut-off: %d\n", res);
                    }
#endif
                    goto ab_end;
                }
            }
        }
    }

    ab_end:
    /* Improve score through hash if we have MAYBE_WIN and MAYBE_LOSE at the
     * same time. */
    if (res == -hash) {
        res = DRAW;
    }

#if AI_DEBUG == 1
    if (board->turn <= DEBUG_DEPTH) {
        printf("Res from #%d: %d\n", n, res);
    }
#endif
    return set_hash(board, res);
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
    
    printf("Recommending move on %dx%d board now.\n", 
           board->size->x, board->size->y);
    print_board(board);
    
    init_ai(board);
    
    printf("Solving...\n");
    for (i = 0; i < board->size->x; i++) {
        if (column_free(board, i)) {
            move(board, i);
            if (has_won(board, board->player^1)) {
                undo(board, 1);
                best_move = i;
#if AI_DEBUG == 1
                printf("Best move through winning: %d.\n", best_move);
#endif
                goto best_move_end;
            }

            res = -alpha_beta(board, -beta, -alpha);
#if AI_DEBUG == 1
            printf("Move %d would lead to: %d.\n", i, res);
#endif
            undo(board, 1);

            if (res >= beta) {
                best_move = i;
#if AI_DEBUG == 1
                printf("Best move through beta cut-off: %d.\n", best_move);
#endif
                goto best_move_end;
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
    best_move_end:
    printf("Done. Took %lu steps.\n", ai_counter);
    print_hash_stats();
    printf("Result: %d\n", best_move);
    return best_move;
}

/* Initialize move reordering for given board size. */
void init_reorder(board_size *size)
{
    int i, j, s;

    printf("Initializing move order history...\n");
    for (i = 0; i < size->x; i++) {
        for (j = 0; j < MAX_TURNS; j++) {
            s = min(i, size->x - i - 1);
            move_scores[j][i] = s;
        }
    }
}

/* This is an (ugly) alternative to qsort_r() or nested functions. Sure,
 * gcc+glibc support both, but that would break my imposed standard compliance.
 * As re-entrance and thread-safety are irrelevant here anyway, the code should
 * be fine. */
static int _depth = 0; /* Used to pass an argument into move_cmp(). */

/* Comparison function for sorting moves. */
static int move_cmp(const void *a, const void *b) 
{ 
    const int *ia = (const int *)a; // casting pointer types 
    const int *ib = (const int *)b;
    return move_scores[_depth][*ib] - move_scores[_depth][*ia];
} 

/* Sorts moves according to scores. */
void reorder_moves(board *board, int moves[])
{
    _depth = board->turn;
    qsort(moves, board->size->x, sizeof(int), move_cmp);
}

/* Adjust score for given column. */
void score_move(board *board, int col)
{
    move_scores[board->turn][col] += 1;
}
    
/* Initialize everything needed for AI operation. */
void init_ai(board *board)
{
    ai_counter = 0;
    init_hash();
    init_reorder(board->size);
}
