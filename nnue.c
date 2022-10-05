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
            arr[(r * dims) + c] = tmpArr[(c*32) + r];
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

    transform_weight_indicies(nnue_l1_weights, NNUE_L1SIZE);
    transform_weight_indicies(nnue_l2_weights, NNUE_L2SIZE);

    return 0;
}

static inline void append_index(int32_t index, NnueData *data) {
    data->activeIndicies[data->activeIndexCount++] = index;
}

void append_active_indicies(NnueData *data, Board *board) {
    data->activeIndexCount = 0;

    for (uint32_t ptype = p_P; ptype <= p_k; ++ptype) {

        U64 bitboard = board->bitboards[ptype];
        while (bitboard) {
            int32_t bit = bsf(bitboard);
            int32_t sq = bit;
            int32_t pc = ptype;

            append_index((64*ptype) + sq, data);

            pop_bit(bitboard, bit);
        }
    }
}

//vectorised by the compiler
void add_index(int16_t *restrict acc, uint32_t index) {
    uint32_t offset = NNUE_KPSIZE * index;
    int16_t *restrict w = nnue_in_weights + offset;

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
void subtract_index(int16_t *restrict acc, uint32_t index) {
    uint32_t offset = NNUE_KPSIZE * index;
    int16_t *restrict w = nnue_in_weights + offset;

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

//    for (uint32_t c = 0; c < 2; c++) {
    memcpy(data->accumulation, nnue_in_biases, NNUE_KPSIZE * sizeof(int16_t));

    for (size_t k = 0; k < data->activeIndexCount; k++) {
        uint32_t index = data->activeIndicies[k];
        add_index(data->accumulation, index);

    }
//    }
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

void print_epi16(__m256i x){
    int16_t* ip = (int16_t*)&x;
    for (size_t i = 0; i < 16; i++)
    {
        printf("%d\t", ip[i]);
    }

    printf("\n");
    
}

void print_128_epi16(__m256i x){
    int16_t* ip = (int16_t*)&x;
    for (size_t i = 0; i < 16; i++)
    {
        printf("%d\t", ip[i]);
    }

    printf("\n");
    
}

int32_t nnue_hadd_epi16(__m256i x){
#ifdef AVX2
    __m128i h1 = (__m128i)_mm256_castps256_ps128((__m256)x);
    __m128i h2 = _mm256_extracti128_si256(x, 1);
    __m128i prod = _mm_add_epi16(h1, h2);

    int out = 0;
    int16_t* ip = (int16_t*)&prod;
    for (int8_t i = 0; i < 8; i++) {
        out += ip[i];
    }

    return out;

#endif
}

static inline void propogate_neuron(const uint8_t* source, const int8_t* weights, int32_t *out){
    #ifdef AVX2
        __m256i va = _mm256_loadu_si256((__m256i*)source);
        __m256i vb = _mm256_loadu_si256((__m256i*)weights);
        __m256i prod = _mm256_maddubs_epi16(va, vb);

        *out += nnue_hadd_epi16(prod);
        
    #endif
}

static inline void propogate_l2_neuron(const uint8_t* source, const int8_t* weights, int32_t *out){
    #ifdef AVX2
        __m256i sum = _mm256_setzero_si256();
        for (int8_t i = 0; i < 4; i++) {
            __m256i va = _mm256_loadu_si256((__m256i*)&source[32*i]);
            __m256i vb = _mm256_loadu_si256((__m256i*)&weights[32*i]);
            __m256i prod = _mm256_maddubs_epi16(va, vb);
            sum = _mm256_add_epi16(prod, sum);
        }
        
        *out += nnue_hadd_epi16(sum);
        
    #endif
}

void squish_accumulator(int16_t* x, uint8_t* y){
    for (int32_t i = 0; i < NNUE_L1SIZE; i++) {
        y[i] = (uint8_t)x[i];
    }
}

void add_extra_feautres(NnueData *data, Board *board){
    if (board->side == white){
        add_index(data->tmpAccumulation, 768);
    }
}

void propogate_l2(NnueData *data, Board *board) {
    int16_t *tmpAccum = data->tmpAccumulation;

    memcpy(tmpAccum, data->accumulation, sizeof data->tmpAccumulation);
    add_extra_feautres(data, board);
    clamp_accumulator(tmpAccum);

    squish_accumulator(tmpAccum, &data->small_l1[0]);

    memcpy(data->l2, nnue_l1_biases, sizeof nnue_l1_biases);

    for (int32_t i = 0; i < NNUE_L2SIZE; ++i) {
        propogate_l2_neuron(&data->small_l1[0], &nnue_l1_weights[i*NNUE_L1SIZE], &data->l2[i]);
    }

    clamp_layer(data->l2);
    for (int32_t i = 0; i < NNUE_L2SIZE; i++) {
        data->small_l2[i] = (uint8_t)data->l2[i];
    }
    
}

__attribute_noinline__
void propogate_l3(NnueData *data){
    memcpy(data->l3, nnue_l2_biases, sizeof nnue_l2_biases);

    for (int32_t i = 0; i < NNUE_L3SIZE; ++i) {
        propogate_neuron(&data->small_l2[0], &nnue_l2_weights[i*NNUE_L2SIZE], &data->l3[i]);
    }

    clamp_layer(data->l3);
}

__attribute_noinline__
void propogate_l4(NnueData *data){
    memcpy(data->l4, nnue_l3_biases, sizeof nnue_l3_biases);
    for (int32_t i = 0; i < 32; ++i) {
        data->l4[0] += data->l3[i] * nnue_l3_weights[i];
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
        propogate_l2(data, board);
        propogate_l3(data);
        propogate_l4(data);

        int eval = (int)(((((float)data->l4[0] / 127) / 127) * 410) * 64);

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

    subtract_index(board->currentNnue.accumulation, wi);
}

void nnue_set_bit(int32_t ptype, int32_t bit, Board *board){

    if (!board->networkUpdate)
        return;

    int32_t w_ksq = w_orient[bsf(board->bitboards[p_K])];
    int32_t b_ksq = b_orient[bsf(board->bitboards[p_k])];

    int32_t sq = bit;
    int32_t pc = ptype;

    int32_t wi = (64*pc) + sq;

    add_index(board->currentNnue.accumulation, wi);
}
