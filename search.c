#include "search.h"
#include "board.h"
#include "nnue/propogate.h"
#include "timeman.h"
#include "transposition.h"
#include "Fathom/tbprobe.h"
#include "syzygy.h"
#include "moveOrder.h"
#include "uci.h"
#include <stdio.h>
#include <pthread.h>

#define aspwindow (1700)
#define no_move (-15)

#define DEF_ALPHA (-5000000)
#define DEF_BETA (5000000)

int tbsearch = 0;

int selDepth = 0;

long nodes = 0;
long qnodes = 0;
long tbHits = 0;

const int mvv_lva[12][12] = {
        105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
        104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
        103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
        102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
        101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
        100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

        105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
        104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
        103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
        102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
        101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
        100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

int killer_moves[max_ply][2];

int follow_pv, found_pv;

static inline void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

static inline int partition (int arr[], int low, int high, moveList *movearr)
{
    int pivot = arr[high]; // pivot
    int i = (low - 1); // Index of smaller element and indicates the right position of pivot found so far

    for (int j = low; j <= high - 1; j++)
    {
        // If current element is smaller than the pivot
        if (arr[j] > pivot)
        {
            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
            swap(&(movearr->moves[i]), &(movearr->moves[j]));
        }
    }

    swap(&arr[i + 1], &arr[high]);
    swap(&(movearr->moves[i+1]), &(movearr->moves[high]));

    return (i + 1);
}

void quickSort(int arr[], int low, int high, moveList *movearr)
{
    if (low < high)
    {
        int pi = partition(arr, low, high, movearr);
        quickSort(arr, low, pi - 1, movearr);
        quickSort(arr, pi + 1, high, movearr);
    }
}

int partition_zero_scores(moveList *movearr, int scorearr[]){
    int zerosFound = 0;

    for (int moveId = 0; moveId < (movearr->count - zerosFound); moveId++) {
        if (scorearr[moveId] == 0){

            unsigned swappingToIndex = movearr->count - 1 - zerosFound;

            swap(&movearr->moves[moveId], &movearr->moves[swappingToIndex]);
            swap(&scorearr[moveId], &scorearr[swappingToIndex]);

            zerosFound++;
            moveId--;

        }
    }
    return zerosFound;
}

U64 filemasks[64] = {
        0x101010101010101ULL, 0x202020202020202ULL, 0x404040404040404ULL, 0x808080808080808ULL,
        0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
        0x101010101010101ULL, 0x202020202020202ULL, 0x404040404040404ULL, 0x808080808080808ULL,
        0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
        0x101010101010101ULL, 0x202020202020202ULL, 0x404040404040404ULL, 0x808080808080808ULL,
        0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
        0x101010101010101ULL, 0x202020202020202ULL, 0x404040404040404ULL, 0x808080808080808ULL,
        0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
        0x101010101010101ULL, 0x202020202020202ULL, 0x404040404040404ULL, 0x808080808080808ULL,
        0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
        0x101010101010101ULL, 0x202020202020202ULL, 0x404040404040404ULL, 0x808080808080808ULL,
        0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
        0x101010101010101ULL, 0x202020202020202ULL, 0x404040404040404ULL, 0x808080808080808ULL,
        0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
        0x101010101010101ULL, 0x202020202020202ULL, 0x404040404040404ULL, 0x808080808080808ULL,
        0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
};

U64 pastPawnMasks[2][64] = {
        {0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
         0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
         0x3ULL, 0x7ULL, 0xeULL, 0x1cULL,
         0x38ULL, 0x70ULL, 0xe0ULL, 0xc0ULL,
         0x303ULL, 0x707ULL, 0xe0eULL, 0x1c1cULL,
         0x3838ULL, 0x7070ULL, 0xe0e0ULL, 0xc0c0ULL,
         0x30303ULL, 0x70707ULL, 0xe0e0eULL, 0x1c1c1cULL,
         0x383838ULL, 0x707070ULL, 0xe0e0e0ULL, 0xc0c0c0ULL,
         0x3030303ULL, 0x7070707ULL, 0xe0e0e0eULL, 0x1c1c1c1cULL,
         0x38383838ULL, 0x70707070ULL, 0xe0e0e0e0ULL, 0xc0c0c0c0ULL,
         0x303030303ULL, 0x707070707ULL, 0xe0e0e0e0eULL, 0x1c1c1c1c1cULL,
         0x3838383838ULL, 0x7070707070ULL, 0xe0e0e0e0e0ULL, 0xc0c0c0c0c0ULL,
         0x30303030303ULL, 0x70707070707ULL, 0xe0e0e0e0e0eULL, 0x1c1c1c1c1c1cULL,
         0x383838383838ULL, 0x707070707070ULL, 0xe0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0ULL,
         0x3030303030303ULL, 0x7070707070707ULL, 0xe0e0e0e0e0e0eULL, 0x1c1c1c1c1c1c1cULL,
         0x38383838383838ULL, 0x70707070707070ULL, 0xe0e0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0c0ULL,},

        {0x303030303030300ULL, 0x707070707070700ULL, 0xe0e0e0e0e0e0e00ULL, 0x1c1c1c1c1c1c1c00ULL,
         0x3838383838383800ULL, 0x7070707070707000ULL, 0xe0e0e0e0e0e0e000ULL, 0xc0c0c0c0c0c0c000ULL,
         0x303030303030000ULL, 0x707070707070000ULL, 0xe0e0e0e0e0e0000ULL, 0x1c1c1c1c1c1c0000ULL,
         0x3838383838380000ULL, 0x7070707070700000ULL, 0xe0e0e0e0e0e00000ULL, 0xc0c0c0c0c0c00000ULL,
         0x303030303000000ULL, 0x707070707000000ULL, 0xe0e0e0e0e000000ULL, 0x1c1c1c1c1c000000ULL,
         0x3838383838000000ULL, 0x7070707070000000ULL, 0xe0e0e0e0e0000000ULL, 0xc0c0c0c0c0000000ULL,
         0x303030300000000ULL, 0x707070700000000ULL, 0xe0e0e0e00000000ULL, 0x1c1c1c1c00000000ULL,
         0x3838383800000000ULL, 0x7070707000000000ULL, 0xe0e0e0e000000000ULL, 0xc0c0c0c000000000ULL,
         0x303030000000000ULL, 0x707070000000000ULL, 0xe0e0e0000000000ULL, 0x1c1c1c0000000000ULL,
         0x3838380000000000ULL, 0x7070700000000000ULL, 0xe0e0e00000000000ULL, 0xc0c0c00000000000ULL,
         0x303000000000000ULL, 0x707000000000000ULL, 0xe0e000000000000ULL, 0x1c1c000000000000ULL,
         0x3838000000000000ULL, 0x7070000000000000ULL, 0xe0e0000000000000ULL, 0xc0c0000000000000ULL,
         0x300000000000000ULL, 0x700000000000000ULL, 0xe00000000000000ULL, 0x1c00000000000000ULL,
         0x3800000000000000ULL, 0x7000000000000000ULL, 0xe000000000000000ULL, 0xc000000000000000ULL,
         0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
         0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,}
};

static inline int score_move(int move, int hashmove, Board *board){

    if (found_pv && !board->helperThread){
        if (move == board->pv_line.moves[board->ply]){
            found_pv = 0;
            return 20000;
        }
    }

    if (hashmove == move){
        return 10000;
    }

    if (getcapture(move)){
        int start_piece, end_piece;
        int target_piece = P;

        int target_square = gettarget(move);

        if (board->side == white) {start_piece = p; end_piece = k;}
        else{start_piece = P; end_piece = K;}

        for (int bb_piece = start_piece; bb_piece < end_piece; bb_piece++){
            if (get_bit(board->bitboards[bb_piece], target_square)){
                target_piece = bb_piece;
                break;
            }
        }

        return mvv_lva[getpiece(move)][target_piece] + 1000;
    } else {
        if (move == killer_moves[board->ply][0]){
            return(800);
        }
        if (move == killer_moves[board->ply][1]){
            return(700);
        }

        if (board->prevmove != 0) {
            int score = get_move_score(board->prevmove, move);

            //this seems stupid and it is
            //but not really because of the function partition_zero_scores()
            if (score > 100)
                return score;

        }

        if (getpromoted(move))
            return 80;

        if (getcastle(move))
            return 50;

        if (getpiece(move) == P){
            U64 attack_mask = pawn_mask[white][gettarget(move)];

            U64 attacked_pieces = BN | BR;

            if (pawn_mask[black][gettarget(move)] & WP)
                attacked_pieces |= BB | BQ;

            attacked_pieces &= attack_mask;

            if (count_bits(attacked_pieces) == 2)
                return 100;

            if (attacked_pieces)
                return 40;

            if (!(pastPawnMasks[white][gettarget(move)] & board->bitboards[p]))
                return 30;

            if (getdouble(move))
                return 10;
        }

        if (getpiece(move) == p){
            U64 attack_mask = pawn_mask[black][gettarget(move)];

            U64 attacked_pieces = (WN | BR);

            if (pawn_mask[white][gettarget(move)] & board->bitboards[p])
                attacked_pieces |= (BB | BQ);

            attacked_pieces &= attack_mask;

            if (count_bits(attacked_pieces) == 2)
                return 100;

            if (attacked_pieces)
                return 40;

            if (!(pastPawnMasks[black][gettarget(move)] & board->bitboards[P]))
                return 30;

            if (getdouble(move))
                return 10;
        }

        if (getpiece(move) == N){
            return count_bits(knight_mask[gettarget(move)] & (board->occupancies[black] - BP - BN)) * 30;
        }

        if (getpiece(move) == n){
            return count_bits(knight_mask[gettarget(move)] & (board->occupancies[white] - WP - WN)) * 30;
        }

        if (getpiece(move) == R){
            if (filemasks[getsource(move)] & board->bitboards[P]) {
                if (!(filemasks[gettarget(move)] & board->bitboards[P])) {
                    return 15;
                }
            }
        }

        if (getpiece(move) == r){
            if (filemasks[getsource(move)] & board->bitboards[p]) {
                if (!(filemasks[gettarget(move)] & board->bitboards[p])) {
                    return 15;
                }
            }
        }

        if (getpiece(move) == Q){
            if (is_move_direct_check(move, board))
                return 50;
            return count_bits(king_mask[bsf(BK)] & get_queen_attacks(gettarget(move), board->occupancies[both]));
        }

        if (getpiece(move) == q){
            if (is_move_direct_check(move, board))
                return 50;
            return count_bits(king_mask[bsf(WK)] & get_queen_attacks(gettarget(move), board->occupancies[both]));
        }

        return 0;
    }
}


static inline void sort_moves(moveList *move_list, int hashmove, Board *board){

    int scores[move_list->count];

    for (int i = 0; i < move_list->count; i++) {
        scores[i] = score_move(move_list->moves[i], hashmove, board);
    }

    int zerosFound = partition_zero_scores(move_list, scores);
    quickSort(scores, 0, (int )(move_list->count) - 1 - zerosFound, move_list);
}

static inline int quiesce(int alpha, int beta, Board *board) {
    if (board->ply > selDepth){
        selDepth = board->ply;
    }

    if (nodes % 2048 == 0)
        communicate();

    if (stop){
        return 0;
    }

    nodes++;
    qnodes++;

    int stand_pat = nnue_evaluate(board);

    if (stand_pat >= beta){
        return beta;
    }

    if (alpha <= stand_pat){
        alpha = stand_pat;
    }

    moveList legalMoves;
    legalMoves.count = 0;

    //TODO wtf is this?
    U64 old_occupancies = board->occupancies[white] | board->occupancies[black];
    board->occupancies[both] = old_occupancies;

    copy_board();

    generate_moves(&legalMoves, board);
    sort_moves(&legalMoves, no_move, board);

    for (int moveId = 0; moveId < legalMoves.count; moveId++){
        int move = legalMoves.moves[moveId];

        if (make_move(move, only_captures, 0, board)){
            int score = -quiesce(-beta, -alpha, board);

            take_back();

            if (score >= beta){
                return beta;
            }

            if (score > alpha)
                alpha = score;
        }
    }

    return alpha;

}

static inline int ZwSearch(int beta, int depth, Board *board){
    nodes++;

    if (nodes % 2048 == 0)
        communicate();

    if (stop){
        return 0;
    }

    if (depth == 0)
        return quiesce(beta-1, beta, board);

    moveList legalMoves;
    legalMoves.count = 0;

    generate_moves(&legalMoves, board);
    sort_moves(&legalMoves, no_move, board);

    copy_board();

    for (int moveId = 0; moveId < legalMoves.count; moveId++){
        int move  = legalMoves.moves[moveId];

        int score;

        if (make_move(move, all_moves, 0, board)){

            score = -ZwSearch(-beta + 1, depth-1, board);

            take_back();

            if (score >= beta){
                return beta;
            }
        }
    }
    return beta-1;
}

void find_pv(moveList *moves, Board *board){
    if (board->helperThread)
        return;

    follow_pv = 0;
    for (int moveId = 0; moveId < moves->count; moveId++){
        if (moves->moves[moveId] == board->pv_line.moves[board->ply]){
            found_pv = 1;
            follow_pv = 1;
        }
    }
}

static inline int negamax(int depth, int alpha, int beta, Line *pline, Board *board) {
    //general maintenence
    nodes++;

    if (nodes % 2048 == 0)
        communicate();

    if (stop) {
        return 0;
    }

    //do check extensions before probing hash table
    int in_check = is_square_attacked(bsf((board->side == white) ? board->bitboards[K] : board->bitboards[k]), (board->side ^ 1), board);

    if (in_check)
        depth++;

    if (board->occupancies[both] == (WK | BK | WP | BP)) {
        if (getpiece(board->prevmove) == P) {
            if (!(pastPawnMasks[white][gettarget(board->prevmove)] & board->bitboards[p]))
                depth++;
        } else if (getpiece(board->prevmove) == P) {
            if (!(pastPawnMasks[black][gettarget(board->prevmove)] & board->bitboards[P]))
                depth++;
        }
    }

    if (board->ply == 0)
        board->zobrist_history_search_index = board->zobrist_history_length;

    if (is_threefold_repetition(board)) {
        return 0;
    }

    //HASH TABLE PROBE
    int staticeval = NO_EVAL;
    int hash_move = no_move;
    int hash_lookup = ProbeHash(depth, alpha, beta, &hash_move, &staticeval, pline, board);
    if ((hash_lookup) != valUNKNOWN && board->ply != 0) {
        return hash_lookup;
    }

    if (staticeval == NO_EVAL)
        staticeval = nnue_evaluate(board);

    //TB PROBE
    if (board->ply != 0) {
        U32 tbres = get_wdl(board);

        if (!tbsearch) {
            if (tbres != TB_RESULT_FAILED) {

                tbHits++;
                if (tbres == 4)
                    return 4000000;
                if (tbres == 2)
                    return 0;
                if (tbres == 0)
                    return -4000000;
            }
        } else {
            int move = get_root_move(board);
            if (move != 0) {

                if (tbres == 4)
                    return 4000000;
                if (tbres == 2)
                    return 0;
                if (tbres == 0)
                    return -4000000;
            }
        }
    }

    found_pv = 0;
    int pvnode = beta - alpha > 1;

    Line line;
    line.length = 0;

    int hashf = hashfALPHA;

    if (depth <= 0) {
        pline->length = 0;
        return quiesce(alpha, beta, board);
    }

    int eval;
    if (depth >= 3 && !in_check && board->ply
        && board->occupancies[both] != (WK | BK | WP | BP | WN | BN)) {
        copy_board();

        board->side ^= 1;
        board->enpessant = no_sq;

        eval = -ZwSearch(1 - beta, depth - 3, board);

        take_back();
        if (eval >= beta) {
            RecordHash(depth - 2, beta, no_move, hashfBETA, staticeval, NULL, board);
            return beta;
        }
    }


    if (!pvnode && !in_check && depth < 3) {
        if ((staticeval - (125 * 64)) > beta){
            return beta;
        }
    }

    if (!pvnode && !in_check && depth <= 3) {

        int value = staticeval + (125 * 64);
        if (value < beta) {
            int new_value;
            if (depth == 1) {
                new_value = quiesce(alpha, beta, board);
                pline->length = 0;
                return max(value, new_value);
            }
            value += (175 * 64);
            if (value < beta && depth <= 3) {
                new_value = quiesce(alpha, beta, board);
                if (new_value < beta) {
                    pline->length = 0;
                    return max(new_value, value);
                }
            }
        }
    }

    moveList legalMoves;
    memset(&legalMoves, 0, sizeof legalMoves);

    generate_moves(&legalMoves, board);

    if (follow_pv) {
        find_pv(&legalMoves, board);
    }

    sort_moves(&legalMoves, hash_move, board);

    copy_board();

    int legalMoveCount = 0;
    int move;
    int bestMove = no_move;

    for (int moveId = 0; moveId < legalMoves.count; moveId++) {
        move = legalMoves.moves[moveId];

        if (make_move(move, all_moves, 1, board)) {

            legalMoveCount++;


            if (legalMoveCount == 0) {
                eval = -negamax(depth - 1, -beta, -alpha, &line, board);
            } else {
                //LMR
                //NO_LMR test for debugging purposes.

                if ((depth >= 3) && (legalMoveCount > 5) && (in_check == 0) && (getcapture(move) == 0) && (getpromoted(move) == 0)) {
#ifndef NO_LMR
                    eval = -negamax(depth - 2, -alpha - 1, -alpha, &line, board);
#else
                    eval = alpha + 1;
#endif
                } else {
                    eval = alpha + 1;
                }

                //PV search
                if (eval > alpha) {
//                    if ((depth >= 3) && (legalMoveCount > 5) && (in_check == 0) && (get_move_capture(move) == 0) && (get_move_promoted(move) == 0))
//                    {
//                        if (legalMoveCount > 10 && depth > 4) {
//                            print_fen(board);
//                            printf("\n%d\n", legalMoveCount);
//                            print_move(move);
//                            printf("\n%d\n\n", depth);
//                        }
//                    }

                    eval = -negamax(depth - 1, -alpha - 1, -alpha, &line, board);
                    if ((eval > alpha) && (eval < beta)) {
                        eval = -negamax(depth - 1, -beta, -alpha, &line, board);
                    }
                }
            }


            take_back();

            if (eval > alpha) {

                alpha = eval;

                bestMove = move;
                hashf = hashfEXACT;

                pline->moves[0] = move;
                memcpy(pline->moves + 1, line.moves, line.length * 4);
                pline->length = line.length + 1;

            }

            if (eval >= beta) {

//                if (legalMoveCount > 10 && depth > 2){
//                    print_fen(board);
//                    printf("\n%d\n", legalMoveCount);
//                    print_move(move);
//                    printf("\n%d\n\n", board->ply);
//                }

                if (!getcapture(move) && !board->helperThread) {
                    killer_moves[board->ply][1] = killer_moves[board->ply][0];
                    killer_moves[board->ply][0] = move;

                }

                RecordHash(depth, beta, bestMove, hashfBETA, staticeval, pline, board);

                return beta;
            }
        }

    }

    if (legalMoveCount == 0) {
        if (in_check) {
            return (-4900000) + board->ply;

        } else {
            return 0;
        }
    }

    RecordHash(depth, alpha, bestMove, hashf, staticeval, pline, board);
    return alpha;

}

void remove_illigal_moves(moveList *moves, Board *board){
    copy_board();
    for (int i = 0; i < moves->count; ++i) {
        if (make_move(moves->moves[i], all_moves, 1, board)) {
            take_back();
        } else {
            memcpy(&moves->moves[i], &moves->moves[i+1], (256*4) - (i*4));
            moves->count--;
            i--;
        }
    }
}

typedef struct NegamaxArgs NegamaxArgs;
struct NegamaxArgs{
    Line *pline;
    Board board;
};

typedef struct Thread Thread;
struct Thread{
    pthread_t pthread;
    Line line;
    NegamaxArgs args;
};

void negamax_thread(void *args){
    NegamaxArgs *nargs = args;
    for (int i = 0; i < max_ply; ++i) {
        memset(nargs->pline, 0, sizeof (Line));

        int eval = negamax(i, DEF_ALPHA, DEF_BETA, nargs->pline, &nargs->board);

        memcpy(&nargs->board.pv_line, &nargs->pline, sizeof(Line));

        if (stop)
            return;
    }
}

int willMakeNextDepth(int curd, const float *times){

    if (curd < 4)
        return 1;

    float multiplier = 0;
    float divisor = 0;

    for (int i = curd - 3; i < curd; ++i) {
        if (times[i-1] > 1) {
            multiplier += (times[i] / times[i - 1]);
            divisor += 1;
        }
    }

    if (divisor == 0)
        return 1;

    multiplier = multiplier / divisor;
    float timepred = (multiplier * times[curd-1]);
    float timeleft = (float)(moveTime - (get_time_ms() - startingTime));

    return (timepred < (timeleft * 2)) ? 1 : 0;
}

void *search_position(void *arg){
    //Table bases

    Board board;
    memcpy(&board, &UciBoard, sizeof(Board));

    int depth = *(int*)arg;

    if (get_wdl(&board) != TB_RESULT_FAILED) {
        int move = get_root_move(&board);

        //this is shit code but thats okay its not my fault :)
        if (move != 0) {
            printf("bestmove ");
            print_move(move);
            printf("\n");
            return NULL;
        } else{
            tbsearch = 1;
        }
    }

    start_time();

    stop = 0;
    nodes = 0;
    qnodes = 0;
    tbHits = 0;
    selDepth = 0;

    memset(killer_moves, 0, sizeof(killer_moves));

    Line negamax_line;
    negamax_line.length = 0;

    int alpha = DEF_ALPHA;
    int beta = DEF_BETA;

    int eval;

    //best move from previous depth
    int prevBestMove = 0;

    //how long did it take to search to current depth
    float depthTime[max_ply];
    memset(depthTime, 0, sizeof depthTime);

//    int hash_move = no_move;
//    int staticeval = 0;
//    int hash_lookup = ProbeHash(6, alpha, beta, &hash_move, &staticeval, &pv_line, &board);
//    if ((hash_lookup) != valUNKNOWN) {
//        dynamicTimeManagment = 0;
//    }
    reset_hash_table();

    moveList legalMoves;
    generate_moves(&legalMoves, &board);
    remove_illigal_moves(&legalMoves, &board);
    sort_moves(&legalMoves, no_move, &board);
    int numThreads = threadCount < legalMoves.count ? (int) threadCount : (int) legalMoves.count;
    Thread threads[numThreads];

    if (threadCount > 1) {

        memset(&threads, 0, sizeof threads);

        for (int i = 0; i < numThreads; ++i) {
            threads[i].args = (struct NegamaxArgs) {.pline = &threads[i].line, .board = board};
            make_move(legalMoves.moves[i], all_moves, 1, &threads[i].args.board);

            pthread_create(&threads[i].pthread, NULL, (void *(*)(void *)) negamax_thread, &threads[i].args);
        }
    }

    for (int currentDepth = 1; currentDepth <= depth; currentDepth++){

        board.ply = 0;

        follow_pv = 1;
        found_pv = 0;

        if (dynamicTimeManagment && !willMakeNextDepth(currentDepth, depthTime))
            break;

        depthTime[currentDepth] = (float )get_time_ms();

        int nmRes = negamax(currentDepth, alpha, beta, &negamax_line, &board);

        if (stop) {
            break;
        }

        //if the evaluation is outside of aspiration window bounds, reset alpha and beta and continue the search
        if ((nmRes >= beta) || (nmRes <= alpha)){
            board.ply = 0;
            selDepth = 0;

            follow_pv = 1;
            found_pv = 0;

            alpha = DEF_ALPHA;
            beta = DEF_BETA;

            memset(&negamax_line, 0, sizeof negamax_line);

            nmRes = negamax(currentDepth, alpha, beta, &negamax_line, &board);

        }

        //if time ran out during aspiration research, break.
        if (stop){
            break;
        }

        depthTime[currentDepth] = (float)get_time_ms() - depthTime[currentDepth];

        eval = nmRes;

        alpha = nmRes - aspwindow;
        beta = nmRes + aspwindow;

        memcpy(&board.pv_line, &negamax_line, sizeof negamax_line);
        memset(&negamax_line, 0, sizeof negamax_line);

        //TIME MANAGMENT
        if (board.pv_line.moves[0] == prevBestMove && dynamicTimeManagment) {
            moveTime -= (moveTime / 6);
        }

        prevBestMove = board.pv_line.moves[0];

        printf("info score %s %d depth %d seldepth %d nodes %ld qnodes %ld tbhits %ld time %d pv ",
               (abs(eval) > 4000000) ? "mate" : "cp" , (abs(eval) > 4000000) ? (4900000 - abs(eval)) * (eval / abs(eval)) : eval/64,
               currentDepth, selDepth, nodes, qnodes, tbHits, (get_time_ms() - startingTime));

        for (int i = 0; i < currentDepth; i++){
            if (board.pv_line.moves[i] == 0)
                break;
            print_move(board.pv_line.moves[i]);
            printf(" ");
        }

        printf("\n");

        if ((abs(eval) > 3000000)){
            break;
        }

    }

    if (threadCount > 1) {
        int originalStop = stop;

        stop = 1;
        for (int i = 0; i < numThreads; ++i) {
            pthread_join(threads[i].pthread, NULL);
        }

        stop = originalStop;
    }

    dynamicTimeManagment = 0;
    tbsearch = 0;

    printf("bestmove ");
    print_move(board.pv_line.moves[0]);
    printf("\n");

}
