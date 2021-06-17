all:
	gcc main.c bitboard.c bitboard.h board.c board.h perft.c perft.h uci.c uci.h search.c search.h evaluate.c evaluate.h timeman.c timeman.h transposition.c transposition.h -Ofast -o mbbChess
debug:
	gcc main.c bitboard.c bitboard.h board.c board.h perft.c perft.h uci.c uci.h search.c search.h evaluate.c evaluate.h timeman.c timeman.h transposition.c transposition.h -o mbbChess
run:
	./mbbChess
