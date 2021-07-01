//
// Created by levigibson on 5/31/21.
//

#include "board.h"

#include <stdio.h>

/**
U64 bitboards[12];
U64 occupancies[3];
int enpessant = no_sq;
int side = -1;
int castle = 0;
**/

const char *square_to_coordinates[] = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

U64 piece_keys[12][64];
U64 castle_keys[16];
U64 enpessent_keys[64];
U64 side_keys[2];

void init_zobrist_keys(){
  for (int piece = P; piece <= k; piece++){
    for (int square = 0; square < 64; square++){
      piece_keys[piece][square] = get_random_U64();
    }
  }

  for (int i = 0; i < 16; i++) {
    castle_keys[i] = get_random_U64();
  }

  for (int square = 0; square < 64; square++){
    enpessent_keys[square] = get_random_U64();
  }

  side_keys[white] = get_random_U64();
  side_keys[black] = get_random_U64();

  zobrist_history_length = 0;
}


U64 generate_zobrist_key(){
  U64 key = 0ULL;
  for (int piece = P; piece <= k; piece++){
    U64 bitboard = bitboards[piece];
    while (bitboard){
      int square = get_ls1b_index(bitboard);

      key ^= piece_keys[piece][square];

      pop_bit(bitboard, square);
    }
  }

  if (enpessant != no_sq){
    key ^= enpessent_keys[enpessant];
  }

  key ^= side_keys[side];

  key ^= castle_keys[castle];

  return key;
}

int is_threefold_repetition(){

  if (zobrist_history_length < 8)
    return 0;

  if (zobrist_history_length >= 100){
    return 1;
  }

  for (int position = 0; position <= zobrist_history_length; position++){
    int repetitions = 0;
    for (int compare = position+1; compare <= zobrist_history_length; compare++){
      if (zobrist_history[compare] == zobrist_history[position]){

        repetitions++;

        if (repetitions >= 2){
          return 1;
        }
      }
    }
  }

  return 0;

}

static inline void generate_knight_moves(moveList *legalMoves){
    int ptype = ((side == white) ? N : n);
    U64 bitboard = bitboards[ptype];

    U64 friendly_occupanices = occupancies[side];

    while (bitboard){
        int square = get_ls1b_index(bitboard);

        U64 bb_moves = knight_mask[square] & (~friendly_occupanices);

        while (bb_moves){
            int target = get_ls1b_index(bb_moves);

            if ((1ULL << target) & occupancies[both]) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                legalMoves->count++;
            }
            else {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
                legalMoves->count++;
            }

            pop_bit(bb_moves, target);
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_pawn_moves(moveList *legalMoves){
    int ptype = ((side == white) ? P : p);
    U64 bitboard = bitboards[ptype];

    U64 enemy_occupanices = (side == white) ? occupancies[black] : occupancies[white];

    if (enpessant != no_sq)
        enemy_occupanices |= 1ULL << enpessant;

    while (bitboard){
        int square = get_ls1b_index(bitboard);

        //Pawn Captures
        U64 bb_moves = pawn_mask[side][square] & enemy_occupanices;

        while (bb_moves){
            int target = get_ls1b_index(bb_moves);

            //Enpessant
            if (target == enpessant) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 1, 1, 0, 0);
                legalMoves->count++;
            }
            else {
                if ((target <= 7) || (target >= 56)) {
                    if (side == white) {
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, Q, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, B, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, R, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, N, 0, 1, 0, 0);
                        legalMoves->count++;
                    } else {
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, q, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, b, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, r, 0, 1, 0, 0);
                        legalMoves->count++;
                        legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, n, 0, 1, 0, 0);
                        legalMoves->count++;
                    }

                } else{
                    legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                    legalMoves->count++;
                }
            }

            pop_bit(bb_moves, target);
        }

        //Pawn quiet moves
        if (side == white){
            if (!(occupancies[both] & 1ULL << (square-8))) {

                if (square <= 15) {
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, Q, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, R, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, B, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, N, 0, 0, 0, 0);
                    legalMoves->count++;
                }
                else {
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square-8), ptype, 0, 0, 0, 0, 0);
                    legalMoves->count++;
                }

                //double pawn pushes
                if (square >= 48) {
                    if (!(occupancies[both] & 1ULL << (square-16))){
                        legalMoves->moves[legalMoves->count] = encode_move(square, (square-16), ptype, 0, 0, 0, 1, 0);
                        legalMoves->count++;
                    }
                }
            }
        } else {
            if (!(occupancies[both] & 1ULL << (square+8))) {

                if (square >= 48) {
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, q, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, r, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, b, 0, 0, 0, 0);
                    legalMoves->count++;
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, n, 0, 0, 0, 0);
                    legalMoves->count++;
                }
                else {
                    legalMoves->moves[legalMoves->count] = encode_move(square, (square+8), ptype, 0, 0, 0, 0, 0);
                    legalMoves->count++;
                }

                //double pawn pushes
                if (square <= 15) {
                    if (!(occupancies[both] & 1ULL << (square+16))){
                        legalMoves->moves[legalMoves->count] = encode_move(square, (square+16), ptype, 0, 0, 0, 1, 0);
                        legalMoves->count++;
                    }
                }
            }
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_bishop_moves(moveList *legalMoves){
    int ptype = ((side == white) ? B : b);
    U64 bitboard = bitboards[ptype];

    U64 friendly_occupanices = occupancies[side];

    while (bitboard){
        int square = get_ls1b_index(bitboard);

        U64 bb_moves = get_bishop_attacks(square, occupancies[both]) & (~friendly_occupanices);

        while (bb_moves){
            int target = get_ls1b_index(bb_moves);

            if ((1ULL << target) & occupancies[both]) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                legalMoves->count++;
            } else {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
                legalMoves->count++;
            }

            pop_bit(bb_moves, target);
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_rook_moves(moveList *legalMoves){
    int ptype = ((side == white) ? R : r);
    U64 bitboard = bitboards[ptype];

    U64 friendly_occupanices = occupancies[side];

    while (bitboard){
        int square = get_ls1b_index(bitboard);

        U64 bb_moves = get_rook_attacks(square, occupancies[both]) & (~friendly_occupanices);

        while (bb_moves){
            int target = get_ls1b_index(bb_moves);

            if ((1ULL << target) & occupancies[both]) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                legalMoves->count++;
            }
            else {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
                legalMoves->count++;
            }

            pop_bit(bb_moves, target);
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_queen_moves(moveList *legalMoves){
    int ptype = ((side == white) ? Q : q);
    U64 bitboard = bitboards[ptype];

    U64 friendly_occupanices = occupancies[side];

    while (bitboard){
        int square = get_ls1b_index(bitboard);

        U64 bb_moves = (get_bishop_attacks(square, occupancies[both]) | get_rook_attacks(square, occupancies[both])) & (~friendly_occupanices);

        while (bb_moves){
            int target = get_ls1b_index(bb_moves);

            if ((1ULL << target) & occupancies[both]) {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
                legalMoves->count++;
            } else {
                legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
                legalMoves->count++;
            }

            pop_bit(bb_moves, target);
        }

        pop_bit(bitboard, square);
    }
}

static inline void generate_king_moves(moveList *legalMoves){
    int ptype = ((side == white) ? K : k);
    U64 bitboard = bitboards[ptype];

    U64 friendly_occupanices = occupancies[side];

    if (bitboard == 0){
      print_board();
    }

    int square = get_ls1b_index(bitboard);

    U64 bb_moves = king_mask[square] & (~friendly_occupanices);

    while (bb_moves){
        int target = get_ls1b_index(bb_moves);

        if ((1ULL << target) & occupancies[both]) {
            //source, target, ptype, prom, enpessant, capture
            legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 1, 0, 0);
            legalMoves->count++;
        }
        else{
            legalMoves->moves[legalMoves->count] = encode_move(square, target, ptype, 0, 0, 0, 0, 0);
            legalMoves->count++;

        }

        pop_bit(bb_moves, target);
    }

    //castling
    if (side == white){
      if (castle & wk){
        //0x6000000000000000ULL
        //(1ULL << f1) | (1ULL << g1)))
        if (!(occupancies[both] & 0x6000000000000000ULL)){
            if (!(is_square_attacked(f1, black)) && !(is_square_attacked(g1, black)) && !(is_square_attacked(e1, black))){
            legalMoves->moves[legalMoves->count] = encode_move(square, 62, ptype, 0, 0, 0, 0, 1);
            legalMoves->count++;
          }
        }
      }
      if (castle & (wq)){
        //0xe00000000000000ULL
        //((1ULL << d1) | (1ULL << c1) | (1ULL << b1))
        if (!(occupancies[both] & 0xe00000000000000ULL)){
          if (!(is_square_attacked(c1, black)) && !(is_square_attacked(d1, black)) && !(is_square_attacked(e1, black))){
            legalMoves->moves[legalMoves->count] = encode_move(square, 58, ptype, 0, 0, 0, 0, 1);
            legalMoves->count++;
          }
        }
      }
    }else{
      if (castle & bk){
        //0x60ULL
        //((1ULL << f8) | (1ULL << g8))
        if (!(occupancies[both] & 0x60ULL)){
          if (!(is_square_attacked(f8, white)) && !(is_square_attacked(g8, white)) && !(is_square_attacked(e8, white))){
            legalMoves->moves[legalMoves->count] = encode_move(square, 6, ptype, 0, 0, 0, 0, 1);
            legalMoves->count++;
          }
        }
      }
      if (castle & (bq)){
        //0xeULL
        //((1ULL << d8) | (1ULL << c8) | (1ULL << b8))
        if (!(occupancies[both] & 0xeULL)){
          if (!(is_square_attacked(c8, white)) && !(is_square_attacked(d8, white)) && !(is_square_attacked(e8, white))){
            legalMoves->moves[legalMoves->count] = encode_move(square, 2, ptype, 0, 0, 0, 0, 1);
            legalMoves->count++;
          }
        }
      }
    }
}

static inline void update_occupancies(){

  memset(occupancies, 0ULL, sizeof(occupancies));

    for (int piece = P; piece <= K; piece++){
        occupancies[white] |= bitboards[piece];
    }

    for (int piece = p; piece <= k; piece++){
        occupancies[black] |= bitboards[piece];
    }

  occupancies[both] = (occupancies[black] | occupancies[white]);
}

void generate_moves(moveList *legalMoves){

  update_occupancies();

  generate_knight_moves(legalMoves);
  generate_pawn_moves(legalMoves);
  generate_bishop_moves(legalMoves);
  generate_rook_moves(legalMoves);
  generate_queen_moves(legalMoves);
  generate_king_moves(legalMoves);
}



const int castling_right_table[64] = {
        7, 15, 15, 15,  3, 15, 15, 11,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        15, 15, 15, 15, 15, 15, 15, 15,
        13, 15, 15, 15, 12, 15, 15, 14
};

int make_move(int move, int flag){
    if (flag == all_moves){
        copy_board();

        int ptype = get_move_piece(move);
        int source = get_move_source(move);
        int target = get_move_target(move);
        int capture = get_move_capture(move);

        bitboards[ptype] ^= (1ULL << source);
        bitboards[ptype] |= (1ULL << target);

        if (capture){
            if (get_move_enpessant(move)){
                if (side == white){
                    pop_bit(bitboards[p], target+8);
                } else {
                    pop_bit(bitboards[P], target-8);
                }
            } else {
              if (side == white){
                for (int piece = p; piece <= k; piece++) {
                    if (get_bit(bitboards[piece], target)) {

                        pop_bit(bitboards[piece], target);

                        break;
                    }
                }
              } else {
                for (int piece = P; piece <= K; piece++) {
                    if (get_bit(bitboards[piece], target)) {

                        pop_bit(bitboards[piece], target);

                        break;
                    }
                }
              }
            }
        }

        if (get_move_double(move)){
            enpessant = (side == white) ? (target + 8) : (target - 8);
        } else {
            enpessant = no_sq;
        }

        if (get_move_castle(move)){
          if (side == white){
            if (target == 62){
              pop_bit(bitboards[R], 63);
              set_bit(bitboards[R], 61);
            } else if (target == 58){
              pop_bit(bitboards[R], 56);
              set_bit(bitboards[R], 59);
            }
          } else {
            if (target == 6){
              pop_bit(bitboards[r], 7);
              set_bit(bitboards[r], 5);
            } else if (target == 2){
              pop_bit(bitboards[r], 0);
              set_bit(bitboards[r], 3);
            }
          }
        }

        int promoted = get_move_promoted(move);
        if (promoted){
          pop_bit(bitboards[ptype], target);
          set_bit(bitboards[promoted], target);
        }

        update_occupancies();

        if (side == white){
          if (is_square_attacked(get_ls1b_index(bitboards[K]), black)){
            take_back();
            return 0;
          }
        } else {
          if (is_square_attacked(get_ls1b_index(bitboards[k]), white)){
            take_back();
            return 0;
          }
        }

        castle &= castling_right_table[source];
        castle &= castling_right_table[target];

        side ^= 1;

        if ((ptype == P) || (ptype == p) || (castle_copy != castle) || capture){
          memset(zobrist_history, 0, sizeof(zobrist_history));
          zobrist_history_length = 0;
        }

        //UPDATE ZOBRIST HISTORY
        zobrist_history[zobrist_history_length] = generate_zobrist_key();
        zobrist_history_length++;
    } else {

      if (get_move_capture(move)){

        return make_move(move, all_moves);

      } else {
        return 0;
      }

    }

    return 1;
}

static inline U64 get_queen_attacks(int square, U64 occupancies){
  return (get_bishop_attacks(square, occupancies) | get_rook_attacks(square, occupancies));
}

int is_square_attacked(int square, int testingSide){

  if ((testingSide == white) && (pawn_mask[black][square] & bitboards[P])) return 1;
  if ((testingSide == black) && (pawn_mask[white][square] & bitboards[p])) return 1;

  if (knight_mask[square] & ((testingSide == white ? bitboards[N] : bitboards[n]))) return 1;

  if (get_bishop_attacks(square, occupancies[both]) & ((testingSide == white ? bitboards[B] : bitboards[b]))) return 1;
  if (get_rook_attacks(square, occupancies[both]) & ((testingSide == white ? bitboards[R] : bitboards[r]))) return 1;

  if (get_queen_attacks(square, occupancies[both]) & ((testingSide == white ? bitboards[Q] : bitboards[q]))) return 1;

  if (king_mask[square] & ((testingSide == white ? bitboards[K] : bitboards[k]))) return 1;

  return 0;
}

int char_pieces[] = {
        ['P'] = P,
        ['N'] = N,
        ['B'] = B,
        ['R'] = R,
        ['Q'] = Q,
        ['K'] = K,
        ['p'] = p,
        ['n'] = n,

        ['b'] = b,
        ['r'] = r,
        ['q'] = q,
        ['k'] = k
};
char piece_symbols[12] = {
        'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'
};

void print_move(int move){
    printf("%s%s", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)]);
    if ((get_move_promoted(move) == Q) | (get_move_promoted(move) == q)){
      printf("q");
    }
    if ((get_move_promoted(move) == B) | (get_move_promoted(move) == b)){
      printf("b");
    }
    if ((get_move_promoted(move) == N) | (get_move_promoted(move) == n)){
      printf("n");
    }
    if ((get_move_promoted(move) == R) | (get_move_promoted(move) == r)){
      printf("r");
    }
}

void print_board(){
    int rank = 8;
    for (int square = 0; square < 64; square++){

        if (!(square % 8)) {
            printf("\n");
            printf("%d | ", rank);
            rank--;
        }

        int found_piece = 0;

        for (int piece = P; piece <= k; piece++){
            if (get_bit(bitboards[piece], square)){
                printf("%c ", piece_symbols[piece]);
                found_piece = 1;
                break;
            }
        }

        if (!found_piece)
            printf(". ");
    }
    printf("\n    _______________\n");
    printf("    A B C D E F G H\n\n");
}



void parse_fen(char *fen)
{
    memset(bitboards, 0ULL, sizeof(bitboards));

    memset(occupancies, 0ULL, sizeof(occupancies));

    memset(zobrist_history, 0, sizeof(zobrist_history));
    zobrist_history_length = 0;

    side = 0;
    enpessant = no_sq;
    castle = 0;

    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;

            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                int piece = char_pieces[*fen];

                set_bit(bitboards[piece], square);

                fen++;
            }

            if (*fen >= '0' && *fen <= '9')
            {
                int offset = *fen - '0';

                int piece = -1;

                for (int bb_piece = P; bb_piece <= k; bb_piece++)
                {
                    if (get_bit(bitboards[bb_piece], square))
                        piece = bb_piece;
                }
                if (piece == -1)
                    file--;

                file += offset;

                fen++;
            }

            if (*fen == '/')
                fen++;
        }
    }
    fen++;

    (*fen == 'w') ? (side = white) : (side = black);

    fen += 2;

    while (*fen != ' ')
    {
        switch (*fen)
        {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
            case '-': break;
        }

        fen++;
    }

    fen++;

    if (*fen != '-')
    {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');

        enpessant = rank * 8 + file;
    }

    else
        enpessant = no_sq;

    for (int piece = P; piece <= K; piece++)
        occupancies[white] |= bitboards[piece];

    for (int piece = p; piece <= k; piece++)
        occupancies[black] |= bitboards[piece];

    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];


}
