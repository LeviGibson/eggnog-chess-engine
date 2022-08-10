#include "search.h"
#include "nnue.h"
#include "timeman.h"
#include "transposition.h"
#include "Fathom/tbprobe.h"
#include "syzygy.h"
#include "uci.h"
#include "see.h"
#include "moveorder.h"
#include <math.h>
#include <stdio.h>
#include <pthread.h>

int32_t DEF_ASPWINDOW = 1760;

#define DEF_ALPHA (-5000000)
#define DEF_BETA (5000000)

int32_t aspwindow;
int32_t tbsearch = 0;

int32_t selDepth = 0;

long nodes = 0;
long qnodes = 0;
long tbHits = 0;

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

    if (stand_pat < (alpha - (1300*64))) {
        return alpha;
    }

    if (alpha <= stand_pat){
        alpha = stand_pat;
    }

    MoveList legalMoves;
    legalMoves.count = 0;

    U64 old_occupancies = board->occupancies[white] | board->occupancies[black];
    board->occupancies[both] = old_occupancies;

    board->quinode = 1;
    copy_board();

    int32_t tmp[4] = {0,0,0,0};

    generate_moves(&legalMoves, board);
    sort_moves(&legalMoves, tmp, thread);

    for (int32_t moveId = 0; moveId < legalMoves.count; moveId++){
        int32_t move = legalMoves.moves[moveId];

        if (getcapture(move) && (stand_pat + (legalMoves.scores[moveId]*64) + (1300*64)) < alpha) {
            continue;
        }

        if (make_move(move, only_captures, 0, board)){
            int32_t score = -quiesce(-beta, -alpha, thread);

            take_back();

            if (score > alpha)
                alpha = score;

            if (score >= beta){
                return beta;
            }
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

void shift_moveeval(MoveEval *ptr){
    ptr->move[5] = ptr->move[4];
    ptr->move[4] = ptr->move[3];
    ptr->move[3] = ptr->move[2];
    ptr->move[2] = ptr->move[1];
    ptr->move[1] = ptr->move[0];
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
        int32_t qui = quiesce(alpha, beta, thread);
        pline->length = 0;
        board->quinode = 0;
        return qui;
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
    MoveEval best = {.move = {NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE, NO_MOVE}, .eval = -100000000};

    //Looping over all the legal moves
    for (uint8_t moveId = 0; moveId < legalMoves.count; moveId++) {
        move = legalMoves.moves[moveId];

//        if (legalMoveCount && depth > 3) {
//            if (legalMoves.scores[moveId] < -3000)
//                board->depthAdjuster -= 4;
//            if (legalMoves.scores[moveId] < -2500)
//                board->depthAdjuster -= 3;
//            if (legalMoves.scores[moveId] < -2000)
//                board->depthAdjuster -= 2;
//        }

        //TODO tune this
#ifndef NO_LMR
//        if (legalMoveCount && depth <= 2 && (!getcapture(move)) && (!in_check) && !board->pvnode && ((staticeval + (legalMoves.scores[moveId] / 2)) < alpha) && !is_move_direct_check(move, board))
//            continue;

#endif
        //since king moves are super expensive (for the neural networks) many of them are discarded at depth 1.
//        if (depth == 1 && !board->pvnode && !in_check && legalMoves.scores[moveId] < -2700 && (getpiece(move) == p_K || getpiece(move) == p_k))
//            continue;

        //The illigal moves (moving pinned pieces mostly) are not removed during the move generation, they are removed here.
        if (make_move(move, all_moves, 1, board)) {

            legalMoveCount++;

            if (legalMoveCount == 0) {
                //Pv Search
                eval = -search(depth - 1, -beta, -alpha, &line, thread);
            } else {
                //Late Move Reduction
                if ((depth >= 3) && (legalMoves.scores[moveId] < 0) && (in_check == 0) && (getcapture(move) == 0) && (!isPastPawnPush)) {
#ifndef NO_LMR
//                    board->depthAdjuster += (float )legalMoves.scores[moveId] / 4000;
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
                shift_moveeval(&best);
                best.move[0] = move;
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

                    RecordHash(depth, beta, &best, hashfBETA, pline, board);

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

    RecordHash(depth, alpha, &best, hashf, pline, board);
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

//This function allocates memory for `HelperThread *threads`, so don't do it yourself.
void generate_threads(HelperThread **threads, int32_t *numThreads, Thread *thread){
    Board *board = &thread->board;
    int32_t tmp[4] = {0,0,0,0};
    MoveList legalMoves;
    generate_moves(&legalMoves, board);
    generate_only_legal_moves(&legalMoves, board);
    sort_moves(&legalMoves, tmp, thread);
    *numThreads = threadCount < legalMoves.count ? (int) threadCount : (int) legalMoves.count;

    *threads = malloc(sizeof(HelperThread) * threadCount);

    if (threadCount > 1) {
        memset(*threads, 0, sizeof (HelperThread) * (*numThreads));

        for (int32_t i = 0; i < *numThreads; ++i) {
            (*threads)[i].args = (struct NegamaxArgs) {.pline = &(*threads)[i].line, .thread.board = *board};
            make_move(legalMoves.moves[i], all_moves, 1, &(*threads)[i].args.thread.board);

            pthread_create(&(*threads)[i].pthread, NULL, (void *(*)(void *)) negamax_thread, &(*threads)[i].args);
        }
    }
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

    reset_hash_table();

    HelperThread *threads = NULL;
    int32_t numThreads;
    generate_threads(&threads, &numThreads, &thread);

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
            thread.board.depthAdjuster = 0;

            if (currentDepth > 2)
                aspwindow *= 2;

            printf("info string aspiration research. Window = %d\n", aspwindow/64);

            alpha = DEF_ALPHA;
            beta = DEF_BETA;

            memset(&negamax_line, 0, sizeof negamax_line);

            nmRes = search(currentDepth, alpha, beta, &negamax_line, &thread);

        } else {
            aspwindow -= (aspwindow/4);
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

                floatingMoveTime += floatingMoveTime * (sqrtf(floatingAspWindow) / 100);
                moveTime = (int) floatingMoveTime;
            }

            if (board->side == white)
                moveTime = min(wtime / 2, moveTime);
            else
                moveTime = min(btime / 2, moveTime);
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

    free(threads);

    dynamicTimeManagment = 0;
    tbsearch = 0;

    printf("bestmove ");
    print_move(board->prevPv.moves[0]);
    printf("\n");

    return NULL;
}
