//
// Created by levigibson on 6/17/21.
//

#include <stdio.h>
#include <pthread.h>
#include "transposition.h"
#include "board.h"
#include "uci.h"

HASHE *hash_table = NULL;
int32_t *lines = NULL;
int32_t lineMoveCount = 0;

pthread_mutex_t lmcLock;
pthread_mutex_t *ttLocks = NULL;

void lock(pthread_mutex_t *lock){
    if (threadCount > 1)
        pthread_mutex_lock(lock);
}

void unlock(pthread_mutex_t *lock){
    if (threadCount > 1)
        pthread_mutex_unlock(lock);
}

void reset_hash_table(){
    memset(hash_table, 0, sizeof(HASHE)*tt_size);
    memset(lines, 0, sizeof(int32_t)*tt_size*6);
    lineMoveCount = 0;
}

void reinit_transposition(){
    if (hash_table) {
        free(hash_table);
    }

    if (lines){
        free(lines);
        free(ttLocks);
    }

    hash_table = malloc(sizeof(HASHE)*tt_size);
    lines = malloc(sizeof(int32_t)*tt_size*6);
    ttLocks = malloc(sizeof (pthread_mutex_t)*tt_size);

    pthread_mutex_init(&lmcLock, NULL);
    for (int32_t i = 0; i < tt_size; ++i) {
        pthread_mutex_init(&ttLocks[i], NULL);
    }

    reset_hash_table();
}

void transposition_free(){
    free(lines);
    free(hash_table);

    pthread_mutex_destroy(&lmcLock);
    for (int32_t i = 0; i < tt_size; ++i) {
        pthread_mutex_destroy(&ttLocks[i]);
    }
}

void recover_line(int32_t depth, HASHE *phashe, Line *pline, int32_t alpha, int32_t *ret, Board *board){
    if (depth <= 1 || phashe->value <= alpha || phashe->line == NULL) {
        return;
    }

    int* line = phashe->line;

    memcpy(pline->moves, &line[1], sizeof(int) * (*line));
    pline->length = *line;
}


int32_t ProbeHash(int32_t depth, int32_t alpha, int32_t beta, int32_t *move, Line *pline, Board *board){
    uint32_t index = board->current_zobrist_key % tt_size;
    HASHE * phashe = &hash_table[index];
    int32_t ret = valUNKNOWN;

    lock(&ttLocks[index]);

    if (phashe->key == board->current_zobrist_key) {
        memcpy(move, phashe->best, sizeof(phashe->best));

        if (phashe->depth >= depth) {

            if (phashe->flags == hashfEXACT) {
                ret = phashe->value;
                recover_line(depth, phashe, pline, alpha, &ret, board);
            }
            else if ((phashe->flags == hashfALPHA) && (phashe->value <= alpha)) {
                ret = alpha;
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

void RecordHash(int32_t depth, int32_t val, MoveEval *best, int32_t hashf, Line *pline, Board *board){
    if ((val != 0) && (val < 4500000) && (val > -450000)) {
        uint32_t index = board->current_zobrist_key % tt_size;

        lock(&ttLocks[index]);

        HASHE *phashe = &hash_table[index];
        phashe->key = board->current_zobrist_key;
        phashe->value = val;
        phashe->flags = hashf;
        phashe->depth = depth;
        phashe->line = NULL;

//        if (phashe->best[0] != best){
//            phashe->best[3] = phashe->best[2];
//            phashe->best[2] = phashe->best[1];
//            phashe->best[1] = phashe->best[0];
//            phashe->best[0] = best;
//        }
        memcpy(&phashe->best[0], &best->move[0], sizeof(best->move));

        if (threadCount > 1 && depth > 1 && pline != NULL) {
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

