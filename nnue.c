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
alignas(64) int8_t nnue_l3_weights[NNUE_L3SIZE * NNUE_OUTSIZE];

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

int32_t load_model(const char *path){
    //avoid compiler warnings
    unsigned long tmp;

    FILE *fin = fopen(path, "rb");

    //FEATURE TRANSFORMER
    fseek(fin, TRANSFORMERSTART, SEEK_SET);
    tmp = fread(nnue_in_biases, sizeof(int16_t), NNUE_KPSIZE, fin);
    tmp = fread(nnue_in_weights, sizeof(int16_t), NNUE_INSIZE * NNUE_KPSIZE, fin);

    fseek(fin, 4, SEEK_CUR);

    //Hidden Layer 1
    tmp = fread(nnue_l1_biases, sizeof (nnue_l1_biases[0]), NNUE_L2SIZE, fin);
    tmp = fread(nnue_l1_weights, sizeof (nnue_l1_weights[0]), NNUE_L1SIZE * NNUE_L2SIZE, fin);
    transform_weight_indicies(nnue_l1_weights, NNUE_L1SIZE);

    //Hidden Layer 2
    tmp = fread(nnue_l2_biases, sizeof (nnue_l2_biases[0]), NNUE_L3SIZE, fin);
    tmp = fread(nnue_l2_weights, sizeof (nnue_l2_weights[0]), NNUE_L2SIZE * NNUE_L3SIZE, fin);
    transform_weight_indicies(nnue_l2_weights, NNUE_L2SIZE);

    //Output Layer
    tmp = fread(nnue_l3_biases, sizeof (nnue_l3_biases[0]), NNUE_OUTSIZE, fin);

    tmp = fread(nnue_l3_weights, sizeof (nnue_l3_weights[0]), NNUE_L2SIZE * NNUE_OUTSIZE, fin);

    fclose(fin);

    for (int32_t i = 0; i < NnueHashSize; ++i) {
        evalHashTable[i].eval = NO_EVAL;
    }

    return 0;
}

int32_t NnuePtypes[12] = {6, 5, 4, 3, 2, p_K, 12, 11, 10, 9, 8, p_k};



enum {
    PS_W_PAWN = 0,
    PS_B_PAWN = 1 * 64,
    PS_W_KNIGHT = 2 * 64,
    PS_B_KNIGHT = 3 * 64,
    PS_W_BISHOP = 4 * 64,
    PS_B_BISHOP = 5 * 64,
    PS_W_ROOK = 6 * 64,
    PS_B_ROOK = 7 * 64,
    PS_W_QUEEN = 8 * 64,
    PS_B_QUEEN = 9 * 64,
};

uint32_t PieceToIndex[2][14] = {
        {0, 0, PS_W_QUEEN, PS_W_ROOK, PS_W_BISHOP, PS_W_KNIGHT, PS_W_PAWN,
                0, PS_B_QUEEN, PS_B_ROOK, PS_B_BISHOP, PS_B_KNIGHT, PS_B_PAWN, 0},
        {0, 0, PS_B_QUEEN, PS_B_ROOK, PS_B_BISHOP, PS_B_KNIGHT, PS_B_PAWN,
                0, PS_W_QUEEN, PS_W_ROOK, PS_W_BISHOP, PS_W_KNIGHT, PS_W_PAWN, 0}
};


static inline int32_t orient(int32_t c, int32_t s) {
    return s ^ (c == white ? 0x00 : 0x3f);
}

static inline int32_t make_index(int32_t c, int32_t s, int32_t pc, int32_t ksq) {
    return orient(c, s) + PieceToIndex[c][pc] + ((10*64+1) * ksq) + 1;
}

static inline void append_index(int32_t c, int32_t index, NnueData *data) {
    data->activeIndicies[c][data->activeIndexCount[c]++] = index;
}

void append_active_indicies(NnueData *data, Board *board) {
    data->activeIndexCount[black] = 0;
    data->activeIndexCount[white] = 0;

    int32_t w_ksq = w_orient[bsf(board->bitboards[p_K])];
    int32_t b_ksq = b_orient[bsf(board->bitboards[p_k])];

    for (uint32_t ptype = p_P; ptype < p_k; ++ptype) {
        if (ptype == p_K)
            continue;

        U64 bitboard = board->bitboards[ptype];
        while (bitboard) {
            int32_t bit = bsf(bitboard);
            int32_t sq = w_orient[bit];
            int32_t pc = NnuePtypes[ptype];

            append_index(white, make_index(white, sq, pc, w_ksq), data);
            append_index(black, make_index(black, sq, pc, b_ksq), data);

            pop_bit(bitboard, bit);
        }
    }
}

//vectorised by the compiler
void add_index(int16_t *restrict acc, uint32_t index, uint32_t c) {
    uint32_t offset = 256 * index;
    int16_t *restrict w = nnue_in_weights + offset;
    acc += 256*c;

#ifdef AVX2

    for (int32_t j = 0 ; j < 256 ; j += 16) {
        __m256i _x = _mm256_loadu_si256((void*)&acc[j]);
        __m256i _y = _mm256_loadu_si256((void*)&w[j]);
        _mm256_storeu_si256((__m256i*)&acc[j], _mm256_add_epi16(_x, _y));
    }

#else

    for (uint16_t j = 0; j < 256; j++)
        acc[j] += w[j];

#endif
}

//vectorised by the compiler
void subtract_index(int16_t *restrict acc, uint32_t index, uint32_t c) {
    uint32_t offset = 256 * index;
    int16_t *restrict w = nnue_in_weights + offset;
    acc += 256*c;

#ifdef AVX2

    for (int32_t j = 0 ; j < 256 ; j += 16) {
        __m256i _x = _mm256_loadu_si256((void*)&acc[j]);
        __m256i _y = _mm256_loadu_si256((void*)&w[j]);
        _mm256_storeu_si256((__m256i*)&acc[j], _mm256_sub_epi16(_x, _y));
    }

#else

    for (uint16_t j = 0; j < 256; j++)
        acc[j] -= w[j];

#endif
}

void refresh_accumulator(NnueData *data, Board *board) {
    append_active_indicies(data, board);

    for (uint32_t c = 0; c < 2; c++) {
        memcpy(data->accumulation[c], nnue_in_biases, 256 * sizeof(int16_t));

        for (size_t k = 0; k < data->activeIndexCount[c]; k++) {
            uint32_t index = data->activeIndicies[c][k];
            add_index(data->accumulation[0], index, c);
        }
    }
}


void clamp_layer(int32_t *layer){
#ifdef AVX2

    __m256i _8128 = _mm256_set1_epi32(8128);
    __m256i _0 = _mm256_set1_epi32(0);

    for (int32_t i = 0; i < 32; i += 8) {
        __m256i _x = _mm256_load_si256((void*)&layer[i]);
        _x = _mm256_min_epi32(_x, _8128);
        _x = _mm256_max_epi32(_x, _0);
        _x = _mm256_srli_epi32(_x, 6);

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

    for (int32_t i = 0; i < 512; i += 16) {
        __m256i _x = _mm256_load_si256((void*)&acc[i]);
        _x = _mm256_min_epi16(_x, _127);
        _x = _mm256_max_epi16(_x, _0);
        _mm256_storeu_si256((__m256i*)&acc[i], _x);
    }

#else

    for (int32_t i = 0; i < 512; ++i) {
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
    int16_t *tmpAccum = data->tmpAccumulation;

    memcpy(tmpAccum, data->accumulation, sizeof data->tmpAccumulation);
    memcpy(data->l1, nnue_l1_biases, sizeof nnue_l1_biases);

    clamp_accumulator(tmpAccum);

    for (int32_t i = 0; i < 512; ++i) {
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
        propogate_l1(data);
        propogate_l2(data);
        propogate_l3(data);
        data->eval = (board->side == white) ? data->l3[0] : -data->l3[0];

        hashptr->eval = data->eval;
        hashptr->key = board->current_zobrist_key;
    }

    //convert winning advantages into material rather than activity
    if (data->eval > (180*64) && (board->side == board->searchColor)){
        int32_t mat = materialScore(board);
        mat = mat > 0 ? mat + 1 : 1;
        data->eval *= mat;
    } else if (data->eval < (180*64) && (board->side != board->searchColor)){
        int32_t mat = -materialScore(board);
        mat = mat > 0 ? mat + 1 : 1;
        data->eval *= mat;
    }

    return data->eval;
}

void nnue_pop_bit(int32_t ptype, int32_t bit, Board *board){

    pop_bit(board->bitboards[ptype], bit);

    if (!board->nnueUpdate)
        return;

    int32_t w_ksq = w_orient[bsf(board->bitboards[p_K])];
    int32_t b_ksq = b_orient[bsf(board->bitboards[p_k])];

    int32_t sq = w_orient[bit];
    int32_t pc = NnuePtypes[ptype];

    int32_t wi = make_index(white, sq, pc, w_ksq);
    int32_t bi = make_index(black, sq, pc, b_ksq);

    subtract_index(board->currentNnue.accumulation[0], wi, white);
    subtract_index(board->currentNnue.accumulation[0], bi, black);
}

void nnue_set_bit(int32_t ptype, int32_t bit, Board *board){

    set_bit(board->bitboards[ptype], bit);

    if (!board->nnueUpdate)
        return;

    int32_t w_ksq = w_orient[bsf(board->bitboards[p_K])];
    int32_t b_ksq = b_orient[bsf(board->bitboards[p_k])];

    int32_t sq = w_orient[bit];
    int32_t pc = NnuePtypes[ptype];

    int32_t wi = make_index(white, sq, pc, w_ksq);
    int32_t bi = make_index(black, sq, pc, b_ksq);

    add_index(board->currentNnue.accumulation[0], wi, white);
    add_index(board->currentNnue.accumulation[0], bi, black);
}
