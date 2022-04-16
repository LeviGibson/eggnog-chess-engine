//
// Created by levigibson on 10/3/21.
//
#include "nnue.h"
#include <string.h>
#include <stdio.h>

#ifdef AVX2
#include <immintrin.h>
#endif

typedef struct EvalHashEntry EvalHashEntry;

struct EvalHashEntry{
    U64 key;
    int32_t eval;
};

#define NnueHashSize 1000000
EvalHashEntry evalHashTable[NnueHashSize];

alignas(64) int16_t nnue_l1_weights[NNUE_INSIZE * NNUE_KPSIZE ];
alignas(64) int8_t nnue_l2_weights[NNUE_L1SIZE * NNUE_L2SIZE ];
alignas(64) int8_t nnue_l3_weights[NNUE_L2SIZE * NNUE_L3SIZE ];
alignas(64) int8_t nnue_l4_weights[NNUE_L3SIZE * NNUE_L4SIZE];

alignas(64) int16_t nnue_l1_biases[NNUE_KPSIZE ];
alignas(64) int32_t nnue_l2_biases[NNUE_L2SIZE ];
alignas(64) int32_t nnue_l3_biases[NNUE_L3SIZE ];
alignas(64) int32_t nnue_l4_biases[NNUE_L4SIZE];

int32_t load_nnue(const char *path) {
    //avoid compiler warnings
    unsigned long tmp;

    FILE *fin = fopen(path, "rb");

    //FEATURE TRANSFORMER

    tmp = fread(nnue_l1_weights, sizeof(int16_t), NNUE_INSIZE * NNUE_KPSIZE, fin);
    tmp = fread(nnue_l2_weights, sizeof(nnue_l2_weights[0]), NNUE_L1SIZE * NNUE_L2SIZE, fin);
    tmp = fread(nnue_l3_weights, sizeof(nnue_l3_weights[0]), NNUE_L2SIZE * NNUE_L3SIZE, fin);
    tmp = fread(nnue_l4_weights, sizeof(nnue_l4_weights[0]), NNUE_L2SIZE * NNUE_L4SIZE, fin);

    tmp = fread(nnue_l1_biases, sizeof(int16_t), NNUE_KPSIZE, fin);
    tmp = fread(nnue_l2_biases, sizeof(nnue_l2_biases[0]), NNUE_L2SIZE, fin);
    tmp = fread(nnue_l3_biases, sizeof(nnue_l3_biases[0]), NNUE_L3SIZE, fin);
    tmp = fread(nnue_l4_biases, sizeof(nnue_l4_biases[0]), NNUE_L4SIZE, fin);

    fclose(fin);

    for (int32_t i = 0; i < NnueHashSize; ++i) {
        evalHashTable[i].eval = NO_EVAL;
    }

    return 0;
}

int32_t flip_piece_pers[12] = {p_p, p_n, p_b, p_r, p_q, p_k, p_P, p_N, p_B, p_R, p_Q, p_K};

static inline void append_index(int32_t c, int32_t index, NnueData *data) {
    data->activeIndicies[c][data->activeIndexCount[c]++] = index;
}

void append_active_indicies(NnueData *data, Board *board) {
    data->activeIndexCount[black] = 0;
    data->activeIndexCount[white] = 0;

    for (int32_t ptype = p_P; ptype < p_k; ++ptype) {

        U64 bitboard = board->bitboards[ptype];
        while (bitboard) {
            int32_t sq = bsf(bitboard);

            if (ptype < 6)
                append_index(white, ptype*64 + sq, data);
            else
                append_index(black, (ptype-6)*64 + w_orient[sq], data);

            pop_bit(bitboard, sq);
        }
    }
}

//vectorised by the compiler
void add_index(int16_t *restrict acc, uint32_t index, uint32_t c) {
    uint32_t offset = NNUE_KPSIZE * index;
    int16_t *restrict w = nnue_l1_weights + offset;
    acc += NNUE_KPSIZE*c;

#ifdef AVX2

    for (int32_t j = 0 ; j < NNUE_KPSIZE ; j += 16) {
        __m256i _x = _mm256_loadu_si256((void*)&acc[j]);
        __m256i _y = _mm256_loadu_si256((void*)&w[j]);
        _mm256_storeu_si256((__m256i*)&acc[j], _mm256_add_epi16(_x, _y));
    }

#else

    for (uint16_t j = 0; j < NNUE_KPSIZE; j++)
        acc[j] += w[j];

#endif
}

//vectorised by the compiler
void subtract_index(int16_t *restrict acc, uint32_t index, uint32_t c) {
    uint32_t offset = NNUE_KPSIZE * index;
    int16_t *restrict w = nnue_l1_weights + offset;
    acc += NNUE_KPSIZE*c;

#ifdef AVX2

    for (int32_t j = 0 ; j < NNUE_KPSIZE ; j += 16) {
        __m256i _x = _mm256_loadu_si256((void*)&acc[j]);
        __m256i _y = _mm256_loadu_si256((void*)&w[j]);
        _mm256_storeu_si256((__m256i*)&acc[j], _mm256_sub_epi16(_x, _y));
    }

#else

    for (uint16_t j = 0; j < NNUE_KPSIZE; j++)
        acc[j] -= w[j];

#endif
}

void refresh_accumulator(NnueData *data, Board *board) {
    append_active_indicies(data, board);

    for (uint32_t c = 0; c < 2; c++) {
        memcpy(data->accumulation[c], nnue_l1_biases, NNUE_KPSIZE * sizeof(int16_t));

        for (size_t k = 0; k < data->activeIndexCount[c]; k++) {
            uint32_t index = data->activeIndicies[c][k];
            add_index(data->accumulation[0], index, c);
        }
    }
}


void clamp_layer(int32_t *layer){
#ifdef AVX2

    __m256i _16256 = _mm256_set1_epi32(16256);
    __m256i _0 = _mm256_set1_epi32(0);

    for (int32_t i = 0; i < 32; i += 8) {
        __m256i _x = _mm256_load_si256((void*)&layer[i]);
        _x = _mm256_min_epi32(_x, _16256);
        _x = _mm256_max_epi32(_x, _0);
        _x = _mm256_srli_epi32(_x, 7);

        _mm256_storeu_si256((__m256i*)&layer[i], _x);
    }

#else

    for (int32_t i = 0; i < 32; ++i) {
        layer[i] = clamp(layer[i], 0, 16256);
        layer[i] /= 128;
    }

#endif
}

void clamp_accumulator(int16_t *acc){
#if defined(AVX2)

    __m256i _127 = _mm256_set1_epi16(127);
    __m256i _0 = _mm256_set1_epi16(0);

    for (int32_t i = 0; i < NNUE_L1SIZE; i += 16) {
        __m256i _x = _mm256_load_si256((void*)&acc[i]);
        _x = _mm256_min_epi16(_x, _127);
        _x = _mm256_max_epi16(_x, _0);
        _mm256_storeu_si256((__m256i*)&acc[i], _x);
    }

#else

    for (int32_t i = 0; i < NNUE_KPSIZE; ++i) {
        acc[i] = clamp(acc[i], 0, 127);
    }

#endif
}

static inline void propogate_neuron(const int16_t a, const int8_t *b, int32_t *restrict c) {

#ifdef AVX2
    __m256i va = _mm256_set1_epi16(a);

    for (int32_t i = 0 ; i < 32 ; i += 16) {
        __m256i vb = _mm256_cvtepi8_epi16( _mm_load_si128((__m128i*)&b[i]) );
        __m256i prod = _mm256_mullo_epi16(va, vb);

        __m256i v1 = _mm256_cvtepi16_epi32((__m128i)_mm256_castps256_ps128((__m256)prod));
        __m256i v2 = _mm256_cvtepi16_epi32((__m128i)_mm256_extractf128_ps((__m256)prod, 1));

        __m256i sum1 = _mm256_add_epi32(v1, _mm256_loadu_si256((const __m256i*)&c[i]));
        __m256i sum2 = _mm256_add_epi32(v2, _mm256_loadu_si256((const __m256i*)&c[i+8]));

        _mm256_storeu_si256((__m256i*)&c[i], sum1);
        _mm256_storeu_si256((__m256i*)&c[i+8], sum2);
    }
#else
    for (int32_t i = 0; i < 32; ++i)
        c[i] += a * b[i];
#endif

}

void propogate_l1(NnueData *data) {
    int16_t *tmpAccum = data->l1;

    memcpy(tmpAccum, data->accumulation, sizeof(data->l1));
    memcpy(data->l2, nnue_l2_biases, sizeof(nnue_l2_biases));

    clamp_accumulator(tmpAccum);

    for (int32_t i = 0; i < NNUE_L1SIZE; ++i) {
        if (tmpAccum[i]) {
            int32_t offset = 32 * i;
            propogate_neuron(tmpAccum[i], &nnue_l2_weights[offset], data->l2);
        }
    }

    clamp_layer(data->l2);
}

void propogate_l2(NnueData *data){
    memcpy(data->l3, nnue_l3_biases, sizeof nnue_l3_biases);

    for (int32_t o = 0; o < 32; ++o) {
        if (!data->l2[o])
            continue;

        int32_t offset = 32 * o;
        propogate_neuron((short )data->l2[o], &nnue_l3_weights[offset], data->l3);
    }

    clamp_layer(data->l3);
}

void propogate_l3(NnueData *data){
    memcpy(data->l4, nnue_l4_biases, sizeof nnue_l4_biases);
    for (int32_t i = 0; i < 32; ++i) {
        data->l4[0] += data->l3[i] * nnue_l4_weights[i];
    }
}

int32_t materialScore(Board *board){
    int32_t eval = 0;

    eval += 1 * count_bits(board->bitboards[p_P]);
    eval += 3 * count_bits(board->bitboards[p_N]);
    eval += 3 * count_bits(board->bitboards[p_B]);
    eval += 5 * count_bits(board->bitboards[p_R]);
    eval += 10 * count_bits(board->bitboards[p_Q]);

    eval -= 1 * count_bits(board->bitboards[p_p]);
    eval -= 3 * count_bits(board->bitboards[p_n]);
    eval -= 3 * count_bits(board->bitboards[p_b]);
    eval -= 5 * count_bits(board->bitboards[p_r]);
    eval -= 10 * count_bits(board->bitboards[p_q]);

    return board->side == white ? eval : -eval;
}

int32_t nnue_evaluate(Board *board) {

//    uint32_t hashIndex = board->current_zobrist_key % (NnueHashSize);
//    EvalHashEntry *hashptr = &evalHashTable[hashIndex];
    NnueData *data = &board->currentNnue;

//    if (evalHashTable[hashIndex].key == board->current_zobrist_key){
//        data->eval = hashptr->eval;
//    } else {
    propogate_l1(data);
    propogate_l2(data);
    propogate_l3(data);
    data->eval = (board->side == white) ? data->l4[0] : -data->l4[0];

//        hashptr->eval = data->eval;
//        hashptr->key = board->current_zobrist_key;
//    }

    //convert winning advantages into material rather than activity
//    if (data->eval > (180*64) && (board->side == board->searchColor)){
//        int32_t mat = materialScore(board);
//        mat = mat > 0 ? mat + 1 : 1;
//        data->eval *= mat;
//    } else if (data->eval < (180*64) && (board->side != board->searchColor)){
//        int32_t mat = -materialScore(board);
//        mat = mat > 0 ? mat + 1 : 1;
//        data->eval *= mat;
//    }

    return data->eval;
}

void nnue_pop_bit(int32_t ptype, int32_t bit, Board *board){

    if (!board->nnueUpdate)
        return;

    if (ptype < 6)
        subtract_index(board->currentNnue.accumulation[0], ptype*64 + bit, white);
    else
        subtract_index(board->currentNnue.accumulation[0], (ptype-6)*64 + w_orient[bit], black);
}

void nnue_set_bit(int32_t ptype, int32_t bit, Board *board){

    if (!board->nnueUpdate)
        return;

    if (ptype < 6)
        add_index(board->currentNnue.accumulation[0], ptype*64 + bit, white);
    else
        add_index(board->currentNnue.accumulation[0], (ptype-6)*64 + w_orient[bit], black);
}
