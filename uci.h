#ifndef MBBCHESS_UCI_H
#define MBBCHESS_UCI_H

#include "board.h"

int uci_move_sequence_length;
int dynamicTimeManagment;

Board UciBoard;
int threadCount;

void parse_position(char *command);
void parse_go(char *command);
void uci_loop();
int parse_move(char * move_string);

#endif

