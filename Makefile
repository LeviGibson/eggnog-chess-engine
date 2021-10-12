FILES = main.c\
 syzygy.c syzygy.h \
 bitboard.c bitboard.h \
 board.c board.h \
 perft.c perft.h \
 uci.c uci.h \
 search.c search.h \
 evaluate.c evaluate.h \
 timeman.c timeman.h \
 transposition.c transposition.h \
 moveOrder.c moveOrder.h \
 Fathom/*.c Fathom/*.h

NNUEFILES = nnue/load.c nnue/load.h nnue/propogate.c nnue/propogate.h

all:
	gcc -O2 $(FILES) $(NNUEFILES) -o eggnog-chess-engine
debug:
	gcc $(FILES) $(NNUEFILES) -o eggnog-chess-engine
prof:
	gcc -pg -no-pie -fno-builtin $(FILES) $(NNUEFILES) -o eggnog-chess-engine
win:
	x86_64-w64-mingw32-gcc -o eggnog-chess-engine.exe $(FILES) $(NNUEFILES) -O2
run:
	./eggnog-chess-engine