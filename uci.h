#ifndef MBBCHESS_UCI_H
#define MBBCHESS_UCI_H

#include "board.h"

int uci_move_sequence_length;
int dynamicTimeManagment;
int historyMoveDivisor;

Board UciBoard;
int threadCount;
int moveOverhead;

void change_to_correct_directory(char *argv);
void parse_position(char *command);
void parse_go(char *command);
void uci_loop();
int parse_move(char * move_string);

#endif

