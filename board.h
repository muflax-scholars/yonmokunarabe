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
    board_size *size;
    unsigned short player;
    unsigned int turn;
    uint64_t bitmap[2];         /* occupied positions for each player */
    unsigned int *height_map; /* height of each column */
    unsigned int *history;    /* null-terminated move history */
} board;

typedef enum { 
    WHITE = 0,
    BLACK = 1
} players;

#endif /* end of include guard: YONMOKUNARABE_BOARD_H */

