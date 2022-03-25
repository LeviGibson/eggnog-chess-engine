#include "search.h"
#include "nnue.h"
#include "timeman.h"
#include "transposition.h"
#include "Fathom/tbprobe.h"
#include "syzygy.h"
#include "uci.h"
#include "moveOrder.h"
#include "see.h"
#include <stdio.h>
#include <pthread.h>

#define fabs(x) (((x) > 0) ? (x) : -(x))

int32_t DEF_ASPWINDOW = 1760;

#define DEF_ALPHA (-5000000)
#define DEF_BETA (5000000)

int32_t aspwindow;
int32_t tbsearch = 0;

int32_t selDepth = 0;

long nodes = 0;
long qnodes = 0;
long tbHits = 0;

const int32_t mvv_lva[12][12] = {
        15, 25, 35, 45, 55, 65,  15, 25, 35, 45, 55, 65,
        14, 24, 34, 44, 54, 64,  14, 24, 34, 44, 54, 64,
        13, 23, 33, 43, 53, 63,  13, 23, 33, 43, 53, 63,
        12, 22, 32, 42, 52, 62,  12, 22, 32, 42, 52, 62,
        11, 21, 31, 41, 51, 61,  11, 21, 31, 41, 51, 61,
        10, 20, 30, 40, 50, 60,  10, 20, 30, 40, 50, 60,

        15, 25, 35, 45, 55, 65,  15, 25, 35, 45, 55, 65,
        14, 24, 34, 44, 54, 64,  14, 24, 34, 44, 54, 64,
        13, 23, 33, 43, 53, 63,  13, 23, 33, 43, 53, 63,
        12, 22, 32, 42, 52, 62,  12, 22, 32, 42, 52, 62,
        11, 21, 31, 41, 51, 61,  11, 21, 31, 41, 51, 61,
        10, 20, 30, 40, 50, 60,  10, 20, 30, 40, 50, 60
};

int32_t historyCount;
int32_t killer_moves[MAX_PLY][2];
float history_moves[12][64][64];

typedef struct MoveEval MoveEval;

struct MoveEval{
    int32_t move;
    int32_t eval;
};

static inline void swap(int* a, int* b) {
    int32_t t = *a;
    *a = *b;
    *b = t;
}

void insertion_sort(MoveList *movearr){
    int32_t i = 1;
    while (i < movearr->count){
        int32_t j = i;
        while (j > 0 && movearr->scores[j-1] < movearr->scores[j]) {
            swap(&movearr->scores[j], &movearr->scores[j - 1]);
            swap(&movearr->moves[j], &movearr->moves[j - 1]);
            j--;
        }
        i++;
    }
}

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

void print_m256(__m256i _x){
    U64 *x = (U64*)&_x;
    for (int32_t i = 0; i < 4; ++i) {
        for (int32_t j = 0; j < 64; ++j) {
            if (j % 16 == 0)
                printf("\n");
            printf("%d ", get_bit(x[i], j) ? 1 : 0);
        }
    }

    printf("\n\n");
}

#ifdef AVX2
uint16_t andmask[16] = {1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7,
                       1 << 8, 1 << 9, 1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15};

__m256i inverse_maskmove_epi16(U64 x){
    __m256i _and = _mm256_loadu_si256((const void *) andmask);
    int16_t x16 = *(int16_t*)&x;
    __m256i _mask = _mm256_set1_epi16(x16);
    _mask = _mm256_and_si256(_mask, _and);
    _mask = _mm256_cmpeq_epi16(_mask, _and);
    return _mask;
}

int16_t hadd_epi16(__m256i x) {
    const __m128i hiQuad = (__m128i)_mm256_extractf128_ps((__m256)x, 1);
    const __m128i loQuad = (__m128i)_mm256_castps256_ps128((__m256)x);
    const __m128i sumQuad = _mm_add_epi16(loQuad, hiQuad);
    const __m128i hiDual = (__m128i)_mm_movehl_ps((__m128)sumQuad, (__m128)sumQuad);
    const __m128i sumDual = _mm_add_epi16(sumQuad, hiDual);
    return _mm_extract_epi16(sumDual, 0) + _mm_extract_epi16(sumDual, 1) + _mm_extract_epi16(sumDual, 2) + _mm_extract_epi16(sumDual, 3);
}


#elif defined(AVX)

uint16_t andmask[16] = {1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7,
                       1 << 8, 1 << 9, 1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15};

int32_t bitshifts[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

__m256i cmpeq(__m256i x, __m256i y){
    __m128i x2 = (__m128i)_mm256_extractf128_ps((__m256)x, 1);
    __m128i x1 = (__m128i)_mm256_castps256_ps128((__m256)x);

    __m128i y2 = (__m128i)_mm256_extractf128_ps((__m256)y, 1);
    __m128i y1 = (__m128i)_mm256_castps256_ps128((__m256)y);

    x1 = _mm_cmpeq_epi16(x1, y1);
    x2 = _mm_cmpeq_epi16(x2, y2);

    __m256i c = (__m256i)_mm256_castps128_ps256((__m128)x1);
    c = (__m256i)_mm256_insertf128_ps((__m256)c, (__m128)x2, 1);

    return c;
}

__m256i inverse_maskmove_epi16(U64 x){
    __m256i _and = _mm256_loadu_si256((const void *) andmask);
    int16_t x16 = *(int16_t*)&x;
    __m256i _mask = _mm256_set1_epi16(x16);
    _mask = (__m256i)_mm256_and_ps((__m256)_mask, (__m256)_and);
    _mask = cmpeq(_mask, _and);

    return _mask;
}

int16_t hadd_epi16(__m256i x) {
    const __m128i hiQuad = (__m128i)_mm256_extractf128_ps((__m256)x, 1);
    const __m128i loQuad = (__m128i)_mm256_castps256_ps128((__m256)x);
    const __m128i sumQuad = _mm_add_epi16(loQuad, hiQuad);
    const __m128i hiDual = (__m128i)_mm_movehl_ps((__m128)sumQuad, (__m128)sumQuad);
    const __m128i sumDual = _mm_add_epi16(sumQuad, hiDual);
    return _mm_extract_epi16(sumDual, 0) + _mm_extract_epi16(sumDual, 1) + _mm_extract_epi16(sumDual, 2) + _mm_extract_epi16(sumDual, 3);
}

#endif

int16_t getScoreFromMoveTable(U64 bitboard, const int16_t *bbPart){

#ifdef AVX2
    bitboard = ~bitboard;
        __m256i score = _mm256_set1_epi16(0);

        for (int32_t i = 0; i < 64; i += 16) {
            __m256i _x = _mm256_loadu_si256((const void *) &bbPart[i]);
            _x = _mm256_xor_si256(_x, inverse_maskmove_epi16(bitboard >> i));

            score = _mm256_add_epi16(_x, score);
        }

        return hadd_epi16(score);

#elif defined(AVX)
    bitboard = ~bitboard;
    int16_t score = 0;

    for (int32_t i = 0; i < 64; i += 16) {
        __m256i _x = _mm256_loadu_si256((const void *) &bbPart[i]);
        _x = (__m256i)_mm256_xor_ps((__m256)_x, (__m256)inverse_maskmove_epi16(bitboard >> i));

        score += hadd_epi16(_x);
    }

    return score;
#else

    int16_t score = 0;

    for (int32_t i = 0; i < 64; ++i) {
        if (get_bit(bitboard, i)){
            score += bbPart[i];
        } else {
            score -= bbPart[i];
        }
    }

    return score;

#endif

}

#define MOVE_HASH_SIZE 200000
int32_t moveScoreHash[MOVE_HASH_SIZE];

void search_init(){
    for (int i = 0; i < MOVE_HASH_SIZE; ++i) {
        moveScoreHash[i] = NO_MOVE;
    }
}

//Board is included in Thread *thread
//returns the score of a move, integer.
//int32_t *hashmove is  the best move stored in the hash table (from a previous depth)
int32_t score_move(int32_t move, const int32_t *hashmove, Thread *thread){
    Board *board = &thread->board;

    if (thread->found_pv){
        if (move == board->prevPv.moves[board->ply]){
            thread->found_pv = 0;
            return 200000;
        }
    }

    for (int32_t i = 0; i < 4; i++) {
        if (hashmove[i] == -15 || hashmove[i] == 0)
            break;
        if (hashmove[i] == move) {
            return 100000 - i;
        }
    }

    if (getcapture(move)){


        int32_t start_piece, end_piece;
        int32_t target_piece = p_P;

        int32_t target_square = gettarget(move);

        if (board->side == white) {start_piece = p_p; end_piece = p_k;}
        else{start_piece = p_P; end_piece = p_K;}

        for (int32_t bb_piece = start_piece; bb_piece < end_piece; bb_piece++){
            if (get_bit(board->bitboards[bb_piece], target_square)){
                target_piece = bb_piece;
                break;
            }
        }

        if (board->side == white){
            if ((getpiece(move) != p_P) && (pawn_mask[white][gettarget(move)] & BP))
                return 10000;
        } else {
            if ((getpiece(move) != p_p) && (pawn_mask[black][gettarget(move)] & WP))
                return 10000;
        }

        int32_t val = seeCapture(move, board);
        if (board->quinode)
            return val + mvv_lva[getpiece(move)][target_piece];
        return val + mvv_lva[getpiece(move)][target_piece] + 10000;

    } else {
        if (move == killer_moves[board->ply][0]){
            return(8000);
        }
        if (move == killer_moves[board->ply][1]){
            return(7000);
        }

        if (board->quinode){ return 0; }

        int32_t score = 0;
        int32_t pieceCount = count_bits(WB | WN | WR | WQ | BB | BN | BR | BQ);

        score = get_nnom_score(move, board);

        if (historyCount > 0) {
            float historyscore = (history_moves[getpiece(move)][getsource(move)][gettarget(move)] / (float) historyCount) * (float)historyMoveDivisor;
            score += ((int32_t )historyscore) * 10;
        }

        return (int)score;
    }
}



//sorts a list of moves
//int32_t *hashmove is the move stored in the transposition table from a previous depth
//pretty much ignore this function it's very boring. The main function is the function above (score_move)
static inline void sort_moves(MoveList *move_list, int32_t *hashmove, Thread *thread){

    for (int32_t i = 0; i < move_list->count; i++) {
        move_list->scores[i] = score_move(move_list->moves[i], hashmove, thread);
    }

    insertion_sort(move_list);

//    if (thread->board.quinode) return;
//    print_fen(&thread->board);
//    printf("\n");
//     for (int32_t i = 0; i < move_list->count; i++) {
//         print_move(move_list->moves[i]);
//         printf(" : %d\n", move_list->scores[i]);
//     }
//
//     printf("\n");
}

static inline int32_t quiesce(int32_t alpha, int32_t beta, Thread *thread) {
    Board *board = &thread->board;

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

    int32_t stand_pat = nnue_evaluate(board);

    if (stand_pat >= beta){
        return beta;
    }

    if (alpha <= stand_pat){
        alpha = stand_pat;
    }

    MoveList legalMoves;
    legalMoves.count = 0;

    U64 old_occupancies = board->occupancies[white] | board->occupancies[black];
    board->occupancies[both] = old_occupancies;

    copy_board();
    board->quinode = 1;

    int32_t tmp[4] = {0,0,0,0};

    generate_moves(&legalMoves, board);
    sort_moves(&legalMoves, tmp, thread);

    for (int32_t moveId = 0; moveId < legalMoves.count; moveId++){
        int32_t move = legalMoves.moves[moveId];

        if (legalMoves.scores[moveId] <= -220)
            continue;

        if (make_move(move, only_captures, 0, board)){
            int32_t score = -quiesce(-beta, -alpha, thread);

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

void find_pv(MoveList *moves, Thread *thread){
    Board *board = &thread->board;

    thread->follow_pv = 0;
    for (int32_t moveId = 0; moveId < moves->count; moveId++){
        if (moves->moves[moveId] == board->prevPv.moves[board->ply]){
            thread->found_pv = 1;
            thread->follow_pv = 1;
        }
    }
}

//the main search function
//don't call this directly. Call search_position().
static inline int32_t search(int32_t depth, int32_t alpha, int32_t beta, Line *pline, Thread *thread) {
    //general maintenence
    nodes++;

    if (nodes % 2048 == 0)
        communicate();

    if (stop) {
        return 0;
    }

    Board *board = &thread->board;

    //extend depth when entering king+pawn endgames
    if (!board->kpExtended && board->occupancies[both] == (WK | BK | WP | BP)){
        if (board->ply)
            depth += (depth/2);
        board->kpExtended = 1;
    }

    //do check extensions before probing hash table
    int32_t in_check = is_square_attacked(bsf((board->side == white) ? board->bitboards[p_K] : board->bitboards[p_k]), (board->side ^ 1), board);

    if (in_check)
        depth++;

//    if (count_bits(board->unprotectedPieces[white] | board->unprotectedPieces[black]) <= 4)
//        board->depthAdjuster-=.5f;

    //depthAdjuster is a float that is added to and subtracted from in parent nodes
    //if it is greater than 1, reset it, then add one to the depth
    //if it is less than 1, reset it and subtract one from the depth
    depth += (int)board->depthAdjuster;
    board->depthAdjuster -= (float)(int)board->depthAdjuster;

    //find weather this node's move is a past pawn push. If it is, there is not late move reduction.
    int32_t isPastPawnPush = 0;
    if (board->occupancies[both] == (WK | BK | WP | BP)) {
        if (getpiece(board->prevmove) == p_P) {
            if (!(pastPawnMasks[white][gettarget(board->prevmove)] & board->bitboards[p_p]))
                isPastPawnPush = 1;
        } else if (getpiece(board->prevmove) == p_P) {
            if (!(pastPawnMasks[black][gettarget(board->prevmove)] & board->bitboards[p_P]))
                isPastPawnPush = 1;
        }
    }

    //value for move ordering.

    //board->zobrist_history_search_index is a value for threefold-repetition detection during the search.
    //If all the repititions are during the search, then we can return 0 if there are only 2 repetitions/s
    if (board->ply == 0)
        board->zobrist_history_search_index = board->zobrist_history_length;

    //self explanitory
    if (is_threefold_repetition(board)) {
        return 0;
    }


    //hash table probe
    //hash_move[4] is the best moves from previous depths stored in the transposition table.
    int32_t hash_move[4] = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE};
    int32_t hash_lookup = ProbeHash(depth, alpha, beta, hash_move, pline, board);

    //return hash lookup if it meets the parameters (in function ProbeHash)
    if (hash_lookup != valUNKNOWN && board->ply != 0) {
        if (abs(hash_lookup) > (CHECKMATE_SCORE - MAX_PLY))
            hash_lookup = (CHECKMATE_SCORE - board->ply) * hash_lookup > 0 ? 1 : -1;
        return hash_lookup;
    }
    int32_t hashf = hashfALPHA;

    int32_t staticeval = nnue_evaluate(board);

    //probing the syzygy table bases
    if (board->ply != 0) {
        U32 tbres = get_wdl(board);

        if (!tbsearch) {
            if (tbres != TB_RESULT_FAILED) {

                tbHits++;
                if (tbres == TB_WIN)
                    return CHECKMATE_SCORE - board->ply;
                if (tbres == TB_DRAW)
                    return 0;
                if (tbres == TB_LOSS)
                    return -CHECKMATE_SCORE + board->ply;
            }
        } else {
            int32_t move = get_root_move(board);
            if (move != 0) {

                if (tbres == TB_WIN)
                    return CHECKMATE_SCORE - board->ply;
                if (tbres == TB_DRAW)
                    return 0;
                if (tbres == TB_LOSS)
                    return -CHECKMATE_SCORE + board->ply;
            }
        }
    }

    //is this a pv node? idk. This code finds that out.
    thread->found_pv = 0;

    board->pvnode = beta - alpha > 1;
    if (!board->pvnode) {
        board->depthAdjuster -= .125f;
    }

    //storing the principle variation
    Line line;
    line.length = 0;

    if (depth <= 0) {
        pline->length = 0;
        return quiesce(alpha, beta, thread);
    }

    //Null Move Pruning
    int32_t eval;
    if (depth >= 3 && !in_check && board->ply
        && (WQ | WR) && (BQ | BR)){
        copy_board();

        make_null_move(board);
        board->enpessant = no_sq;

        Line nmline;
        nmline.length = 0;
        eval = -search(depth-3, -beta, 1-beta, &nmline, thread);

        take_back();
        if (eval >= beta) {
            return beta;
        }
    }


    //Static Null Move Pruning / Evaluation pruning
    if (!board->pvnode && !in_check && depth < 3) {
        if ((staticeval - (23 * 64 * depth)) > beta) {
            return beta;
        }
    }

    //Generating and sorting the legal moves    
    MoveList legalMoves;
    memset(&legalMoves, 0, sizeof legalMoves);
    generate_moves(&legalMoves, board);
    if (thread->follow_pv) {
        find_pv(&legalMoves, thread);
    }
    sort_moves(&legalMoves, hash_move, thread);


    copy_board();

    int32_t legalMoveCount = 0;
    int32_t move;
    MoveEval best = {.move = NO_MOVE, .eval = -100000000};

    //Looping over all the legal moves
    for (uint8_t moveId = 0; moveId < legalMoves.count; moveId++) {
        move = legalMoves.moves[moveId];
        //TODO tune this
#ifndef NO_LMR
//        int margins[3] = {0, 0, 200};
//        if (legalMoveCount && depth <= 2 && (!getcapture(move)) && (!in_check) && !board->pvnode && ((staticeval + (legalMoves.scores[moveId]*64)) + (margins[depth]*64) < alpha) && !is_move_direct_check(move, board))
//            continue;
#endif

        //The illigal moves (moving pinned pieces mostly) are not removed during the move generation, they are removed here.
        if (make_move(move, all_moves, 1, board)) {

            legalMoveCount++;

            if (legalMoveCount == 0) {
                //Pv Search
                eval = -search(depth - 1, -beta, -alpha, &line, thread);
            } else {
                //Late Move Reduction
                if ((depth >= 3) && (legalMoves.scores[moveId] < 5000) && (in_check == 0) && (getcapture(move) == 0) && (!isPastPawnPush)) {
#ifndef NO_LMR
                    eval = -search(depth - 2, -alpha - 1, -alpha, &line, thread);
#else
                    eval = alpha + 1;
#endif
                } else {
                    eval = alpha + 1;
                }

                //If the reduced search produced good results (greater than alpha), research with regular depth, than with full window.
                if (eval > alpha) {
                    eval = -search(depth - 1, -alpha - 1, -alpha, &line, thread);
                    if ((eval > alpha) && (eval < beta)) {
                        eval = -search(depth - 1, -beta, -alpha, &line, thread);
                    }
                }
            }


            take_back();

            //best.eval is the best move found so far (different from alpha, because best.eval does not inherit values from parent nodes.)
            if (eval > best.eval) {

                best.move = move;
                best.eval = eval;

                if (eval > alpha) {

                    alpha = eval;
                    hashf = hashfEXACT;

                    pline->moves[0] = move;
                    memcpy(pline->moves + 1, line.moves, line.length * 4);
                    pline->length = line.length + 1;

                }

                //beta cutoff
                if (eval >= beta) {
                    if (!getcapture(move)) {
                        //killer move heuristic
                        if (!board->helperThread) {
                            killer_moves[board->ply][1] = killer_moves[board->ply][0];
                            killer_moves[board->ply][0] = move;
                        }

                        //history move heuristic
                        history_moves[getpiece(move)][getsource(move)][gettarget(move)] += (float) (depth * depth * legalMoveCount);
                        historyCount += depth * depth * legalMoveCount;
                    }

                    RecordHash(depth, beta, best.move, hashfBETA, pline, board);

                    return beta;
                }
            }
        }

    }

    if (legalMoveCount == 0) {
        if (in_check) {
            return (-CHECKMATE_SCORE) + board->ply;

        } else {
            return 0;
        }
    }

    RecordHash(depth, alpha, best.move, hashf, pline, board);
    return alpha;

}

//idk theads are weird
typedef struct NegamaxArgs NegamaxArgs;
struct NegamaxArgs{
    Line *pline;
    Thread thread;
};

typedef struct HelperThread HelperThread;
struct HelperThread{
    pthread_t pthread;
    Line line;
    NegamaxArgs args;
};

//helper thread function (populates the transposition table to help the main search)
void negamax_thread(void *args){
    NegamaxArgs *nargs = args;

    for (int32_t i = 0; i < MAX_PLY; ++i) {
        memset(nargs->pline, 0, sizeof (Line));

        int32_t eval = search(i, DEF_ALPHA, DEF_BETA, nargs->pline, &nargs->thread);

        memcpy(&nargs->thread.board.prevPv, nargs->pline, sizeof(Line));

        if (stop)
            return;
    }
}

//This function predicts how long it will take to make it to the next depth (iterative deepening)
//if the time is not within the time limit, stop searching
//if the time is, then keep searching
//if the actual time exeeds the predicted time, then there's probably a change in PV, so it's worth searching
int32_t willMakeNextDepth(int32_t curd, const float *times){

    if (curd < 4)
        return 1;

    float multiplier = 0;
    float divisor = 0;

    for (int32_t i = curd - 3; i < curd; ++i) {
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

//the main interface function for the search.
void *search_position(void *arg){
    //Table bases

    Thread thread;
    Board *board = &thread.board;

    memcpy(board, &UciBoard, sizeof(Board));

    int32_t depth = *(int*)arg;

    if (get_wdl(board) != TB_RESULT_FAILED) {
        int32_t move = get_root_move(board);

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

    board->quinode = 0;
    stop = 0;
    nodes = 0;
    qnodes = 0;
    tbHits = 0;
    selDepth = 0;
    historyCount = 0;
    aspwindow = DEF_ASPWINDOW;

    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_moves, 0, sizeof(history_moves));

    Line negamax_line;
    negamax_line.length = 0;

    int32_t alpha = DEF_ALPHA;
    int32_t beta = DEF_BETA;

    int32_t eval;

    //best move from previous depth
    int32_t prevBestMove = 0;

    //how long did it take to search to current depth
    float depthTime[MAX_PLY];
    memset(depthTime, 0, sizeof depthTime);

    int32_t tmp[4] = {0,0,0,0};
    reset_hash_table();

    MoveList legalMoves;
    generate_moves(&legalMoves, board);
    generate_only_legal_moves(&legalMoves, board);
    sort_moves(&legalMoves, tmp, &thread);
    int32_t numThreads = threadCount < legalMoves.count ? (int) threadCount : (int) legalMoves.count;
    HelperThread threads[numThreads];

    if (threadCount > 1) {

        memset(&threads, 0, sizeof threads);

        for (int32_t i = 0; i < numThreads; ++i) {
            threads[i].args = (struct NegamaxArgs) {.pline = &threads[i].line, .thread.board = *board};
            make_move(legalMoves.moves[i], all_moves, 1, &threads[i].args.thread.board);

            pthread_create(&threads[i].pthread, NULL, (void *(*)(void *)) negamax_thread, &threads[i].args);
        }
    }

#ifdef NO_LMR
    printf("info string This build is without Late Move Reduction, and should be used for debugging purposes only.\n");
#endif

    for (int32_t currentDepth = 1; currentDepth <= depth; currentDepth++){

        board->ply = 0;

        thread.follow_pv = 1;
        thread.found_pv = 0;
        thread.board.depthAdjuster = 0;

        if (dynamicTimeManagment && !willMakeNextDepth(currentDepth, depthTime))
            break;

        depthTime[currentDepth] = (float )get_time_ms();

        int32_t nmRes = search(currentDepth, alpha, beta, &negamax_line, &thread);

        if (stop) {
            break;
        }

        //if the evaluation is outside of aspiration window bounds, reset alpha and beta and continue the search
        if ((nmRes >= beta) || (nmRes <= alpha)){
            board->ply = 0;
            selDepth = 0;

            thread.follow_pv = 1;
            thread.found_pv = 0;

            if (currentDepth > 2)
                aspwindow *= 2;

            printf("info string aspiration research. Window = %d\n", aspwindow/64);

            alpha = DEF_ALPHA;
            beta = DEF_BETA;

            memset(&negamax_line, 0, sizeof negamax_line);

            nmRes = search(currentDepth, alpha, beta, &negamax_line, &thread);

        } else {
            aspwindow -= (aspwindow/2);
        }

        //if time ran out during aspiration research, break.
        if (stop){
            break;
        }

        depthTime[currentDepth] = (float)get_time_ms() - depthTime[currentDepth];

        eval = nmRes;

        alpha = nmRes - aspwindow;
        beta = nmRes + aspwindow;

        memcpy(&board->prevPv, &negamax_line, sizeof negamax_line);
        memset(&negamax_line, 0, sizeof negamax_line);

        //TIME MANAGMENT
        if (dynamicTimeManagment) {
            if (board->prevPv.moves[0] == prevBestMove) {
                moveTime -= (moveTime / 12);
            } else {
                moveTime += (moveTime / 10);
            }

            if (abs(nmRes)/60 < 180) {
                float floatingAspWindow = (float) aspwindow / 60;
                float floatingMoveTime = (float) moveTime;

                floatingMoveTime += floatingMoveTime * ((floatingAspWindow - 25) / 200);
                moveTime = (int) floatingMoveTime;
            }
        }

        prevBestMove = board->prevPv.moves[0];

        printf("info score %s %d depth %d seldepth %d nodes %ld nps %ld qnodes %ld tbhits %ld time %d pv ",
               (abs(eval) > 4000000) ? "mate" : "cp" , (abs(eval) > 4000000) ? (4900000 - abs(eval)) * (eval / abs(eval)) : eval/64,
               currentDepth, selDepth, nodes, ((nodes*1000)/(max(1, get_time_ms() - startingTime))), qnodes, tbHits, (get_time_ms() - startingTime));

        for (int32_t i = 0; i < board->prevPv.length; i++){
            if (board->prevPv.moves[i] == 0)
                break;
            print_move(board->prevPv.moves[i]);
            printf(" ");
        }

        printf("\n");

        if ((abs(eval) > 3000000)){
            break;
        }

    }

    if (threadCount > 1) {
        int32_t originalStop = stop;

        stop = 1;
        for (int32_t i = 0; i < numThreads; ++i) {
            pthread_join(threads[i].pthread, NULL);
        }

        stop = originalStop;
    }

    dynamicTimeManagment = 0;
    tbsearch = 0;

    printf("bestmove ");
    print_move(board->prevPv.moves[0]);
    printf("\n");

    return NULL;
}
