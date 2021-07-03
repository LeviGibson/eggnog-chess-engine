all:
	gcc main.c bitboard.c bitboard.h board.c board.h perft.c perft.h uci.c uci.h search.c search.h evaluate.c evaluate.h timeman.c timeman.h transposition.c transposition.h -Ofast -o eggnog-chess-engine
debug:
	gcc main.c bitboard.c bitboard.h board.c board.h perft.c perft.h uci.c uci.h search.c search.h evaluate.c evaluate.h timeman.c timeman.h transposition.c transposition.h -o eggnog-chess-engine
prof:
	gcc -pg -no-pie -fno-builtin main.c bitboard.c bitboard.h board.c board.h perft.c perft.h uci.c uci.h search.c search.h evaluate.c evaluate.h timeman.c timeman.h transposition.c transposition.h -o eggnog-chess-engine
run:
	./eggnog-chess-engine
