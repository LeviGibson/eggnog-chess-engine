//
// Created by levigibson on 3/23/22.
//

#include "nnom.h"
#include <stdio.h>
#include <stdalign.h>
#include <immintrin.h>

alignas(64) int16_t l1_weights[IN_SIZE][L1_SIZE];
alignas(64) int16_t l2_weights[L2_SIZE][L1_SIZE];

alignas(64) int16_t l1_biases[L1_SIZE];
alignas(64) int32_t l2_biases[L2_SIZE];

void nnom_refresh_l1_helper(Board *board){
    NnomData *data = &board->nnom;
    memcpy(&data->l1[board->side], l1_biases, sizeof(l1_biases));
    generate_nnom_indicies(board);

    for (int32_t i = 0; i < data->indexCount; ++i) {
        uint32_t index = data->indicies[board->side][i];
        for (int32_t j = 0; j < L1_SIZE; ++j) {
            data->l1[board->side][j] += l1_weights[index][j];
        }
    }
}

void nnom_refresh_l1(Board *board){
    nnom_refresh_l1_helper(board);
    board->side ^= 1;
    nnom_refresh_l1_helper(board);
    board->side ^= 1;
}

int32_t get_nnom_score(int move, Board *board){
    NnomData *data = &board->nnom;
    int orientedPiece = getpiece(move);
    int orientedSquare = gettarget(move);
    if (board->side == black){
        orientedPiece -= 6;
        orientedSquare = w_orient[orientedSquare];
    }
    int32_t moveIndex = (orientedPiece * 64) + orientedSquare;
    int32_t score = l2_biases[moveIndex];

    for (int32_t i = 0; i < L1_SIZE; ++i) {
        int16_t l1value = max(0, data->l1[board->side][i]) / 64;
        score += l2_weights[moveIndex][i] * l1value;
    }

    return score;
}



int flipPiecePers[12] = {p_p, p_n, p_b, p_r, p_q, p_k, p_P, p_N, p_B, p_R, p_Q, p_K};

void get_index(uint32_t  *i1, uint32_t *i2, int32_t p, int32_t sq, int32_t wk, int32_t bk, int32_t side){
    if (side == black){
        p = flipPiecePers[p];
        sq = w_orient[sq];
    }

    *i1 = wk + (768*p) + (64*sq);
    *i2 = bk + (768*p) + (64*sq) + 49152;
}

static inline void nnom_add_index(int16_t *restrict a, const int16_t *restrict b){
#ifdef AVX2
    for (int i = 0; i < L1_SIZE; i += 16) {
        __m256i _a = _mm256_loadu_si256(&a[i]);
        __m256i _b = _mm256_loadu_si256(&b[i]);
        _mm256_storeu_si256((__m256i *) &a[i], _mm256_add_epi16(_a, _b));
    }
#else
    for (int i = 0; i < L1_SIZE; ++i){
        a[i] += b[i];
    }
#endif
}

static inline void nnom_subtract_index(int16_t *restrict a, const int16_t *restrict b){
#ifdef AVX2
    for (int i = 0; i < L1_SIZE; i += 16) {
        __m256i _a = _mm256_loadu_si256(&a[i]);
        __m256i _b = _mm256_loadu_si256(&b[i]);
        _mm256_storeu_si256((__m256i *) &a[i], _mm256_sub_epi16(_a, _b));
    }
#else
    for (int i = 0; i < L1_SIZE; ++i){
        a[i] -= b[i];
    }
#endif
}

void nnom_set_bit(int32_t ptype, int32_t bit, Board *board){
    uint32_t i1;
    uint32_t i2;

    int32_t wk = bsf(board->bitboards[p_K]);
    int32_t bk = bsf(board->bitboards[p_k]);

    get_index(&i1, &i2, ptype, bit, wk, bk, white);

    nnom_add_index(board->nnom.l1[white], l1_weights[i1]);
    nnom_add_index(board->nnom.l1[white], l1_weights[i2]);

    int32_t tmp = wk;
    wk = bk;
    bk = tmp;
    wk = w_orient[wk];
    bk = w_orient[bk];

    get_index(&i1, &i2, ptype, bit, wk, bk, black);

    nnom_add_index(board->nnom.l1[black], l1_weights[i1]);
    nnom_add_index(board->nnom.l1[black], l1_weights[i2]);
}

void nnom_pop_bit(int32_t ptype, int32_t bit, Board *board){
    uint32_t i1;
    uint32_t i2;

    int32_t wk = bsf(board->bitboards[p_K]);
    int32_t bk = bsf(board->bitboards[p_k]);

    get_index(&i1, &i2, ptype, bit, wk, bk, white);

    nnom_subtract_index(board->nnom.l1[white], l1_weights[i1]);
    nnom_subtract_index(board->nnom.l1[white], l1_weights[i2]);

    int32_t tmp = wk;
    wk = bk;
    bk = tmp;
    wk = w_orient[wk];
    bk = w_orient[bk];

    get_index(&i1, &i2, ptype, bit, wk, bk, black);

    nnom_subtract_index(board->nnom.l1[black], l1_weights[i1]);
    nnom_subtract_index(board->nnom.l1[black], l1_weights[i2]);
}

void generate_nnom_indicies_helper(Board *board){
    board->nnom.indexCount = 0;

    int32_t wk = bsf(board->bitboards[p_K]);
    int32_t bk = bsf(board->bitboards[p_k]);

    if (board->side == black){
        wk = w_orient[wk];
        bk = w_orient[bk];

        int32_t tmp = wk;
        wk = bk;
        bk = tmp;
    }

    for (int p = 0; p < 12; ++p) {
        U64 bb = board->bitboards[p];
        while (bb){
            int32_t square = bsf(bb);

            uint32_t i1;
            uint32_t i2;

            get_index(&i1, &i2, p, square, wk, bk, board->side);
            board->nnom.indicies[board->side][board->nnom.indexCount] = i1;
            board->nnom.indicies[board->side][board->nnom.indexCount+1] = i2;

            board->nnom.indexCount += 2;

            pop_bit(bb, square);
        }
    }
}

void generate_nnom_indicies(Board *board){
    generate_nnom_indicies_helper(board);
    board->side ^= 1;
    generate_nnom_indicies_helper(board);
    board->side ^= 1;
}

int load_nnom(char *path){
    __attribute__((unused)) unsigned long tmp;

    FILE *fin = fopen(path, "rb");
    tmp = fread(l1_weights, sizeof(int16_t), IN_SIZE * L1_SIZE, fin);
    tmp = fread(l2_weights, sizeof(int16_t), L1_SIZE * L2_SIZE, fin);

    tmp = fread(l1_biases, sizeof(int16_t), L1_SIZE, fin);
    tmp = fread(l2_biases, sizeof(int16_t), L2_SIZE, fin);

    return 0;
}
