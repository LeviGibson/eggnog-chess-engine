FILES = main.c syzygy.c syzygy.h bitboard.c bitboard.h board.c board.h perft.c perft.h uci.c uci.h search.c search.h evaluate.c evaluate.h timeman.c timeman.h transposition.c transposition.h Fathom/*.c Fathom/*.h

all:
	gcc -Ofast $(FILES) -o eggnog-chess-engine
debug:
	gcc $(FILES) -o eggnog-chess-engine
prof:
	gcc -pg -no-pie -fno-builtin $(FILES) -o eggnog-chess-engine
win:
	x86_64-w64-mingw32-gcc -o eggnog-chess-engine.exe $(FILES) -Ofast
run:
	./eggnog-chess-engine