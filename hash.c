/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ai.h"
#include "board.h"
#include "hash.h"

/* What, you need more than one hash? Pff. */
hash_node *hash[HASHSIZE];

unsigned int hash_counter = 0; /* How many slots of the hash are used? */
unsigned int col_counter  = 0; /* How many collisions happened? */
unsigned int miss_counter = 0; /* How many entries couldn't be found? */

/* Return result from hash. */
board_state get_hash(board *board)
{
	hash_node *node;
	
#if HASH_CUT_OFF > -1
	/*Skip hashs if recalculation would be faster.*/
	if (board->turn > HASH_CUT_OFF) {
		return UNKNOWN;
	}
#endif

#if HASH_REPLACE == 0
	/* Collisions are saved in a linked list. */
	node = hash[board->hash % HASHSIZE];
	while (node != NULL) {
		if (node->bitmap[WHITE] == board->bitmap[WHITE] &&
			node->bitmap[BLACK] == board->bitmap[BLACK]) { /* hash found */
			return node->res;
		} else { /* check other nodes */
			node = node->next;
		}
	}
#else
	/* Collisions replace the old entry. */
	node = hash[board->hash % HASHSIZE];
	if (node != NULL) {
		if (node->bitmap[WHITE] == board->bitmap[WHITE] &&
			node->bitmap[BLACK] == board->bitmap[BLACK]) { /* hash found */
			return node->res;
		}
	}
#endif
	/* not in the hash */
	miss_counter += 1;
    return UNKNOWN;
}

/* Set hash for board. Returns same result again. */
board_state set_hash(board *board, board_state res)
{
	hash_node *node, *new;
	uint64_t board_hash;

#if USE_SYMMETRY == 1
#if SYMMETRY_CUTOFF > -1
	if (board->turn < SYMMETRY_CUTOFF) {
#endif
		if (board->hash > board->sym_hash) {
			board_hash = board->hash;
		} else {
			board_hash = board->sym_hash;
		}
#if SYMMETRY_CUTOFF > -1
	}
#endif
#else
	board_hash = board->hash;
#endif
    
#if HASH_CUT_OFF > -1
	/* Skip hashs if recalculation would be faster. */
	if (board->turn > HASH_CUT_OFF) {
		return res;
	}
#endif

#if HASH_REPLACE == 0
	/* Collisions are saved in a linked list. */

    if ((new = malloc(sizeof(hash_node))) == NULL)
		abort();
	new->bitmap[0] = board->bitmap[0];
	new->bitmap[1] = board->bitmap[1];
	new->res       = res;
	new->next      = NULL;

	node = hash[board_hash % HASHSIZE];
	if (node != NULL) { /* insert node into list */
		col_counter += 1;
		new->next = node;
	} else {
		hash_counter += 1;
	}
	hash[board_hash % HASHSIZE] = new;
#else
	/* Collisions replace the old entry. */
	node = hash[board_hash % HASHSIZE];
	if (node != NULL) { /* replace old node */
		col_counter += 1;
		node->bitmap[0] = board->bitmap[0];
		node->bitmap[1] = board->bitmap[1];
		node->res       = res;
	} else {
		hash_counter += 1;
		if ((new = malloc(sizeof(hash_node))) == NULL)
			abort();
		new->bitmap[0] = board->bitmap[0];
		new->bitmap[1] = board->bitmap[1];
		new->res       = res;
		hash[board_hash % HASHSIZE] = new;
	}
#endif
	/* Return same result regardlass of hash. */
    return res;
}

/* Initializes hash. Also call this whenever the board size changes. */
void init_hash()
{
	int i;
	hash_node *node;
#if HASH_REPLACE == 0
	hash_node *next;
#endif
	
    printf("Initializing hash (%lu bytes)...\n", HASHSIZE*sizeof(hash_node));

    hash_counter = col_counter = miss_counter = 0;

	for (i = 0; i < HASHSIZE; i++) {
		node = hash[i];
		while (node != NULL) {
#if HASH_REPLACE == 0
			next = node->next;
			free(node);
			node = next;
#else
			free(node);
#endif
		}
		hash[i] = NULL;
	} 
}

/* Prints hash stats. */
void print_hash_stats()
{
	printf("Hash entries: %d, collision: %d, misses: %d, "
		   "collision percentage: %d%%, used: %d%%.\n",
		   hash_counter, col_counter, miss_counter,
		   col_counter*100 / (hash_counter > 0 ? hash_counter : 1), 
		   (hash_counter)*100 / HASHSIZE);
		
}
