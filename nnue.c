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

EvalHashEntry evalHashTable[tt_size * 4];

#define TRANSFORMERSTART ((3 * 4) + 181)

alignas(64) int16_t in_weights[INSIZE * KPSIZE ];
alignas(64) int8_t l1_weights[L1SIZE * L2SIZE ];
alignas(64) int8_t l2_weights[L2SIZE * L3SIZE ];
alignas(64) int8_t l3_weights[L3SIZE * OUTSIZE];

alignas(64) int16_t in_biases[KPSIZE ];
alignas(64) int32_t l1_biases[L2SIZE ];
alignas(64) int32_t l2_biases[L3SIZE ];
alignas(64) int32_t l3_biases[OUTSIZE];

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
    tmp = fread(in_biases, sizeof(int16_t), KPSIZE, fin);
    tmp = fread(in_weights, sizeof(int16_t), INSIZE * KPSIZE, fin);

    fseek(fin, 4, SEEK_CUR);

    //Hidden Layer 1
    tmp = fread(l1_biases, sizeof (l1_biases[0]), L2SIZE, fin);
    tmp = fread(l1_weights, sizeof (l1_weights[0]), L1SIZE * L2SIZE, fin);
    transform_weight_indicies(l1_weights, L1SIZE);

    //Hidden Layer 2
    tmp = fread(l2_biases, sizeof (l2_biases[0]), L3SIZE, fin);
    tmp = fread(l2_weights, sizeof (l2_weights[0]), L2SIZE * L3SIZE, fin);
    transform_weight_indicies(l2_weights, L2SIZE);

    //Output Layer
    tmp = fread(l3_biases, sizeof (l3_biases[0]), OUTSIZE, fin);

    tmp = fread(l3_weights, sizeof (l3_weights[0]), L2SIZE * OUTSIZE, fin);

    fclose(fin);

    for (int32_t i = 0; i < tt_size; ++i) {
        evalHashTable[i].eval = NO_EVAL;
    }

    return 0;
}

int32_t NnuePtypes[12] = {6, 5, 4, 3, 2, K, 12, 11, 10, 9, 8, k};

const static int32_t w_orient[64] = {
        56, 57, 58, 59, 60, 61, 62, 63,
        48, 49, 50, 51, 52, 53, 54, 55,
        40, 41, 42, 43, 44, 45, 46, 47,
        32, 33, 34, 35, 36, 37, 38, 39,
        24, 25, 26, 27, 28, 29, 30, 31,
        16, 17, 18, 19, 20, 21, 22, 23,
        8, 9, 10, 11, 12, 13, 14, 15,
        0, 1, 2, 3, 4, 5, 6, 7,
};

const static int32_t b_orient[64] = {
        7, 6, 5, 4, 3, 2, 1, 0,
        15, 14, 13, 12, 11, 10, 9, 8,
        23, 22, 21, 20, 19, 18, 17, 16,
        31, 30, 29, 28, 27, 26, 25, 24,
        39, 38, 37, 36, 35, 34, 33, 32,
        47, 46, 45, 44, 43, 42, 41, 40,
        55, 54, 53, 52, 51, 50, 49, 48,
        63, 62, 61, 60, 59, 58, 57, 56
};

enum {
    PS_W_PAWN = 1,
    PS_B_PAWN = 1 * 64 + 1,
    PS_W_KNIGHT = 2 * 64 + 1,
    PS_B_KNIGHT = 3 * 64 + 1,
    PS_W_BISHOP = 4 * 64 + 1,
    PS_B_BISHOP = 5 * 64 + 1,
    PS_W_ROOK = 6 * 64 + 1,
    PS_B_ROOK = 7 * 64 + 1,
    PS_W_QUEEN = 8 * 64 + 1,
    PS_B_QUEEN = 9 * 64 + 1,
    PS_END = 10 * 64 + 1
};

uint32_t PieceToIndex[2][14] = {
        {0, 0, PS_W_QUEEN, PS_W_ROOK, PS_W_BISHOP, PS_W_KNIGHT, PS_W_PAWN,
                0, PS_B_QUEEN, PS_B_ROOK, PS_B_BISHOP, PS_B_KNIGHT, PS_B_PAWN, 0},
        {0, 0, PS_B_QUEEN, PS_B_ROOK, PS_B_BISHOP, PS_B_KNIGHT, PS_B_PAWN,
                0, PS_W_QUEEN, PS_W_ROOK, PS_W_BISHOP, PS_W_KNIGHT, PS_W_PAWN, 0}
};


int32_t orient(int32_t c, int32_t s) {
    return s ^ (c == white ? 0x00 : 0x3f);
}

int32_t make_index(int32_t c, int32_t s, int32_t pc, int32_t ksq) {
    return orient(c, s) + PieceToIndex[c][pc] + PS_END * ksq;
}

static void append_index(int32_t c, int32_t index, NnueData *data) {
    data->activeIndicies[c][data->activeIndexCount[c]++] = index;
}

void append_active_indicies(NnueData *data, Board *board) {
    data->activeIndexCount[black] = 0;
    data->activeIndexCount[white] = 0;

    int32_t w_ksq = w_orient[bsf(board->bitboards[K])];
    int32_t b_ksq = b_orient[bsf(board->bitboards[k])];

    for (uint32_t ptype = P; ptype < k; ++ptype) {
        if (ptype == K)
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
    int16_t *restrict w = in_weights + offset;
    acc += 256*c;

    for (unsigned j = 0; j < 256; j++)
        acc[j] += w[j];
}

//vectorised by the compiler
void subtract_index(int16_t *restrict acc, uint32_t index, uint32_t c) {
    uint32_t offset = 256 * index;
    int16_t *restrict w = in_weights + offset;
    acc += 256*c;

    for (unsigned j = 0; j < 256; j++)
        acc[j] -= w[j];
}

void refresh_accumulator(NnueData *data, Board *board) {
    append_active_indicies(data, board);

    for (u_int32_t c = 0; c < 2; c++) {
        memcpy(data->accumulation[c], in_biases, 256 * sizeof(int16_t));

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
    __m256i va = _mm256_set1_epi32(a);

    for (int32_t i = 0 ; i < 32 ; i += 8) {
        __m256i vb = _mm256_cvtepi8_epi32( _mm_loadl_epi64((__m128i*)&b[i]) );
        __m256i prod = _mm256_madd_epi16(va, vb);
        __m256i sum = _mm256_add_epi32(prod, _mm256_loadu_si256((const __m256i*)&c[i]));
        _mm256_storeu_si256((__m256i*)&c[i], sum);
    }
#else
    for (int32_t i = 0; i < 32; ++i)
        c[i] += a * b[i];
#endif

}

void propogate_l1(NnueData *data) {
    int16_t *tmpAccum = data->tmpAccumulation;

    memcpy(tmpAccum, data->accumulation, sizeof data->tmpAccumulation);
    memcpy(data->l1, l1_biases, sizeof l1_biases);

    clamp_accumulator(tmpAccum);

    for (int32_t i = 0; i < 512; ++i) {
        if (tmpAccum[i]) {
            int32_t offset = 32 * i;
            propogate_neuron(tmpAccum[i], &l1_weights[offset], data->l1);
        }
    }

    clamp_layer(data->l1);
}

void propogate_l2(NnueData *data){
    memcpy(data->l2, l2_biases, sizeof l2_biases);

    for (int32_t o = 0; o < 32; ++o) {
        if (!data->l1[o])
            continue;

        int32_t offset = 32 * o;
        propogate_neuron((short )data->l1[o], &l2_weights[offset], data->l2);
    }

    clamp_layer(data->l2);
}

void propogate_l3(NnueData *data){
    memcpy(data->l3, l3_biases, sizeof l3_biases);
    for (int32_t i = 0; i < 32; ++i) {
        data->l3[0] += data->l2[i] * l3_weights[i];
    }
}

int32_t materialScore(Board *board){
    int32_t eval = 0;

    eval += 1 * count_bits(board->bitboards[P]);
    eval += 3 * count_bits(board->bitboards[N]);
    eval += 3 * count_bits(board->bitboards[B]);
    eval += 5 * count_bits(board->bitboards[R]);
    eval += 10 * count_bits(board->bitboards[Q]);

    eval -= 1 * count_bits(board->bitboards[p]);
    eval -= 3 * count_bits(board->bitboards[n]);
    eval -= 3 * count_bits(board->bitboards[b]);
    eval -= 5 * count_bits(board->bitboards[r]);
    eval -= 10 * count_bits(board->bitboards[q]);

    return board->side == white ? eval : -eval;
}

int32_t nnue_evaluate(Board *board) {

    uint32_t hashIndex = board->current_zobrist_key % (tt_size*4);
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
        mat = mat > 0 ? mat+1 : 1;
        data->eval *= mat;
    } else if (data->eval < (180*64) && (board->side != board->searchColor)){
        int32_t mat = -materialScore(board);
        mat = mat > 0 ? mat+1 : 1;
        data->eval *= mat;
    }

    return data->eval;
}

void nnue_pop_bit(int32_t ptype, int32_t bit, Board *board){

    pop_bit(board->bitboards[ptype], bit);

    int32_t w_ksq = w_orient[bsf(board->bitboards[K])];
    int32_t b_ksq = b_orient[bsf(board->bitboards[k])];

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

    int32_t w_ksq = w_orient[bsf(board->bitboards[K])];
    int32_t b_ksq = b_orient[bsf(board->bitboards[k])];

    int32_t sq = w_orient[bit];
    int32_t pc = NnuePtypes[ptype];

    int32_t wi = make_index(white, sq, pc, w_ksq);
    int32_t bi = make_index(black, sq, pc, b_ksq);

    add_index(board->currentNnue.accumulation[0], wi, white);
    add_index(board->currentNnue.accumulation[0], bi, black);
}
