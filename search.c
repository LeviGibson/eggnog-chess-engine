#include "search.h"
#include "board.h"
#include "evaluate.h"
#include "timeman.h"
#include "transposition.h"

#include <stdio.h>

int ply = 0;
long nodes = 0;

const int mvv_lva[12][12] = {
        105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
        104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
        103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
        102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
        101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
        100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

        105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
        104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
        103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
        102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
        101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
        100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

int killer_moves[max_ply][2];
int history_moves[2][64][64];

Line pv_line;

int follow_pv, found_pv;

static inline int score_move(int move){

  if (found_pv){
    if (move == pv_line.moves[ply]){
      /**
      printf("%d: ", ply);
      print_move(pv_line.moves[ply]);
      printf("\n");**/
      //pv sorting
      found_pv = 0;
      return 20000;
    }
  }

  if (get_move_capture(move)){
        int start_piece, end_piece;
        int target_piece = P;

        int target_square = get_move_target(move);

        if (side == white) {start_piece = p; end_piece = k;}
        else{start_piece = P; end_piece = K;}
        for (int bb_piece = start_piece; bb_piece < end_piece; bb_piece++){
            if (get_bit(bitboards[bb_piece], target_square)){
                target_piece = bb_piece;
                break;
            }
        }

        return mvv_lva[get_move_piece(move)][target_piece]+1000;
    } else {
      if (move == killer_moves[ply][0]){
        return(800);
      }
      if (move == killer_moves[ply][1]){
        return(700);
      }

      return history_moves[side][get_move_source(move)][get_move_target(move)];
    }
}

static inline void sort_moves(moveList *move_list){
    int scores[move_list->count];

    for (int i = 0; i < move_list->count; i++)
        scores[i] = score_move(move_list->moves[i]);

    for (int cur = 0; cur < move_list->count; cur++){
        for (int next = cur+1; next < move_list->count; next++){
            if (scores[cur] < scores[next]){
                int tmpscore = scores[cur];
                scores[cur] = scores[next];
                scores[next] = tmpscore;

                int tmpmove = move_list->moves[cur];
                move_list->moves[cur] = move_list->moves[next];
                move_list->moves[next] = tmpmove;
            }
        }
    }
}

static inline int quiesce(int alpha, int beta) {

  if (nodes % 2048 == 0)
    communicate();

  if (stop){
    return 0;
  }

  nodes++;

  int stand_pat = evaluate();
  if (stand_pat >= beta){
    return beta;
  }

  if (alpha <= stand_pat){
    alpha = stand_pat;
  }

  moveList legalMoves[1];
  legalMoves->count = 0;

  U64 old_occupancies = occupancies[white] | occupancies[black];

  occupancies[both] = old_occupancies;

  copy_board();

  generate_moves(legalMoves);
  sort_moves(legalMoves);

  for (int moveId = 0; moveId < legalMoves->count; moveId++){
    int move = legalMoves->moves[moveId];

    if (!get_move_capture(move))
      continue;

    if (make_move(move, all_moves)){
      int score = -quiesce(-beta, -alpha);

      take_back();

      if (score >= beta){
        return beta;
      }
      if (score > alpha)
        alpha = score;
    }
  }

  return alpha;

}

static inline int ZwSearch(int beta, int depth){

  if (nodes % 2048 == 0)
    communicate();

  if (stop){
    return 0;
  }

  nodes++;
  if (depth == 0)
    return quiesce(beta-1, beta);

  moveList legalMoves;
  legalMoves.count = 0;

  generate_moves(&legalMoves);
  sort_moves(&legalMoves);

  copy_board();

  for (int moveId = 0; moveId < legalMoves.count; moveId++){
    int move  = legalMoves.moves[moveId];

    int score;

    if (make_move(move, all_moves)){

      score = -ZwSearch(1-beta, depth-1);

      take_back();

      if (score >= beta){
        return beta;
      }
    }
  }
  return beta-1;
}

void find_pv(moveList *moves){
  follow_pv = 0;
  for (int moveId = 0; moveId < moves->count; moveId++){
    if (moves->moves[moveId] == pv_line.moves[ply]){
      found_pv = 1;
      follow_pv = 1;
    }
  }
}

static inline int negamax(int depth, int alpha, int beta, Line *pline){
  //general maintenence
  if (nodes % 2048 == 0)
    communicate();
  if (stop){
    return 0;
  }
  nodes++;

  /**
  U64 key = generate_zobrist_key();
  int hash_result = hash_lookup(key, beta, depth, pline);
  if(hash_result != no_entry){
    return hash_result;
  }
  **/

  found_pv = 0;

  int b_search_pv = 1;

  Line line;
  line.length = 0;

  if (depth <= 0){
    pline->length = 0;
    return quiesce(alpha, beta);
  }

  if (is_threefold_repetition()){
    return 0;
  }

  int in_check = is_square_attacked(get_ls1b_index((side == white) ? bitboards[K] : bitboards[k]), (side ^ 1));

  if (in_check)
    depth++;

  int eval;

  if (depth >= 3 && in_check == 0 && ply){
        copy_board();

        side ^= 1;
        enpessant = no_sq;

        eval = -ZwSearch(1-beta, depth-3);

        take_back();
        if (eval >= beta){
            return beta;
        }
    }



  moveList legalMoves;
  legalMoves.count = 0;

  generate_moves(&legalMoves);

  if (follow_pv){
    find_pv(&legalMoves);
  }

  sort_moves(&legalMoves);

  copy_board();

  int legalMoveCount = 0;
  int move;
  for (int moveId = 0; moveId < legalMoves.count; moveId++){
    move = legalMoves.moves[moveId];
    if (make_move(move, all_moves)){

      legalMoveCount++;

      ply++;

      if (legalMoveCount == 0){
        eval = -negamax(depth-1, -beta, -alpha, &line);
      } else {

        //LMR

        if ((depth >= 3) && (legalMoveCount > 5) && (in_check == 0) && (get_move_capture(move) == 0) && (get_move_promoted(move) == 0)){
          eval = -negamax(depth-2, -alpha-1, -alpha, &line);
          //eval = alpha + 1;
        }
        else {
          eval = alpha + 1;
        }

        if (eval > alpha){
          eval = -negamax(depth-1, -alpha-1, -alpha, &line);
          if ((eval > alpha) && (eval < beta)){
            eval = -negamax(depth-1, -beta, -alpha, &line);
          }
        }
      }


      ply--;

      if (eval > alpha){

        alpha = eval;
        b_search_pv = 0;

        pline->moves[0] = move;
        memcpy(pline->moves + 1, line.moves, line.length * 4);
        pline->length = line.length + 1;

      }

      take_back();

      if (eval >= beta){

        if (!get_move_capture(move)){
          killer_moves[ply][1] = killer_moves[ply][0];
          killer_moves[ply][0] = move;

          history_moves[side][get_move_source(move)][get_move_target(move)] += depth*depth;
        }

        //write_hash(key, beta, hash_flag_beta, depth, &line, move);

        return beta;
      }
    }

  }

  if (legalMoveCount == 0){
    if (in_check){
      return (-49000) + ply;
    } else {
      return 0;
    }
  }

  //write_hash(key, alpha, hash_flag_alpha, depth, &line, move);

  return alpha;

}

#define aspwindow 50

void search_position(int depth){

  start_time();

  stop = 0;
  nodes = 0;

  memset(killer_moves, 0, sizeof(killer_moves));
  memset(history_moves, 0, sizeof(history_moves));

  Line negamax_line;
  negamax_line.length = 0;

  int alpha = -50000;
  int beta = 50000;

  int eval;

  for (int currentDepth = 1; currentDepth <= depth; currentDepth++){
    reset_transposition_table();

    ply = 0;

    follow_pv = 1;
    found_pv = 0;

    int nmRes = negamax(currentDepth, alpha, beta, &negamax_line);

    if (stop)
      break;

    eval = nmRes;

    memcpy(&pv_line, &negamax_line, sizeof negamax_line);
    memset(&negamax_line, 0, sizeof negamax_line);

    printf("info score %s %d depth %d nodes %ld pv ",(abs(eval) > 40000) ? "mate" : "cp" , (abs(eval) > 40000) ? (49000 - abs(eval)) * (eval / abs(eval)) : eval, currentDepth, nodes);

    for (int i = 0; i < currentDepth; i++){
      print_move(pv_line.moves[i]);
      printf(" ");
    }

    printf("\n");

    if ((abs(eval) > 40000)){
      break;
    }


  }

  printf("bestmove ");
  print_move(pv_line.moves[0]);
  printf("\n");

}
