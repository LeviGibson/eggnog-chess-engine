//
// Created by levigibson on 6/17/21.
//

#include <stdio.h>
#include <pthread.h>
#include "transposition.h"
#include "board.h"
#include "uci.h"

HASHE hash_table[tt_size];
int lines[tt_linesize];
int lineMoveCount = 0;

pthread_mutex_t lmcLock;
pthread_mutex_t ttLocks[tt_size];

void lock(pthread_mutex_t *lock){
    if (threadCount > 1)
        pthread_mutex_lock(lock);
}

void unlock(pthread_mutex_t *lock){
    if (threadCount > 1)
        pthread_mutex_unlock(lock);
}

void reset_hash_table(){
    memset(hash_table, 0, sizeof hash_table);
    memset(lines, 0, sizeof lines);
    lineMoveCount = 0;
}

void init_transposition(){
    pthread_mutex_init(&lmcLock, NULL);
    for (int i = 0; i < tt_size; ++i) {
        pthread_mutex_init(&ttLocks[i], NULL);
    }

    reset_hash_table();
}

void transposition_free(){
    pthread_mutex_destroy(&lmcLock);
    for (int i = 0; i < tt_size; ++i) {
        pthread_mutex_destroy(&ttLocks[i]);
    }
}

void recover_line(int depth, HASHE *phashe, Line *pline, int alpha, int *ret, Board *board){
    if (depth <= 1 || phashe->value <= alpha || phashe->line == NULL) {
        return;
    }

    int* line = phashe->line;

    memcpy(pline->moves, &line[1], sizeof(int) * (*line));
    pline->length = *line;
}

int ProbeHash(int depth, int alpha, int beta, int *move, int *staticeval, Line *pline, Board *board){
    unsigned index = board->current_zobrist_key % tt_size;
    HASHE * phashe = &hash_table[index];
    int ret = valUNKNOWN;

    lock(&ttLocks[index]);

    if (phashe->key == board->current_zobrist_key) {
//        *move = phashe->best;
        memcpy(move, phashe->best, sizeof(phashe->best));
        *staticeval = phashe->staticeval;

        if (phashe->depth >= depth) {

            if (phashe->flags == hashfEXACT) {
                ret = phashe->value;
                recover_line(depth, phashe, pline, alpha, &ret, board);
            }
            else if ((phashe->flags == hashfALPHA) && (phashe->value <= alpha)) {
                ret =  alpha;
                recover_line(depth, phashe, pline, alpha, &ret, board);
            }
            else if ((phashe->flags == hashfBETA) && (phashe->value >= beta)) {
                ret =  beta;
                recover_line(depth, phashe, pline, alpha, &ret, board);
            }
        }
    }

    unlock(&ttLocks[index]);

    return ret;
}

void RecordHash(int depth, int val, int best, int hashf, int staticeval, Line *pline, Board *board){
    if ((val != 0) && (val < 4500000) && (val > -450000)) {
        unsigned index = board->current_zobrist_key % tt_size;

        lock(&ttLocks[index]);

        HASHE *phashe = &hash_table[index];
        phashe->key = board->current_zobrist_key;
        phashe->value = val;
        phashe->staticeval = staticeval;
        phashe->flags = hashf;
        phashe->depth = depth;
        phashe->line = NULL;

        if (phashe->best[0] != best){
            phashe->best[3] = phashe->best[2];
            phashe->best[2] = phashe->best[1];
            phashe->best[1] = phashe->best[0];
            phashe->best[0] = best;
        }

        if (depth > 1 && pline != NULL) {
            lock(&lmcLock);

            if (lineMoveCount > (tt_linesize - 20)){
                printf("info string hash table reset\n");
                reset_hash_table();
            }

            phashe->line = &lines[lineMoveCount];
            lines[lineMoveCount] = pline->length;
            memcpy(&lines[lineMoveCount + 1], &pline->moves, sizeof(int) * pline->length);
            lineMoveCount += pline->length + 1;

            unlock(&lmcLock);
        }

        unlock(&ttLocks[index]);
    }
}

