/* Copyright muflax <mail@muflax.com>, 2010
 * License: GNU GPL 3 <http://www.gnu.org/copyleft/gpl.html>
 */

#ifndef YONMOKUNARABE_H
#define YONMOKUNARABE_H

/* Different modes of operation. */
enum modes { 
    MODE_NONE,
    MODE_SOLVE,
    MODE_RECOMMEND
};

void usage(); 
char *parse_size(char* arg, board_size* size);

#endif /* end of include guard: YONMOKUNARABE_H */

