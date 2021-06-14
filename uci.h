#ifndef MBBCHESS_UCI_H
#define MBBCHESS_UCI_H

void parse_position(char *command);
void parse_go(char *command);
void uci_loop();
int parse_move(char * move_string);

#endif
