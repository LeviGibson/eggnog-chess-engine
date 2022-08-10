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

#if defined(AVX2) || defined(AVX)
int16_t hadd_epi16(__m256i x) {
    const __m128i hiQuad = (__m128i)_mm256_extractf128_ps((__m256)x, 1);
    const __m128i loQuad = (__m128i)_mm256_castps256_ps128((__m256)x);
    const __m128i sumQuad = _mm_add_epi16(loQuad, hiQuad);
    const __m128i hiDual = (__m128i)_mm_movehl_ps((__m128)sumQuad, (__m128)sumQuad);
    const __m128i sumDual = _mm_add_epi16(sumQuad, hiDual);
    return _mm_extract_epi16(sumDual, 0) + _mm_extract_epi16(sumDual, 1) + _mm_extract_epi16(sumDual, 2) + _mm_extract_epi16(sumDual, 3);
}
#endif

int32_t calculate_l2_value(const int16_t *restrict l1, int32_t index){
#if defined(AVX2)
    __m256i _sum = _mm256_setzero_si256();
    __m256i _0 = _mm256_setzero_si256();

    for (int32_t i = 0; i < L1_SIZE; i += 16){
        __m256i _values = _mm256_loadu_si256(&l1[i]);
        __m256i _weights = _mm256_loadu_si256(&l2_weights[index][i]);

        _values = _mm256_max_epi16(_values, _0);
        _values = _mm256_srli_epi16(_values, 6);

        _values = _mm256_mullo_epi16(_values, _weights);
        _sum = _mm256_add_epi16(_sum, _values);
    }

    return l2_biases[index] + hadd_epi16(_sum);
#else
    int32_t score = l2_biases[index];

    for (int32_t i = 0; i < L1_SIZE; ++i) {
        int16_t l1value = max(0, l1[i]) / 64;
        score += l2_weights[index][i] * l1value;
    }

    return score;
#endif
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
    int32_t score = calculate_l2_value(data->l1[board->side], moveIndex);

    return score;
}

int flipPiecePers[12] = {p_p, p_n, p_b, p_r, p_q, p_k, p_P, p_N, p_B, p_R, p_Q, p_K};

void get_index(uint32_t  *i1, int32_t p, int32_t sq, int32_t wk, int32_t bk, int32_t side){
    if (side == black){
        p = flipPiecePers[p];
        sq = w_orient[sq];
    }

    if (p >= p_p)
        sq = w_orient[sq];

    *i1 = (64*p) + sq;
}

static inline void nnom_add_index(int16_t *restrict a, const int16_t *restrict b){
#if defined(AVX2)
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
#if defined(AVX2)
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
    if (board->quinode || board->seeNode)
        return;

    uint32_t i1;

    int32_t wk = bsf(board->bitboards[p_K]);
    int32_t bk = bsf(board->bitboards[p_k]);

    get_index(&i1, ptype, bit, wk, bk, white);

    nnom_add_index(board->nnom.l1[white], l1_weights[i1]);

    int32_t tmp = wk;
    wk = bk;
    bk = tmp;
    wk = w_orient[wk];
    bk = w_orient[bk];

    get_index(&i1, ptype, bit, wk, bk, black);

    nnom_add_index(board->nnom.l1[black], l1_weights[i1]);
}

//info score cp 16 depth 9 seldepth 22 nodes 179630 nps 343460 qnodes 84616 tbhits 0 time 523 pv c2c4 c7c5 b1c3 e7e5 g2g3 b8c6 f1g2 g8f6
//info score cp 16 depth 9 seldepth 22 nodes 179630 nps 346776 qnodes 84616 tbhits 0 time 518 pv c2c4 c7c5 b1c3 e7e5 g2g3 b8c6 f1g2 g8f6 

void nnom_pop_bit(int32_t ptype, int32_t bit, Board *board){
    if (board->quinode || board->seeNode)
        return;

    uint32_t i1;

    int32_t wk = bsf(board->bitboards[p_K]);
    int32_t bk = bsf(board->bitboards[p_k]);

    get_index(&i1, ptype, bit, wk, bk, white);

    nnom_subtract_index(board->nnom.l1[white], l1_weights[i1]);

    int32_t tmp = wk;
    wk = bk;
    bk = tmp;
    wk = w_orient[wk];
    bk = w_orient[bk];

    get_index(&i1, ptype, bit, wk, bk, black);

    nnom_subtract_index(board->nnom.l1[black], l1_weights[i1]);
}

void nnom_refresh_l1_helper(Board *board){
    if (board->quinode)
        return;

    NnomData *data = &board->nnom;
    memcpy(&data->l1[board->side], l1_biases, sizeof(l1_biases));
    generate_nnom_indicies(board);

    for (int32_t i = 0; i < data->indexCount; ++i) {
        uint32_t index = data->indicies[board->side][i];
        nnom_add_index(data->l1[board->side], l1_weights[index]);
    }
}

void nnom_refresh_l1(Board *board){
    nnom_refresh_l1_helper(board);
    board->side ^= 1;
    nnom_refresh_l1_helper(board);
    board->side ^= 1;
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

            get_index(&i1, p, square, wk, bk, board->side);
            board->nnom.indicies[board->side][board->nnom.indexCount] = i1;

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
