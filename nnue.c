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

#define TRANSFORMERSTART ((3 * 4) + 181)

alignas(64) int16_t nnue_in_weights[NNUE_INSIZE * NNUE_KPSIZE ];
alignas(64) int8_t nnue_l1_weights[NNUE_L1SIZE * NNUE_L2SIZE ];
alignas(64) int8_t nnue_l2_weights[NNUE_L2SIZE * NNUE_L3SIZE ];
alignas(64) int16_t nnue_l3_weights[NNUE_L3SIZE * NNUE_OUTSIZE];

alignas(64) int16_t nnue_in_biases[NNUE_KPSIZE ];
alignas(64) int32_t nnue_l1_biases[NNUE_L2SIZE ];
alignas(64) int32_t nnue_l2_biases[NNUE_L3SIZE ];
alignas(64) int32_t nnue_l3_biases[NNUE_OUTSIZE];

void transform_weight_indicies(int8_t arr[], uint32_t dims){
    int8_t tmpArr[dims*32];
    memcpy(tmpArr, arr, sizeof tmpArr);

    for (int32_t r = 0; r < 32; ++r) {
        for (int32_t c = 0; c < dims; ++c) {
            arr[(c * 32) + r] = tmpArr[(dims*r) + c];
        }
    }
}

int32_t load_nnue(const char *path){
    //avoid compiler warnings
    unsigned long tmp;

    FILE *fin = fopen(path, "rb");

    //FEATURE TRANSFORMER
//    fseek(fin, TRANSFORMERSTART, SEEK_SET);

    tmp = fread(nnue_in_weights, sizeof(int16_t), NNUE_INSIZE * NNUE_KPSIZE, fin);
    tmp = fread(nnue_l1_weights, sizeof (nnue_l1_weights[0]), NNUE_L1SIZE * NNUE_L2SIZE, fin);
    tmp = fread(nnue_l2_weights, sizeof (nnue_l2_weights[0]), NNUE_L2SIZE * NNUE_L3SIZE, fin);
    tmp = fread(nnue_l3_weights, sizeof (nnue_l3_weights[0]), NNUE_L2SIZE * NNUE_OUTSIZE, fin);

    tmp = fread(nnue_in_biases, sizeof(int16_t), NNUE_KPSIZE, fin);
    tmp = fread(nnue_l1_biases, sizeof (nnue_l1_biases[0]), NNUE_L2SIZE, fin);
    tmp = fread(nnue_l2_biases, sizeof (nnue_l2_biases[0]), NNUE_L3SIZE, fin);
    tmp = fread(nnue_l3_biases, sizeof (nnue_l3_biases[0]), NNUE_OUTSIZE, fin);

    fclose(fin);

    for (int32_t i = 0; i < NnueHashSize; ++i) {
        evalHashTable[i].eval = NO_EVAL;
    }

    return 0;
}

int flipPiece[12] = {p_p, p_n, p_b, p_r, p_q, p_k, p_P, p_N, p_B, p_R, p_Q, p_K};

static inline void append_index(int32_t index, NnueData *data, int c) {
    data->activeIndicies[c][data->activeIndexCount[c]++] = index;
}

void append_active_indicies(NnueData *data, Board *board) {
    data->activeIndexCount[0] = 0;
    data->activeIndexCount[1] = 0;

    memset(data->activeIndicies, 0, sizeof(data->activeIndicies));

    for (uint32_t ptype = p_P; ptype <= p_k; ++ptype) {

        U64 bitboard = board->bitboards[ptype];
        while (bitboard) {
            int32_t bit = bsf(bitboard);
            int32_t sq = bit;

            append_index((64*ptype) + sq, data, white);
            append_index((64*flipPiece[ptype]) + w_orient[sq], data, black);

            pop_bit(bitboard, bit);
        }
    }

}

//vectorised by the compiler
void add_index(int16_t *restrict acc, uint32_t index, uint32_t c) {
    uint32_t offset = NNUE_KPSIZE * index;
    int16_t *restrict w = nnue_in_weights + offset;
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
    int16_t *restrict w = nnue_in_weights + offset;
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

    memcpy(data->accumulation[0], nnue_in_biases, NNUE_KPSIZE * sizeof(int16_t));
    memcpy(data->accumulation[1], nnue_in_biases, NNUE_KPSIZE * sizeof(int16_t));

    for (size_t k = 0; k < data->activeIndexCount[white]; k++) {
        uint32_t index = data->activeIndicies[white][k];
        add_index(data->accumulation, index, white);
    }

    for (size_t k = 0; k < data->activeIndexCount[black]; k++) {
        uint32_t index = data->activeIndicies[black][k];
        add_index(data->accumulation, index, black);
    }
}


void clamp_layer(int32_t *layer){
#ifdef AVX2

    __m256i _8128 = _mm256_set1_epi32(8128*2);
    __m256i _0 = _mm256_set1_epi32(0);

    for (int32_t i = 0; i < 32; i += 8) {
        __m256i _x = _mm256_load_si256((void*)&layer[i]);
        _x = _mm256_min_epi32(_x, _8128);
        _x = _mm256_max_epi32(_x, _0);
        _x = _mm256_srli_epi32(_x, 7);

        _mm256_storeu_si256((__m256i*)&layer[i], _x);
    }

#else

    for (int32_t i = 0; i < 32; ++i) {
        layer[i] = clamp(layer[i], 0, 8128);
        layer[i] /= 64;
    }

#endif
}

void clamp_accumulator(int16_t *acc){
#if defined(AVX2)

    __m256i _127 = _mm256_set1_epi16(127);
    __m256i _0 = _mm256_set1_epi16(0);

    for (int32_t i = 0; i < L1_SIZE; i += 16) {
        __m256i _x = _mm256_load_si256((void*)&acc[i]);
        _x = _mm256_min_epi16(_x, _127);
        _x = _mm256_max_epi16(_x, _0);
        _mm256_storeu_si256((__m256i*)&acc[i], _x);
    }

#else

    for (int32_t i = 0; i < L1_SIZE; ++i) {
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


void propogate_l1(NnueData *data, Board *board) {
    int16_t *tmpAccum = data->tmpAccumulation;

    memcpy(tmpAccum, data->accumulation, sizeof data->tmpAccumulation);
    clamp_accumulator(tmpAccum);

    memcpy(data->l1, nnue_l1_biases, sizeof nnue_l1_biases);

    for (int32_t i = 0; i < NNUE_L1SIZE; ++i) {
        if (tmpAccum[i]) {
            int32_t offset = 32 * i;
            propogate_neuron(tmpAccum[i], &nnue_l1_weights[offset], data->l1);
        }
    }

    clamp_layer(data->l1);
}

void propogate_l2(NnueData *data){
    memcpy(data->l2, nnue_l2_biases, sizeof nnue_l2_biases);

    for (int32_t o = 0; o < 32; ++o) {
        if (!data->l1[o])
            continue;

        int32_t offset = 32 * o;
        propogate_neuron((short )data->l1[o], &nnue_l2_weights[offset], data->l2);
    }

    clamp_layer(data->l2);
}

void propogate_l3(NnueData *data){
    memcpy(data->l3, nnue_l3_biases, sizeof nnue_l3_biases);
    for (int32_t i = 0; i < 32; ++i) {
        data->l3[0] += data->l2[i] * nnue_l3_weights[i];
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

    uint32_t hashIndex = board->current_zobrist_key % (NnueHashSize);
    EvalHashEntry *hashptr = &evalHashTable[hashIndex];
    NnueData *data = &board->currentNnue;

    if (evalHashTable[hashIndex].key == board->current_zobrist_key){
        data->eval = hashptr->eval;
    } else {
        propogate_l1(data, board);
        propogate_l2(data);
        propogate_l3(data);

        int eval = (int)(((((float)data->l3[0] / 127) / 127) * 410) * 64);

        data->eval = (board->side == white) ? eval : -eval;

        hashptr->eval = data->eval;
        hashptr->key = board->current_zobrist_key;
    }

//    convert winning advantages into material rather than activity
    if (data->eval > (400*64) && (board->side == board->searchColor)){
        int32_t mat = materialScore(board);
        mat = mat > 0 ? mat + 1 : 1;
        data->eval *= mat;
    } else if (data->eval < (400*64) && (board->side != board->searchColor)){
        int32_t mat = -materialScore(board);
        mat = mat > 0 ? mat + 1 : 1;
        data->eval *= mat;
    }

    return data->eval;
}

void nnue_pop_bit(int32_t ptype, int32_t bit, Board *board){

    if (!board->networkUpdate)
        return;

    int32_t w_ksq = w_orient[bsf(board->bitboards[p_K])];
    int32_t b_ksq = b_orient[bsf(board->bitboards[p_k])];

    int32_t sq = bit;
    int32_t pc = ptype;

    int32_t wi = (64*pc) + sq;
    int32_t bi = (64*flipPiece[pc]) + w_orient[sq];

    subtract_index(board->currentNnue.accumulation, wi, white);
    subtract_index(board->currentNnue.accumulation, bi, black);
}

void nnue_set_bit(int32_t ptype, int32_t bit, Board *board){

    if (!board->networkUpdate)
        return;

    int32_t w_ksq = w_orient[bsf(board->bitboards[p_K])];
    int32_t b_ksq = b_orient[bsf(board->bitboards[p_k])];

    int32_t sq = bit;
    int32_t pc = ptype;

    int32_t wi = (64*pc) + sq;
    int32_t bi = (64*flipPiece[pc]) + w_orient[sq];

    add_index(board->currentNnue.accumulation, wi, white);
    add_index(board->currentNnue.accumulation, bi, black);
}
