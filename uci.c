#include "uci.h"
#include "board.h"
#include "bitboard.h"
#include "search.h"
#include "timeman.h"
#include "Fathom/tbprobe.h"
#include "moveOrder.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_t searchthread;
int dynamicTimeManagment = 0;
int moveOverhead = 0;
int historyMoveDivisor = 1400;

void change_to_correct_directory(char *argv){
    int lastslash = 0;
#ifdef WIN32
    for (int i = 0; argv[i]; ++i) {
        if (argv[i] == '\\'){
            lastslash = i;
        }
    }
#elif defined(WIN64)
    for (int i = 0; argv[i]; ++i) {
        if (argv[i] == '\\'){
            lastslash = i;
        }
    }
#else
    for (int i = 0; argv[i]; ++i) {
        if (argv[i] == '/'){
            lastslash = i;
        }
    }
#endif


    char path[lastslash + 2];
    memcpy(path, argv, sizeof(path) - 1);
    path[lastslash+1] = 0;

    __attribute__((unused)) int x = chdir(path);
}

int parse_move(char *move_string) {

    MoveList legalMoves;
    legalMoves.count = 0;
    generate_moves(&legalMoves, &UciBoard);

    int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

    for (int move_count = 0; move_count < legalMoves.count; move_count++) {

        int move = legalMoves.moves[move_count];
        int promoted_piece = getpromoted(move);

        if ((getsource(move) == source_square) && (gettarget(move) == target_square)) {
            int promoted_piece = getpromoted(move);
            if (promoted_piece) {

                if (move_string[4] == 'q' && (promoted_piece == Q || promoted_piece == q)) {
                    return move;
                } else if (move_string[4] == 'b' && (promoted_piece == B || promoted_piece == b)) {
                    return move;
                } else if (move_string[4] == 'r' && (promoted_piece == R || promoted_piece == r)) {
                    return move;
                } else if (move_string[4] == 'n' && (promoted_piece == N || promoted_piece == n)) {
                    return move;
                }

            } else {
                return move;
            }
        }
    }
    return 0;
}


void parse_position(char *command) {
    uci_move_sequence_length = 0;
    command += 9;

    char *current_char = command;

    if (strncmp(command, "startpos", 8) == 0) {
        parse_fen(start_position, &UciBoard);

    } else {

        current_char = strstr(command, "fen");

        if (current_char == NULL)
            parse_fen(start_position, &UciBoard);
        else {
            current_char += 4;
            parse_fen(current_char, &UciBoard);
        }

    }

    current_char = strstr(command, "moves");
    if (current_char != NULL) {
        current_char += 6;
        while (*current_char) {

            int move = parse_move(current_char);
            uci_move_sequence_length++;

            make_move(move, all_moves, 1, &UciBoard);

            while ((*current_char) && (*current_char != ' ')) {
                current_char++;
            }

            current_char++;

        }
    }

    UciBoard.ply = 0;
    UciBoard.searchColor = UciBoard.side;
}

int depth;

int threadCount = 1;

void parse_go(char *command) {
    char *current_depth = NULL;
    char *current_timelimit = NULL;
    char *current_wtime = NULL;
    char *current_btime = NULL;

    int wtime = 0;
    int btime = 0;

    current_depth = strstr(command, "depth");
    if (current_depth) {
        depth = atoi(current_depth + 6);
        moveTime = 10000000;
    } else {
        moveTime = 1000;
        depth = max_ply;
    }

    current_timelimit = strstr(command, "movetime");
    if (current_timelimit) {
        depth = max_ply;
        moveTime = atoi(current_timelimit + 9);
    }

    char *infinite = strstr(command, "infinite");
    if (infinite) {
        depth = max_ply;
        moveTime = 10000000;
    }

    current_wtime = strstr(command, "wtime");
    if (current_wtime) {
        depth = max_ply;

        wtime = atoi(current_wtime + 6);

        current_btime = strstr(command, "btime");
        btime = atoi(current_btime + 6);

        dynamicTimeManagment = 1;
        moveTime = choose_movetime(wtime, btime, UciBoard.side);
    }

    pthread_create(&searchthread, NULL, search_position, &depth);\
    if(!searchthread){
	printf("Failed to create searchthread");
    }
}

void uci_loop() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char input[5000];
    printf("Eggnog Chess Engine by Levi Gibson\n");

    while (1) {
        memset(input, 0, sizeof(input));
        fflush(stdout);
        if (!fgets(input, 5000, stdin)) {
            continue;
        }

        if (input[0] == '\n') {
            continue;
        } else {
            stop = 1;
	    if(searchthread){
            	pthread_join(searchthread, NULL);
	    }
            stop = 0;
        }

        if (strncmp(input, "isready", 7) == 0) {
            printf("readyok\n");
        }

        if (strncmp(input, "position", 8) == 0) {
            parse_position(input);
        }

        if (strncmp(input, "ucinewgame", 10) == 0) {
            parse_position("position startpos");

        }
        if (strncmp(input, "go", 2) == 0) {
            parse_go(input);
        }
        if (strncmp(input, "quit", 4) == 0) {
            break;
        }
        if (strncmp(input, "uci", 3) == 0) {
            printf("id name Eggnog Chess\n");
            printf("id name Levi Gibson\n");
            printf("option name SyzygyPath type string default <empty>\n");
            printf("option name Threads type spin default 1 min 1 max 512\n");
            printf("option name Move Overhead type spin default 0 min 0 max 5000\n");
            printf("option name Tune type spin default 0 min 0 max 10000\n");
            printf("uciok\n");
        }

        if (strncmp(input, "setoption name Threads value", 28) == 0) {
            threadCount = atoi(input + 29);
        }

        if (strncmp(input, "setoption name Move Overhead value", 34) == 0) {
            moveOverhead = atoi(input + 35);
        }

        if (strncmp(input, "setoption name Tune value", 25) == 0){
        //    worthSearchingLimit = atoi(input + 26);
        //    calculate_ws_table();
        }

        if (strncmp(input, "setoption name SyzygyPath value", 31) == 0) {

            char path[strlen(input) - 32];
            memset(path, 0, sizeof path);
            strncpy(path, input + 32, sizeof(path) - 1);

            tb_init(path);
        }

    }
}

