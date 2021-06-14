#include "evaluate.h"
#include "board.h"
#include "bitboard.h"
#include <stdio.h>

int pst[6][64] = {
                {0,  0,  0,  0,  0,  0,  0,  0,
                50, 50, 50, 50, 50, 50, 50, 50,
                10, 10, 20, 30, 30, 20, 10, 10,
                5,  5, 10, 25, 25, 10,  5,  5,
                0,  0,  0, 20, 20,  0,  0,  0,
                5, -5,-10,  0,  0,-10, -5,  5,
                5, 10, 10,-20,-20, 10, 10,  5,
                0,  0,  0,  0,  0,  0,  0,  0},
        {-50,-40,-30,-30,-30,-30,-40,-50,
                -40,-20,  0,  0,  0,  0,-20,-40,
                -30,  0, 10, 15, 15, 10,  0,-30,
                -30,  5, 15, 20, 20, 15,  5,-30,
                -30,  0, 15, 20, 20, 15,  0,-30,
                -30,  5, 10, 15, 15, 10,  5,-30,
                -40,-20,  0,  5,  5,  0,-20,-40,
                -50,-40,-30,-30,-30,-30,-40,-50},
        {-20,-10,-10,-10,-10,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5, 10, 10,  5,  0,-10,
                -10,  5,  5, 10, 10,  5,  5,-10,
                -10,  0, 10, 10, 10, 10,  0,-10,
                -10, 10, 10, 10, 10, 10, 10,-10,
                -10,  5,  0,  0,  0,  0,  5,-10,
                -20,-10,-10,-10,-10,-10,-10,-20},
        {0,  0,  0,  0,  0,  0,  0,  0,
                5, 10, 10, 10, 10, 10, 10,  5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                -5,  0,  0,  0,  0,  0,  0, -5,
                0,  0,  0,  5,  5,  0,  0,  0},
        {-20,-10,-10, -5, -5,-10,-10,-20,
                -10,  0,  0,  0,  0,  0,  0,-10,
                -10,  0,  5,  5,  5,  5,  0,-10,
                -5,  0,  5,  5,  5,  5,  0, -5,
                0,  0,  5,  5,  5,  5,  0, -5,
                -10,  5,  5,  5,  5,  5,  0,-10,
                -10,  0,  5,  0,  0,  0,  0,-10,
                -20,-10,-10, -5, -5,-10,-10,-20},
        {-30,-40,-40,-50,-50,-40,-40,-30,
                -30,-40,-40,-50,-50,-40,-40,-30,
                -30,-40,-40,-50,-50,-40,-40,-30,
                -30,-40,-40,-50,-50,-40,-40,-30,
                -20,-30,-30,-40,-40,-30,-30,-20,
                -10,-20,-20,-20,-20,-20,-20,-10,
                20, 20,  0,  0,  0,  0, 20, 20,
                20, 30, 10,  0,  0, 10, 30, 20}

};

int king_safety_masks[64][8] = {
  {1, 8, 9, -1, -1, -1, -1, -1},
  {0, 2, 8, 9, 10, -1, -1, -1},
  {1, 3, 9, 10, 11, -1, -1, -1},
  {2, 4, 10, 11, 12, -1, -1, -1},
  {3, 5, 11, 12, 13, -1, -1, -1},
  {4, 6, 12, 13, 14, -1, -1, -1},
  {5, 7, 13, 14, 15, -1, -1, -1},
  {6, 14, 15, -1, -1, -1, -1, -1},
  {0, 1, 9, 16, 17, -1, -1, -1},
  {0, 1, 2, 8, 10, 16, 17, 18},
  {1, 2, 3, 9, 11, 17, 18, 19},
  {2, 3, 4, 10, 12, 18, 19, 20},
  {3, 4, 5, 11, 13, 19, 20, 21},
  {4, 5, 6, 12, 14, 20, 21, 22},
  {5, 6, 7, 13, 15, 21, 22, 23},
  {6, 7, 14, 22, 23, -1, -1, -1},
  {8, 9, 17, 24, 25, -1, -1, -1},
  {8, 9, 10, 16, 18, 24, 25, 26},
  {9, 10, 11, 17, 19, 25, 26, 27},
  {10, 11, 12, 18, 20, 26, 27, 28},
  {11, 12, 13, 19, 21, 27, 28, 29},
  {12, 13, 14, 20, 22, 28, 29, 30},
  {13, 14, 15, 21, 23, 29, 30, 31},
  {14, 15, 22, 30, 31, -1, -1, -1},
  {16, 17, 25, 32, 33, -1, -1, -1},
  {16, 17, 18, 24, 26, 32, 33, 34},
  {17, 18, 19, 25, 27, 33, 34, 35},
  {18, 19, 20, 26, 28, 34, 35, 36},
  {19, 20, 21, 27, 29, 35, 36, 37},
  {20, 21, 22, 28, 30, 36, 37, 38},
  {21, 22, 23, 29, 31, 37, 38, 39},
  {22, 23, 30, 38, 39, -1, -1, -1},
  {24, 25, 33, 40, 41, -1, -1, -1},
  {24, 25, 26, 32, 34, 40, 41, 42},
  {25, 26, 27, 33, 35, 41, 42, 43},
  {26, 27, 28, 34, 36, 42, 43, 44},
  {27, 28, 29, 35, 37, 43, 44, 45},
  {28, 29, 30, 36, 38, 44, 45, 46},
  {29, 30, 31, 37, 39, 45, 46, 47},
  {30, 31, 38, 46, 47, -1, -1, -1},
  {32, 33, 41, 48, 49, -1, -1, -1},
  {32, 33, 34, 40, 42, 48, 49, 50},
  {33, 34, 35, 41, 43, 49, 50, 51},
  {34, 35, 36, 42, 44, 50, 51, 52},
  {35, 36, 37, 43, 45, 51, 52, 53},
  {36, 37, 38, 44, 46, 52, 53, 54},
  {37, 38, 39, 45, 47, 53, 54, 55},
  {38, 39, 46, 54, 55, -1, -1, -1},
  {40, 41, 49, 56, 57, -1, -1, -1},
  {40, 41, 42, 48, 50, 56, 57, 58},
  {41, 42, 43, 49, 51, 57, 58, 59},
  {42, 43, 44, 50, 52, 58, 59, 60},
  {43, 44, 45, 51, 53, 59, 60, 61},
  {44, 45, 46, 52, 54, 60, 61, 62},
  {45, 46, 47, 53, 55, 61, 62, 63},
  {46, 47, 54, 62, 63, -1, -1, -1},
  {48, 49, 57, -1, -1, -1, -1, -1},
  {48, 49, 50, 56, 58, -1, -1, -1},
  {49, 50, 51, 57, 59, -1, -1, -1},
  {50, 51, 52, 58, 60, -1, -1, -1},
  {51, 52, 53, 59, 61, -1, -1, -1},
  {52, 53, 54, 60, 62, -1, -1, -1},
  {53, 54, 55, 61, 63, -1, -1, -1},
  {54, 55, 62, -1, -1, -1, -1, -1}
};

const int scores[] = {100, 300, 325, 500, 900, 0, -100, -300, -325, -500, -900, 0};
const int piece_attack_weights[12] = {10, 12, 12, 10, 2, 1, -10, -12, -12, -10, -2, -1};

int knight_open_score[9] = { -20, -16, -12, -8, -4,  0,  4,  8, 12 };
int rook_open_score[9] = { 15,  12,   9,  6,  3,  0, -3, -6, -9 };

static inline int get_square_pressure(int square){

  int pressure = 0;
  for (int testingSide = white; testingSide <= black; testingSide++){
    if ((testingSide == white) && (pawn_mask[black][square] & bitboards[P])) pressure += 100;
    if ((testingSide == black) && (pawn_mask[white][square] & bitboards[p])) pressure += -100;

    if (knight_mask[square] & ((testingSide == white ? bitboards[N] : bitboards[n]))) pressure += (testingSide == white) ? 80 : -80;

    if (get_bishop_attacks(square, occupancies[both]) & ((testingSide == white ? bitboards[B] : bitboards[b]))) pressure += (testingSide == white) ? 80 : -80;
    if (get_rook_attacks(square, occupancies[both]) & ((testingSide == white ? bitboards[R] : bitboards[r]))) pressure += (testingSide == white) ? 60 : -60;

    if ((get_rook_attacks(square, occupancies[both]) | get_bishop_attacks(square, occupancies[both])) & ((testingSide == white ? bitboards[Q] : bitboards[q]))) pressure += (testingSide == white) ? 20 : -20;

    if (king_mask[square] & ((testingSide == white ? bitboards[K] : bitboards[k]))) pressure += (testingSide == white) ? 10 : -10;
  }

  return pressure;
}

static inline int generate_king_safety(){
  int score = 0;
  for (int t_side = white; t_side <= black; t_side++){
    int kingpos = get_ls1b_index((t_side == white) ? bitboards[K] : bitboards[k]);

    for (int attack_index = 0; attack_index < 8; attack_index++){
      int king_attack = king_safety_masks[kingpos][attack_index];
      //if king attack dosen't exist, break.
      if (king_attack == -1)
        break;

      int square_score = get_square_pressure(king_attack);

      if ((t_side == white) && (square_score > 0)){
        square_score /= 4;
      }
      if ((t_side == black) && (square_score < 0)){
        square_score /= 4;
      }

      score += square_score;
    }

  }
  return score;
}

int evaluate(){
  int eval = 0;

  /**
  if (bitboards[Q] && bitboards[q]){
    int king_safety = generate_king_safety() / 4;
    if (king_safety < 0){
      king_safety = king_safety * king_safety * -1;
    } else {
      king_safety = king_safety * king_safety;
    }
    eval += king_safety;
  }
  **/

  for(int piece = P; piece <= K; piece++){
    U64 bitboard = bitboards[piece];

    eval += scores[piece] * count_bits(bitboards[piece]);

    while (bitboard){
      int target = get_ls1b_index(bitboard);

      eval += pst[piece][target];

      bitboard -= (1ULL << target);
    }
  }

  for(int piece = p; piece <= k; piece++){
    U64 bitboard = bitboards[piece];

    eval += scores[piece] * count_bits(bitboards[piece]);

    while (bitboard){
      int target = get_ls1b_index(bitboard);

      eval -= pst[piece-6][63 - target];

      bitboard -= (1ULL << target);
    }
  }

  return (side == white) ? eval : -eval;
}
