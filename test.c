/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#include <stdlib.h>
#include <stdio.h>
#include "ai.h"
#include "board.h"
#include "common.h"

/* MinUnit */
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                               if (message) return message; } while (0)
int tests_run = 0;                      

/* Global variables. */
short verbose = 1;

/* Make me a new board. */
#define new_board(X, Y) board board; board_size size; \
                        size.x = X; size.y = Y; init_board(&board, &size);
                                
/* The 3 Fhourstone positions. */
static char* test_fhourstone_1() {
    new_board(7, 6);
    complex_move(&board, "34350556");
    mu_assert("Fhourstone 1 broken.", solve(&board) == WIN);
    return 0;
}
static char* test_fhourstone_2() {
    new_board(7, 6);
    complex_move(&board, "2422460");
    mu_assert("Fhourstone 2 broken.", solve(&board) == LOSE);
    return 0;
}
static char* test_fhourstone_3() {
    new_board(7, 6);
    complex_move(&board, "02222000");
    mu_assert("Fhourstone 3 broken.", solve(&board) == DRAW);
    return 0;
}


/* Run all tests. */
static char* all_tests() {
    mu_run_test(test_fhourstone_1);
    mu_run_test(test_fhourstone_2);
    mu_run_test(test_fhourstone_3);
    return 0;
}

int main (int argc, const char *argv[])
{
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("PASSED.\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
