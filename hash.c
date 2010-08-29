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

unsigned int hash_counter = 0; /* How many entries are in the hash? */
unsigned int col_counter  = 0; /* How many collisions are in the hash? */

/* Return result from hash. */
board_state get_hash(board *board)
{
	hash_node *node;
	uint64_t bh;

	bh = board_hash(board);
	node = hash[bh % HASHSIZE];
	while (node != NULL) {
		if (node->board == bh) { /* hash found */
			return node->res;
		} else { /* check other nodes */
			node = node->next;
		}
	}
    return UNKNOWN;
}

/* Set hash for board. Returns same result again. */
board_state set_hash(board *board, board_state res)
{
	hash_node *node, *new;
	uint64_t bh;
    
	hash_counter += 1;

	bh = board_hash(board);
    if ((new = malloc(sizeof(hash_node))) == NULL)
		abort();
	new->board = bh;
	new->res   = res;
	new->next  = NULL;

	node = hash[bh % HASHSIZE];
	if (node != NULL) { /* insert node into list */
		col_counter += 1;
		new->next = node;
	}
	hash[bh % HASHSIZE] = new;
    return res;
}

/* Initializes hash. Also call this whenever the board size changes. */
void init_hash()
{
	int i;
	hash_node *node, *next;
	
    printf("Initializing hash (%lu bytes)...\n", HASHSIZE*sizeof(hash_node));

    hash_counter = col_counter = 0;

	for (i = 0; i < HASHSIZE; i++) {
		node = hash[i];
		while (node != NULL) {
			next = node->next;
			free(node);
			node = next;
		}
		hash[i] = NULL;
	} 
}
