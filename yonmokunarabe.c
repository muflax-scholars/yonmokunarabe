/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 *
 * 四目並べ   
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef __GNU_LIBRARY__
#include <getopt.h> /* for long options */
#else
#include <unistd.h>
#endif
#include "ai.h"
#include "board.h"
#include "common.h"
#include "yonmokunarabe.h"

/* Global variables. */
short verbose = 0;

/* Prints usage. */
static void usage() 
{
    printf("usage: yonmokunarabe [options] [mode]\n"
           "options:\n"
           "\t-h --help        print help (this text)\n"
           "\t-v --verbose     be verbose\n"
           "modes:\n"
           "\t-s --solve WxH   solve board of size WxH and print result\n"
           );
    exit(1);
}

/* Parses arg and sets board_size accordingly. */
void parse_size(char* arg, board_size* size)
{
    char *end;
    size->x = (int) strtol(arg, &end, 10); 
    if (*end != '\0')
        size->y = (int) strtol(end+1, NULL, 10);
    if (size->x == 0 || size->y == 0) {
        printf("Invalid size. Use WIDTHxHEIGHT, like 7x6.\n");
        usage();
    }
}

int main (int argc, char * const argv[])
{
    int c;
    enum modes mode = MODE_NONE;
    board_size size;
    board board;
#ifdef __GNU_LIBRARY__
    int option_index;
    static struct option long_options[] = {
        {"verbose",      no_argument,       0, 'v'},
        {"help",         no_argument,       0, 'h'},
        {"solve",        required_argument, 0, 's'},
        {0, 0, 0, 0}
    };
    
    while ((c = getopt_long(argc, argv, "hvs:", long_options, &option_index)) != -1) {
#else
    while ((c = getopt(argc, argv, "hvs:")) != -1) {
#endif     
        switch (c) {
           case 'v':
             verbose = 1;
             break;
           case 's':
             mode = MODE_SOLVE;
             parse_size(optarg, &size);
             break;
           case 'h':
           case '?':
             usage();
           default:
             abort();
        }
    }
    
    /* Start operation. */
    switch (mode) {
        case MODE_NONE:
            usage();
            break;
        case MODE_SOLVE:
            init_board(&board, &size);
            solve(&board);
            destroy_board(&board);
            break;
        default:
            abort();
    }
    return 0;
}
