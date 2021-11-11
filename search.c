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

#define aspwindow 50
#define no_move -15

int ply = 0;
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
int history_moves[2][64][64];

Line pv_line;

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

U64 pastPawnMasks[64] = {
        0x303030303030303ULL, 0x707070707070707ULL, 0xe0e0e0e0e0e0e0eULL, 0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL, 0x7070707070707070ULL, 0xe0e0e0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0c0c0ULL,
        0x303030303030303ULL, 0x707070707070707ULL, 0xe0e0e0e0e0e0e0eULL, 0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL, 0x7070707070707070ULL, 0xe0e0e0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0c0c0ULL,
        0x303030303030303ULL, 0x707070707070707ULL, 0xe0e0e0e0e0e0e0eULL, 0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL, 0x7070707070707070ULL, 0xe0e0e0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0c0c0ULL,
        0x303030303030303ULL, 0x707070707070707ULL, 0xe0e0e0e0e0e0e0eULL, 0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL, 0x7070707070707070ULL, 0xe0e0e0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0c0c0ULL,
        0x303030303030303ULL, 0x707070707070707ULL, 0xe0e0e0e0e0e0e0eULL, 0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL, 0x7070707070707070ULL, 0xe0e0e0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0c0c0ULL,
        0x303030303030303ULL, 0x707070707070707ULL, 0xe0e0e0e0e0e0e0eULL, 0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL, 0x7070707070707070ULL, 0xe0e0e0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0c0c0ULL,
        0x303030303030303ULL, 0x707070707070707ULL, 0xe0e0e0e0e0e0e0eULL, 0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL, 0x7070707070707070ULL, 0xe0e0e0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0c0c0ULL,
        0x303030303030303ULL, 0x707070707070707ULL, 0xe0e0e0e0e0e0e0eULL, 0x1c1c1c1c1c1c1c1cULL,
        0x3838383838383838ULL, 0x7070707070707070ULL, 0xe0e0e0e0e0e0e0e0ULL, 0xc0c0c0c0c0c0c0c0ULL,
};

static inline int score_move(int move, int hashmove){

    if (found_pv){
        if (move == pv_line.moves[ply]){
            found_pv = 0;
            return 20000;
        }
    }

    if (hashmove == move){
        return 5000;
    }

    if (get_move_capture(move)){
        int start_piece, end_piece;
        int target_piece = P;

        int target_square = get_move_target(move);

        if (side == white) {start_piece = p; end_piece = k;}
        else{start_piece = P; end_piece = K;}

        for (int bb_piece = start_piece; bb_piece < end_piece; bb_piece++){
            if (get_bit(bitboards[bb_piece], target_square)){
                target_piece = bb_piece;
                break;
            }
        }

        return mvv_lva[get_move_piece(move)][target_piece]+1000;
    } else {
        if (move == killer_moves[ply][0]){
            return(800);
        }
        if (move == killer_moves[ply][1]){
            return(700);
        }

        if (prevmove != 0) {
            int score = get_move_score(prevmove, move);

            //this seems stupid and it is
            //but not really because of the function partition_zero_scores()
            if (score > 100)
                return score;

        }

        if (get_move_promoted(move))
            return 80;

        if (get_move_castle(move))
            return 50;

        if (get_move_piece(move) == P){
            U64 attack_mask = pawn_mask[white][get_move_target(move)];

            U64 attacked_pieces = (bitboards[n] | bitboards[r]);

            if (pawn_mask[black][get_move_target(move)] & bitboards[P])
                attacked_pieces |= bitboards[b] | bitboards[q];

            attacked_pieces &= attack_mask;

            if (count_bits(attacked_pieces) == 2)
                return 100;

            if (attacked_pieces)
                return 40;

            if (!(pastPawnMasks[get_move_target(move)] & bitboards[p]))
                return 30;

            if (get_move_double(move))
                return 10;
        }

        if (get_move_piece(move) == p){
            U64 attack_mask = pawn_mask[black][get_move_target(move)];

            U64 attacked_pieces = (bitboards[N] | bitboards[R]);

            if (pawn_mask[white][get_move_target(move)] & bitboards[p])
                attacked_pieces |= bitboards[B] | bitboards[Q];

            attacked_pieces &= attack_mask;

            if (count_bits(attacked_pieces) == 2)
                return 100;

            if (attacked_pieces)
                return 40;

            if (!(pastPawnMasks[get_move_target(move)] & bitboards[P]))
                return 20;

            if (get_move_double(move))
                return 10;
        }

        if (get_move_piece(move) == N){
            return count_bits(knight_mask[get_move_target(move)] & (occupancies[black] - bitboards[p] - bitboards[n])) * 30;
        }

        if (get_move_piece(move) == R){
            if (filemasks[get_move_source(move)] & bitboards[P]) {
                if (!(filemasks[get_move_target(move)] & bitboards[P])) {
                    return 15;
                }
            }
        }

        if (get_move_piece(move) == r){
            if (filemasks[get_move_source(move)] & bitboards[p]) {
                if (!(filemasks[get_move_target(move)] & bitboards[p])) {
                    return 15;
                }
            }
        }

        return 0;
    }
}


static inline void sort_moves(moveList *move_list, int hashmove){

    int scores[move_list->count];

    for (int i = 0; i < move_list->count; i++) {
        scores[i] = score_move(move_list->moves[i], hashmove);
    }

    int zerosFound = partition_zero_scores(move_list, scores);
    quickSort(scores, 0, (int )(move_list->count) - 1 - zerosFound, move_list);
}

static inline int quiesce(int alpha, int beta) {
    if (ply > selDepth){
        selDepth = ply;
    }

    if (nodes % 2048 == 0)
        communicate();

    if (stop){
        return 0;
    }

    nodes++;
    qnodes++;

    int stand_pat = nnue_evaluate(&currentNnue);

    if (stand_pat >= beta){
        return beta;
    }

    if (alpha <= stand_pat){
        alpha = stand_pat;
    }

    moveList legalMoves;
    legalMoves.count = 0;

    U64 old_occupancies = occupancies[white] | occupancies[black];

    occupancies[both] = old_occupancies;

    copy_board();

    generate_moves(&legalMoves);
    sort_moves(&legalMoves, no_move);

    for (int moveId = 0; moveId < legalMoves.count; moveId++){
        int move = legalMoves.moves[moveId];

        if (make_move(move, only_captures, 0)){
            ply++;
            int score = -quiesce(-beta, -alpha);
            ply--;

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

static inline int ZwSearch(int beta, int depth){
    nodes++;

    if (nodes % 2048 == 0)
        communicate();

    if (stop){
        return 0;
    }

    if (depth == 0)
        return quiesce(beta-1, beta);

    moveList legalMoves;
    legalMoves.count = 0;

    generate_moves(&legalMoves);
    sort_moves(&legalMoves, no_move);

    copy_board();

    for (int moveId = 0; moveId < legalMoves.count; moveId++){
        int move  = legalMoves.moves[moveId];

        int score;

        if (make_move(move, all_moves, 0)){

            score = -ZwSearch(-beta + 1, depth-1);

            take_back();

            if (score >= beta){
                return beta;
            }
        }
    }
    return beta-1;
}

void find_pv(moveList *moves){
    follow_pv = 0;
    for (int moveId = 0; moveId < moves->count; moveId++){
        if (moves->moves[moveId] == pv_line.moves[ply]){
            found_pv = 1;
            follow_pv = 1;
        }
    }
}

static inline int negamax(int depth, int alpha, int beta, Line *pline) {
    //general maintenence
    nodes++;

    if (nodes % 2048 == 0)
        communicate();

    if (stop) {
        return 0;
    }

    //do check extensions before probing hash table
    int in_check = is_square_attacked(bsf((side == white) ? bitboards[K] : bitboards[k]), (side ^ 1));

    if (in_check)
        depth++;

    //HASH TABLE PROBE
    int hash_move = no_move;
    int hash_lookup = ProbeHash(depth, alpha, beta, &hash_move);
    if ((hash_lookup) != valUNKNOWN) {
        return hash_lookup;
    }

    //TB PROBE
    if (ply != 0) {
        U32 tbres = get_wdl();

        if (tbres != TB_RESULT_FAILED) {

            tbHits++;
            if (tbres == 4)
                return 4000000;
            if (tbres == 2)
                return 0;
            if (tbres == 0)
                return -4000000;
        }
    }

    found_pv = 0;

    Line line;
    line.length = 0;

    int hashf = hashfALPHA;

    if (depth <= 0) {
        pline->length = 0;
        return quiesce(alpha, beta);
    }

    if (is_threefold_repetition()) {
        return 0;
    }


    int eval;
    if (depth >= 3 && in_check == 0 && ply
        && !((occupancies[white] == (bitboards[K] | bitboards[P])) ||
             (occupancies[black] == (bitboards[k] | bitboards[p])))) {
        copy_board();

        side ^= 1;
        enpessant = no_sq;

        eval = -ZwSearch(1 - beta, depth - 3);

        take_back();
        if (eval >= beta) {
            return beta;
        }
    }

    moveList legalMoves;
    memset(&legalMoves, 0, sizeof legalMoves);

    generate_moves(&legalMoves);

    if (follow_pv) {
        find_pv(&legalMoves);
    }

    sort_moves(&legalMoves, hash_move);

    copy_board();

    int legalMoveCount = 0;
    int move;
    int bestMove = no_move;

    for (int moveId = 0; moveId < legalMoves.count; moveId++) {
        move = legalMoves.moves[moveId];

        if (make_move(move, all_moves, 1)) {

            legalMoveCount++;

            ply++;

            if (legalMoveCount == 0) {
                eval = -negamax(depth - 1, -beta, -alpha, &line);
            } else {
                //LMR
                //NO_LMR test for debugging purposes.

                if ((depth >= 3) && (legalMoveCount > 5) && (in_check == 0) && (get_move_capture(move) == 0) && (get_move_promoted(move) == 0)) {
#ifndef NO_LMR
                    eval = -negamax(depth - 2, -alpha - 1, -alpha, &line);
#else
                    eval = alpha + 1;
#endif
                } else {
                    eval = alpha + 1;
                }

                //PV search
                if (eval > alpha) {
                    eval = -negamax(depth - 1, -alpha - 1, -alpha, &line);
                    if ((eval > alpha) && (eval < beta)) {
                        eval = -negamax(depth - 1, -beta, -alpha, &line);
                    }
                }
            }

            ply--;

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
//                    print_fen();
//                    printf("\n%d\n", legalMoveCount);
//                    print_move(move);
//                    printf("\n%d\n\n", ply);
//                }

                if (!get_move_capture(move)) {
                    killer_moves[ply][1] = killer_moves[ply][0];
                    killer_moves[ply][0] = move;

                    history_moves[side][get_move_source(move)][get_move_target(move)] += depth * depth;
                }

                RecordHash(depth, beta, move, hashfBETA);

                return beta;
            }
        }

    }

    if (legalMoveCount == 0) {
        if (in_check) {
            return (-4900000) + ply;

        } else {
            return 0;
        }
    }

    RecordHash(depth, alpha, bestMove, hashf);
    return alpha;

}

#define DEF_ALPHA (-5000000)
#define DEF_BETA (5000000)

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

    int depth = *(int*)arg;

    if (get_wdl() != TB_RESULT_FAILED) {
        int move = get_root_move();

        //this is shit code but thats okay its not my fault :)
        if (move != 0) {
            printf("bestmove ");
            print_move(move);
            printf("\n");
            return NULL;
        }
    }

    start_time();

    stop = 0;
    nodes = 0;
    qnodes = 0;
    tbHits = 0;
    selDepth = 0;

    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_moves, 0, sizeof(history_moves));

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

    reset_hash_table();

    for (int currentDepth = 1; currentDepth <= depth; currentDepth++){

        ply = 0;

        follow_pv = 1;
        found_pv = 0;

        if (dynamicTimeManagment && !willMakeNextDepth(currentDepth, depthTime))
            break;

        depthTime[currentDepth] = (float )get_time_ms();

        int nmRes = negamax(currentDepth, alpha, beta, &negamax_line);

        if (stop) {
            break;
        }

        //if the evaluation is outside of aspiration window bounds, reset alpha and beta and continue the search
        if ((nmRes >= beta) || (nmRes <= alpha)){
            ply = 0;
            selDepth = 0;

            follow_pv = 1;
            found_pv = 0;

            alpha = DEF_ALPHA;
            beta = DEF_BETA;

            memset(&negamax_line, 0, sizeof negamax_line);

            nmRes = negamax(currentDepth, alpha, beta, &negamax_line);

        }

        //if time ran out during aspiration research, break.
        if (stop){
            break;
        }

        depthTime[currentDepth] = (float)get_time_ms() - depthTime[currentDepth];

        eval = nmRes;

        alpha = nmRes - aspwindow;
        beta = nmRes + aspwindow;

        memcpy(&pv_line, &negamax_line, sizeof negamax_line);
        memset(&negamax_line, 0, sizeof negamax_line);

        //TIME MANAGMENT
        if (pv_line.moves[0] == prevBestMove && dynamicTimeManagment) {
            moveTime -= (moveTime / 6);
        }

        prevBestMove = pv_line.moves[0];

        printf("info score %s %d depth %d seldepth %d nodes %ld qnodes %ld tbhits %ld pv ",
               (abs(eval) > 4000000) ? "mate" : "cp" , (abs(eval) > 4000000) ? (4900000 - abs(eval)) * (eval / abs(eval)) : eval/64,
               currentDepth, selDepth, nodes, qnodes, tbHits);

        for (int i = 0; i < currentDepth; i++){
            if (pv_line.moves[i] == 0)
                break;
            print_move(pv_line.moves[i]);
            printf(" ");
        }

        printf("\n");

        if ((abs(eval) > 3000000)){
            break;
        }

    }

    dynamicTimeManagment = 0;

    printf("bestmove ");
    print_move(pv_line.moves[0]);
    printf("\n");

}