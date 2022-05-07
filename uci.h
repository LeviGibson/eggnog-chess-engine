#ifndef MBBCHESS_UCI_H
#define MBBCHESS_UCI_H

#include "board.h"

int32_t uci_move_sequence_length;
int32_t dynamicTimeManagment;
int32_t tuneParameter;

Board UciBoard;
int32_t threadCount;
int32_t moveOverhead;

int32_t wtime;
int32_t btime;

void change_to_correct_directory(char *argv);
void parse_position(char *command);
void parse_go(char *command);
void uci_loop();
int32_t parse_move(char * move_string);

#endif

