/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#ifndef YONMOKUNARABE_HASH_H
#define YONMOKUNARABE_HASH_H

#include "ai.h"
#include "board.h"

#define HASHSIZE 65535

typedef struct hash_node {
	uint64_t board;
	board_state res;
	struct hash_node *next;
} hash_node;

void init_hash();
board_state get_hash(board *board);
board_state set_hash(board *board, board_state res);

#endif /* end of include guard: YONMOKUNARABE_HASH_H */

