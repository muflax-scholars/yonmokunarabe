/* 四目並べのデータベースを作る      *
 * 有り難う御座いますJohn Tromp先生 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEIGHT 6 
#define WIDTH 6
#define PRINTSTEPS 4
#define FULLSTEPS 0
#define SYMCHECK 10
#define MAXDEPTH WIDTH*HEIGHT

/* PRIME >= 2**((H+1)*W-LOCKSIZE) */
#define PRIME 16777291
#define LOCKSIZE (64-24)

#define TOTAL 6
#define WIN 5
#define DRAWORWIN 4 
#define DRAW 3
#define DRAWORLOSE 2
#define LOSE 1
#define NOHASH 0

#define max(A,B) ((A) > (B) ? (A) : (B))

typedef u_int64_t bitboard;
/* system: 
 * X    X     (H+1)*W  ==> buffer for has_won()
 * H   2*H+1    |        \
 * | \  |  \    |        } board
 * 0  (H+2) ... W       /
 */

typedef struct {
    bitboard highlock:LOCKSIZE;
    bitboard newlock:LOCKSIZE;
    unsigned highres:3;
    unsigned newres:3;
    unsigned plies:6;
} entry;

//entry hashtable[PRIME];
entry *hashtable;

/* global variables are used to save loads of memory */
bitboard board[2] = {0, 0}; /* 二人のプレイヤー　*/
int height[WIDTH] = {0}; /* height relief for fast checks */
int moves[HEIGHT*WIDTH] = {0};
int plies = 0; /* := 番 */
bitboard bottom;

unsigned int overwritten = 0;

void init() {
    int i;
    for (i = bottom = 1; i < WIDTH; i++)
        bottom += (bitboard)1<<((HEIGHT+1)*i ); /* 嫌な。。。　*/
    
    hashtable = (entry *)calloc(PRIME, sizeof(entry));
    if (!hashtable) {
        printf("Need %lu bytes and failed to get them.\n", PRIME*sizeof(entry));
        exit(1);
    }
    
    for (i=0; i < PRIME; i++) {
        hashtable[i].highlock = hashtable[i].newlock = (bitboard)0;
        hashtable[i].highres = hashtable[i].newres = NOHASH;
        hashtable[i].plies = HEIGHT*WIDTH;
    }
}

bitboard hash_usage() {
    bitboard i, c;
    for (i=c=0; i < PRIME; i++) {
        if (hashtable[i].highlock)
            c++;
    }
    return c;
}

bitboard find_symmetry(bitboard code) {
    if (plies < SYMCHECK) {
        bitboard symcode = 0, temp = code;        
        int i;
        for (i=0; i<WIDTH; i++) {
            symcode <<= (HEIGHT+1);
            symcode += temp%(1<<(HEIGHT+1));
            temp >>= (HEIGHT+1);
        }
        if (symcode < code)
            return symcode;
        else
            return code;
    } else {
        return code;
    }
}
    
unsigned int get_hash() {
    bitboard code = board[0] + board[1] + board[plies&1] + bottom;
    code = find_symmetry(code);
       
    bitboard key = (code % PRIME);
    #if LOCKSIZE >= WIDTH*(HEIGHT+1)
        bitboard lock = code;
    #else
        bitboard lock = (code >> (WIDTH*(HEIGHT+1) - LOCKSIZE));
    #endif
        
    if (hashtable[key].highlock == lock) {
        return hashtable[key].highres;
    } else if (hashtable[key].newlock == lock) {
        return hashtable[key].newres;
    } else {
        return NOHASH;
    }
}

void set_hash(int alpha) {
    bitboard code = board[0] + board[1] + board[plies&1] + bottom;
    code = find_symmetry(code);
       
    bitboard key = (code % PRIME);
    #if LOCKSIZE >= WIDTH*(HEIGHT+1)
        bitboard lock = code;
    #else
        bitboard lock = code >> (WIDTH*(HEIGHT+1) - LOCKSIZE);
    #endif

    if (hashtable[key].highlock != lock) {
        if (hashtable[key].plies >= plies) {
            overwritten++;
            hashtable[key].highlock = lock;
            hashtable[key].highres = alpha;
            hashtable[key].plies = plies;
            return;
        } else {
            overwritten++;
            hashtable[key].newlock = lock;
            hashtable[key].newres = alpha;
        }
    }
}

void move(int col, int player) { /* クソ二進法... */
    board[player] ^= (bitboard)1<<(col*(HEIGHT+1) + height[col]++);
    moves[plies++] = col;
}

void unmove(int col, int player) {    
    /* Yes, it's basically the same as move(). Behold the power of xor! */
    board[player] ^= (bitboard)1<<(col*(HEIGHT+1) + --height[col]);
    plies--;
}

int is_legal(int col) {
    return (height[col] < HEIGHT);
}

void print_state(int alpha, int beta) {
    int j;
    
    if (plies <= PRINTSTEPS) {
        printf("plies: %02d (%d), p0: %16ju, p1: %16ju, field: %16ju, alpha: %2d, beta: %2d, moves: ", 
            plies, plies&1, board[0], board[1], board[0]|board[1], alpha, beta);
        for (j=0; j<plies; j++) {
            printf("%d", moves[j]);
        }
        printf("\n");
    }
}    

bitboard has_won(int player) { /*　John Trompのアルゴリズムが。。。 °ω° */
    bitboard diag1 = board[player] & (board[player]>>(HEIGHT));
    bitboard hori = board[player] & (board[player]>>(HEIGHT+1));
    bitboard diag2 = board[player] & (board[player]>>(HEIGHT+2));
    bitboard vert = board[player] & (board[player]>>1);
    return ((diag1 & (diag1 >> 2*HEIGHT)) |
            (hori & (hori >> 2*(HEIGHT+1))) |
            (diag2 & (diag2 >> 2*(HEIGHT+2))) |
            (vert & (vert >> 2)));
}

/* strength is irrelevant */ 
int alphabeta(int alpha, int beta) {
    int threat = -1, freedom = WIDTH, player = plies&1;
    int i, j, result=NOHASH, temp_result;
    
    /*if (plies <= PRINTSTEPS) {
        printf("in.."); print_state(alpha, beta);
    }*/
    
    /* hash is not necessarily accurate, but at least it provides a closer
     * bound.
     */
    if (plies >= MAXDEPTH) return DRAW;
    
    int hash = get_hash();
    if (hash != NOHASH) {
        if (hash == DRAWORLOSE) {
            if ((beta = DRAW) <= alpha) {
                return hash;
            }
        } else if (hash == DRAWORWIN) {
            if ((alpha = DRAW) >= beta) {
                return hash;
            }
        } else {
            return hash;
        }
    }
    
    /* detect threats to the fatherland */
    for (i=0; i<WIDTH; i++) {
        if (is_legal(i)) {
            move(i, player^1);
            if (has_won(player^1)) {
                if (threat == -1) {
                    threat = i;
                } else { /* two threats can't be stopped */
                    unmove(i, player^1);
                    print_state(LOSE, beta);
                    set_hash(LOSE); /* accurate */
                    return LOSE;
                }
            }
            unmove(i, player^1);
        } else {
            freedom--;
        }
    }
    
    if (threat != -1) {
        move(threat, player);
        if (has_won(player)) {
            unmove(threat, player);
            print_state(WIN, beta);
            set_hash(WIN); /* accurate */
            return WIN;
        } else {
            result = TOTAL-alphabeta(TOTAL-beta, TOTAL-alpha);
            alpha = max(alpha,result);
            unmove(threat, player);
            print_state(alpha, beta);
            set_hash(alpha); /* accurate */
            return alpha;
        }
    } else {
        //for (i=0; i<WIDTH; i++) {
        for (j=WIDTH/2; j<(WIDTH+WIDTH/2); j++) {
            i = j%WIDTH;
            if (is_legal(i)) {
                freedom--;
                move(i, player);
                if (has_won(player)) {
                    unmove(i, player);
                    print_state(WIN, beta);
                    set_hash(WIN); /* accurate */
                    return WIN;
                } else {
                    temp_result = TOTAL-alphabeta(TOTAL-beta, TOTAL-alpha);
                    unmove(i, player);
                    if (temp_result > result) {
                        if ((result=temp_result) > alpha && plies >= FULLSTEPS && (alpha = temp_result) >= beta ) {
                            if (result == DRAW && freedom > 0) {
                                result = DRAWORWIN;
                            }
                            break;
                        }
                    } 
                }
            }
        }
        alpha=result;
        
        print_state(alpha, beta);

        set_hash(alpha); /* not accurate, may be just good enough */
        return alpha;
    }
}

void complex_move(char s[]) {
    int i;
    char c;
    for (i=0; (c=s[i]); i++) {
        move(c-'0', i&1);
    }
}

int main() {  
    init();
    complex_move("");
    printf("%c\n", "?L?D?W"[alphabeta(LOSE, WIN)]);
    
    bitboard used = hash_usage();
    printf("hashs used: %ju / %d (%0.3f), %u overwritten.\n", 
           used, PRIME, (float)used / (float)PRIME, overwritten);
    
    return 0;
}
