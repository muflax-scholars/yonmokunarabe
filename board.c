/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "common.h"

/* Initialize board. Just allocate and pass the args. */
void init_board(board *board, board_size *size)
{
    printf("Initializing board (%dx%d)...\n", size->x, size->y);
    board->size          = size;
    board->player        = WHITE;
    board->turn          = 0;
    board->max_turns     = size->x * size->y;
    board->bitmap[WHITE] = 0;
    board->bitmap[BLACK] = 0;
    
    if ((board->height_map = malloc(sizeof(int) * board->size->x)) == NULL)
        abort();
    memset(board->height_map, 0, sizeof(int) * board->size->x);
    if ((board->history = malloc(sizeof(int) * (board->max_turns))) == NULL)
        abort();
    memset(board->history, 0, sizeof(int) * board->max_turns);
}

/* Frees all associated structures within a board so you can free it. 
 * This doesn't include the size! */
void destroy_board(board *board)
{
    printf("Destroying board...\n");
    free(board->height_map);
    free(board->history);
}

/* Return bit from bitmap matching coordinates x, y.
 * Note that there is an intentional free bit between each column to enable fast
 * detection of won games.
 */
uint64_t bitpos(board *board, int x, int y)
{
    return ((uint64_t)1 << (x * (board->size->y+1) + y));
}

/* Return true if position is blocked by either player. */
int blocked(board *board, int x, int y)
{
    uint64_t bit;
    bit = bitpos(board, x, y);
    bit = ( (board->bitmap[WHITE] & bit)
          ||(board->bitmap[BLACK] & bit));
    return (bit != 0);
}

/* Return true if position is blocked by given player. */
int blocked_by(board *board, int x, int y, players player)
{
    uint64_t bit;
    bit = bitpos(board, x, y);
    bit = (board->bitmap[player] & bit);
    return (bit != 0);
}

/* Pretty-print board plus some stats. */
void print_board(board *board)
{
    int i, x, y;
    
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
    for (i=0; i < board->turn; i++) {
        printf("%c", board->history[i] + '0');
    }
    printf("\n");
}

/* Make move in given column. 
 * Returns -1 if a move was illegal, 0 otherwise. */
int move(board *board, int col)
{
    uint64_t bit;

    /*printf("Moving in col %d...\n", col);*/
    /*printf("Before:\n");*/
    /*print_board(board);*/

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
        board->player ^= 1;
        board->history[board->turn] = col;
        board->turn += 1;
        
        /*printf("After:\n");*/
        /*print_board(board);*/
        return 0;
    } else {
        if (verbose)
            printf("Illegal move attempted: columns already full.\n");
        return -1;
    }
}

/* Undoes last n moves. 
 * Returns -1 if n was illegal, 0 otherwise.
 */
int undo(board *board, int n)
{
    uint64_t bit;
    int col;
    
    /*printf("Undoing %d moves...\n", n);*/
    /*printf("Before:\n");*/
    /*print_board(board);*/

    while (n > 0 && board->turn > 0) {
        /* undo */
        col = board->history[board->turn-1];
        board->height_map[col] -= 1;
        board->player ^= 1;
        board->turn -= 1;
        
        bit = bitpos(board, col, board->height_map[col]);
        board->bitmap[board->player] ^= bit;
    
        /*printf("After:\n");*/
        /*print_board(board);*/
        return 0;
    }

    if (n != 0) {
        if (verbose)
            printf("Illegal undo attempted.\n");
        return -1;
    }
    return 0;
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
    if (x & (x >> (2*board->size->y))) {  
        return 1;
    }
    
    /* - */
    x = pos & (pos >> (board->size->y+1));
    if (x & (x >> (2*(board->size->y+1)))) {
        return 1;
    }

    /* / */
    x = pos & (pos >> (board->size->y+2));
    if (x & (x >> (2*(board->size->y+2)))) {
        return 1;
    }
    
    /* | */
    x = pos & (pos >> 1);
    if (x & (x >> 2)) {  
        return 1;
    }
    
    return 0;
}

/* Resets board. Like undo. */
int reset(board *board)
{
    return undo(board, board->turn);
}

/* Returns a symmetric hash for the board. */
uint64_t board_hash(board *board)
{
    int i;
    uint64_t hash = 0;
    for (i = 0; i < board->size->x; i++) {
        hash += 1 << board->height_map[i];
    }
    hash |= board->bitmap[WHITE];
    return hash;
}

/* Returns 1 if column is playable, 0 otherwise. */
int column_free(board *board, int col)
{
    return (board->height_map[col] < board->size->y);
}

/* Faster version of move(), without sanity checks or bookkeeping. Always undo
 * this via fast_undo() afterwards! Used for threat detection.
 */
void fast_move(board *board, int col, players player)
{
    uint64_t bit;
    bit = bitpos(board, col, board->height_map[col]);
    board->bitmap[player] ^= bit;
}

/* Faster version of undo(), to be used with fast_move(). 
 * Used for threat detection.
 */
void fast_undo(board *board, int col, players player)
{
    /* Thanks to XOR, currently identical to fast_move(). */
    fast_move(board, col, player);
}

/* Like move, but allows multiple moves at once. */
void complex_move(board *board, char s[]) {
    int i;
    char c;
    for (i=0; (c=s[i]); i++) {
        move(board, c-'0');
    }
}
