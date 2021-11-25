//
// Created by levigibson on 6/17/21.
//

#include <stdio.h>
#include <pthread.h>
#include "transposition.h"
#include "board.h"

HASHE hash_table[tt_size];
int lines[tt_linesize];
int lineMoveCount = 0;

pthread_mutex_t lmcLock;
pthread_mutex_t ttLocks[tt_size];

void reset_hash_table(){
    memset(hash_table, 0, sizeof hash_table);
    memset(lines, 0, sizeof lines);
    lineMoveCount = 0;
}

void init_transposition(){
    reset_hash_table();
    pthread_mutex_init(&lmcLock, NULL);
    for (int i = 0; i < tt_size; ++i) {
        pthread_mutex_init(&ttLocks[i], NULL);
    }
}

void transposition_free(){
    pthread_mutex_destroy(&lmcLock);
    for (int i = 0; i < tt_size; ++i) {
        pthread_mutex_destroy(&ttLocks[i]);
    }
}

void recover_line(int depth, HASHE *phashe, Line *pline){
    if (depth <= 1) {
        return;
    }

    int* line = phashe->line;

    memcpy(pline->moves, &line[1], sizeof(int) * (*line));
    pline->length = *line;
}

int ProbeHash(int depth, int alpha, int beta, int *move, Line *pline, Board *board){
    unsigned index = board->current_zobrist_key % tt_size;
    HASHE * phashe = &hash_table[index];
    int ret = valUNKNOWN;

    pthread_mutex_lock(&ttLocks[index]);

    if (phashe->key == board->current_zobrist_key) {
        *move = phashe->best;

        if (phashe->depth >= depth) {

            if (phashe->flags == hashfEXACT) {
                recover_line(depth, phashe, pline);
                ret = phashe->value;
            }
            else if ((phashe->flags == hashfALPHA) && (phashe->value <= alpha)) {
                recover_line(depth, phashe, pline);
                ret =  alpha;
            }
            else if ((phashe->flags == hashfBETA) && (phashe->value >= beta)) {
                recover_line(depth, phashe, pline);
                ret =  beta;
            }
        }
    }

    pthread_mutex_unlock(&ttLocks[index]);

    return ret;
}

void RecordHash(int depth, int val, int best, int hashf, Line *pline, Board *board){
    //U64 key = generate_zobrist_key();
    if ((val != 0) && (val < 40000) && (val > -40000)) {
        unsigned index = board->current_zobrist_key % tt_size;

        pthread_mutex_lock(&ttLocks[index]);

        HASHE *phashe = &hash_table[index];
        phashe->key = board->current_zobrist_key;
        phashe->best = best;
        phashe->value = val;
        phashe->flags = hashf;
        phashe->depth = depth;
        phashe->line = NULL;

        if (depth > 1) {
            pthread_mutex_lock(&lmcLock);

            if (lineMoveCount > (tt_linesize - 20)){
                printf("info string hash table reset\n");
                reset_hash_table();
            }

            phashe->line = &lines[lineMoveCount];
            lines[lineMoveCount] = pline->length;
            memcpy(&lines[lineMoveCount + 1], &pline->moves, sizeof(int) * pline->length);
            lineMoveCount += pline->length + 1;

            pthread_mutex_unlock(&lmcLock);
        }

        pthread_mutex_unlock(&ttLocks[index]);

//        locks[index] = 0;
    }
}

