/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#ifndef YONMOKUNARABE_HASH_H
#define YONMOKUNARABE_HASH_H

#include "ai.h"
#include "board.h"

#define HASHSIZE (10 * (1<<(10+10)))   /* Size of internal hash. */
#define HASH_CUT_OFF -1 /* Don't hash boards after that many turns. Set to -1 to
                           disable cut-off or to 0 to disable the hash
                           altogether. A value of around 20 doesn't affect
                           performance much and safes lots of memory so you can
                           turn down HASHSIZE by about 10. */
#define HASH_REPLACE 1  /* Should collisions replace an old slot? Replacing
                           safes plenty of memory and incurs almost no
                           additional misses. */

typedef struct hash_node {
	uint64_t bitmap[2];
	board_state res;
#if HASH_REPLACE == 0
	struct hash_node *next;
#endif
} hash_node;

void init_hash();
board_state get_hash(board *board);
board_state set_hash(board *board, board_state res);
void print_hash_stats();

#endif /* end of include guard: YONMOKUNARABE_HASH_H */

