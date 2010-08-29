/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "common.h"

/* Initialize board. Just allocate and pass the args. */
void init_board(board *board, board_size *size)
{
    board->size          = size;
    board->player        = WHITE;
    board->turn          = 0;
    board->bitmap[WHITE] = 0;
    board->bitmap[BLACK] = 0;
    
    if ((board->height_map = malloc(sizeof(int) * board->size->x)) == NULL)
        abort();
    if ((board->history = malloc(sizeof(int) * 1)) == NULL)
        abort();
    board->history[0]    = '\0';
}

/* Free board and all associated structures. This includes the size! */
void free_board(board *board)
{
    free(board->size);
    free(board->height_map);
    free(board->history);
    free(board);
}

/* Return bit from bitmap matching coordinates x, y. */
int bitpos(board *board, int x, int y)
{
    return (1 << (x * (board->size->y + 1) + y));
}

/* Return true if position is blocked by either player. */
int blocked(board *board, int x, int y)
{
    uint64_t bit;
    bit = bitpos(board, x, y);
    return ((board->bitmap[WHITE] & bit)
            || (board->bitmap[BLACK] & bit));
}

/* Return true if position is blocked by given player. */
int blocked_by(board *board, int x, int y, players player)
{
    uint64_t bit;
    bit = bitpos(board, x, y);
    return (board->bitmap[player] & bit);
}

/* Pretty-print board plus some stats. */
void print_board(board *board)
{
    int c, i, x, y;
    
    if (board == NULL || board->size == NULL) {
        printf("Uninintialized board.\n");
    }

    for (y = board->size->y -1; y >= 0; y--) {
        for (x = 0; x < board->size->x; x++) {
            if (blocked_by(board, x, y, WHITE)) {
                printf("W");
            } else if (blocked_by(board, x, y, BLACK)) {
                printf("B");
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
    printf("turn: %d, player: %c, history: ", board->turn, "WB"[board->player]);
    i = 0;
    while ((c = board->history[i++]) != '\0') {
        printf("%c", c + '0');
    }
    printf("\n");
}

/* Make move in given column. 
 * Returns -1 if a move was illegal, 0 otherwise. */
int move(board *board, int col)
{
    uint64_t bit;

    if (col < 0 || col >= board->size->x) {
        if (verbose)
            printf("Illegal move attempted: out of bounds.\n");
        return -1;
    }

    if (board->height_map[col] < board->size->y) {
        /* move */
        bit = bitpos(board, col, board->height_map[col]);
        board->bitmap[board->player] ^= bit;
        board->height_map[col] += 1;
        board->player          ^= 1;
        board->turn            += 1;
        if ((board->history = realloc(board->history, 
                                      sizeof(int) * (board->turn + 1))
            ) == NULL) {
            abort();
        } else {
            board->history[board->turn] = col;
            board->history[board->turn + 1] = '\0';
        }
        return 0;
    } else {
        if (verbose)
            printf("Illegal move attempted: columns already full.\n");
        return -1;
    }
}

/* Faster version of move(), without sanity checks or bookkeeping. Always undo
 * this via fast_undo() afterwards! Used for threat detection.
 */
void fast_move(board *board, int col)
{
    uint64_t bit;
    bit = bitpos(board, col, board->height_map[col]);
    board->bitmap[board->player] ^= bit;
}

/* Faster version of undo(), to be used with fast_move(). 
 * Used for threat detection.
 */
void fast_undo(board *board, int col)
{
    /* Thanks to XOR, currently identical to fast_move(). */
    fast_move(board, col);
}

/* Returns 1 if given player has won, 0 otherwise. */
int has_won(board *board, players player)
{
    uint64_t pos, x;

    /* Note: This would be faster if the size were already known at compile
     * time. ;)
     */
    pos = board->bitmap[player]; 
    /* \ */
    x = pos & (pos >> board->size->y);
    if (x & (x >> (2*board->size->y*2)))  
        return 1;
    /* - */
    x = pos & (pos >> (board->size->y+1));
    if (x & (x >> (2*board->size->y*2+1)))  
        return 1;
    /* / */
    x = pos & (pos >> (board->size->y+2));
    if (x & (x >> (2*board->size->y*2+2)))  
        return 1;
    /* | */
    x = pos & (pos >> 1);
    if (x & (x >> 2))  
        return 1;
    
    return 0;
}

/* Undoes last n moves. 
 * Returns -1 if n was illegal, 0 otherwise.
 */
int undo(board *board, int n)
{
    uint64_t bit;
    int col;

    while (n > 0 && board->turn > 0) {
        /* move */
        col = board->history[board->turn];
        bit = bitpos(board, col*(board->size->y+1), board->height_map[col]);
        board->bitmap[board->player] ^= bit;
        board->height_map[col] -= 1;
        board->player          ^= 1;
        board->turn            -= 1;
        if ((board->history = realloc(board->history, 
                                      sizeof(int) * (board->turn + 1))
            ) == NULL) {
            abort();
        } else {
            board->history[board->turn + 1] = '\0';
        }
        return 0;
    }

    if (n != 0) {
        if (verbose)
            printf("Illegal undo attempted.\n");
        return -1;
    }
    return 0;
}

/* Resets board. Like undo. */
int reset(board *board)
{
    return undo(board, board->turn);
}

/* Returns a symmetric hash for the board. */
uint64_t hash(board *board)
{
    int i;
    uint64_t hash = 0;
    for (i = 0; i < board->size->x; i++) {
        hash += 1 << board->height_map[i];
    }
    hash |= board->bitmap[WHITE];
    return hash;
}


