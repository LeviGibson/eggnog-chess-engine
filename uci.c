#include "uci.h"
#include "board.h"
#include "bitboard.h"
#include "search.h"
#include "timeman.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>



int parse_move(char * move_string){

    moveList legalMoves;
    legalMoves.count = 0;
    generate_moves(&legalMoves);

    int source_square = (move_string[0] - 'a')+ (8-(move_string[1] - '0'))*8;
    int target_square = (move_string[2] - 'a') + (8-(move_string[3]-'0'))*8;

    for (int move_count = 0; move_count < legalMoves.count; move_count++){

        int move = legalMoves.moves[move_count];
        int promoted_piece = get_move_promoted(move);

        if ((get_move_source(move) == source_square) && (get_move_target(move) == target_square)){
            int promoted_piece = get_move_promoted(move);
            if (promoted_piece)
            {

                if (move_string[4] == 'q' && (promoted_piece == Q || promoted_piece == q)) {
                    return move;
                }
                else if (move_string[4] == 'b' && (promoted_piece == B || promoted_piece == b)) {
                    return move;
                }
                else if (move_string[4] == 'r' && (promoted_piece == R || promoted_piece == r)) {
                    return move;
                }
                else if (move_string[4] == 'n' && (promoted_piece == N || promoted_piece == n)) {
                    return move;
                }

            } else {
            return move;
          }
        }
    }
    return 0;
}

//position startpos moves e2e4 e7e5 d2d4 d7d5 d4e5 c7c5 e5e6 b8c6 e6f7 e8d7 f7g8r

void parse_position(char *command){
    command += 9;

    char *current_char = command;

    if (strncmp(command, "startpos", 8) == 0){
        parse_fen(start_position);

    } else{

        current_char = strstr(command, "fen");

        if (current_char == NULL)
            parse_fen(start_position);
        else{
            current_char += 4;
            parse_fen(current_char);
        }

    }

    current_char = strstr(command, "moves");
    if (current_char != NULL){
        current_char += 6;
        while (*current_char){

          int move = parse_move(current_char);

          make_move(move, all_moves);

          while ((*current_char) && (*current_char != ' ')){
              current_char++;
          }

          current_char++;

        }
        printf("%s\n", current_char);
    }
}

void parse_go(char *command){
    int depth = -1;
    char *current_depth = NULL;
    char *current_timelimit = NULL;

    current_depth = strstr(command, "depth");
    if (current_depth){
        depth = atoi(current_depth+6);
        moveTime = 100000;
    } else {
        moveTime = 2000;
        depth = 20;
    }
    current_timelimit = strstr(command, "movetime");
    if (current_timelimit){
        depth = max_ply;
        moveTime = atoi(current_timelimit+9);
    }

    search_position(depth);

    //search_position(depth, 1);

}

void uci_loop(){
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char input[5000];
    printf("id name Eggnog Chess\n");
    printf("id name Levi Gibson\n");
    printf("uciok\n");

    while (1){
        memset(input, 0, sizeof(input));
        fflush(stdout);
        if (!fgets(input, 5000, stdin)){
            continue;
        }

        if (input[0] == '\n')
            continue;
        if (strncmp(input, "isready", 7) == 0){
            printf("readyok\n");
        }

        if (strncmp(input, "position", 8) == 0){
            parse_position(input);
        }

        if (strncmp(input, "ucinewgame", 10) == 0){
            parse_position("position startpos");

        }
        if (strncmp(input, "go", 2) == 0){

            parse_go(input);

        }
        if (strncmp(input, "quit", 4) == 0){
            break;
        }
        if (strncmp(input, "uci", 3) == 0){
            printf("id name Eggnog Chess\n");
            printf("id name Levi Gibson\n");
            printf("uciok\n");
        }


    }
}
