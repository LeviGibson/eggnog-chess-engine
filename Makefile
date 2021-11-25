FILES = main.c\
 syzygy.c syzygy.h \
 bitboard.c bitboard.h \
 board.c board.h \
 perft.c perft.h \
 uci.c uci.h \
 search.c search.h \
 timeman.c timeman.h \
 transposition.c transposition.h \
 moveOrder.c moveOrder.h \
 Fathom/*.c Fathom/*.h

NNUEFILES = nnue/load.c nnue/load.h nnue/propogate.c nnue/propogate.h
FILE = ./bin/eggnog-chess-engine
COMMONFLAGS = -O3 -pthread

all:
	make avx2 && make avx && make sse && make sse2 && make popcnt
release:
	make avx2 && make avx && make sse && make sse2 && make popcnt && make win_avx2 && make win_avx && make win_sse && make win_sse2 && make win_popcnt
avx2:
	gcc $(COMMONFLAGS) -DAVX2 -mavx2 $(FILES) $(NNUEFILES) -o $(FILE)-avx2-linux
avx:
	gcc $(COMMONFLAGS) -DAVX -mavx $(FILES) $(NNUEFILES) -o $(FILE)-avx-linux
sse:
	gcc $(COMMONFLAGS) -DSSE -msse $(FILES) $(NNUEFILES) -o $(FILE)-sse-linux
sse2:
	gcc $(COMMONFLAGS) -DSSE2 -msse2 $(FILES) $(NNUEFILES) -o $(FILE)-sse2-linux
popcnt:
	gcc $(COMMONFLAGS) -DPOPCNT -mpopcnt $(FILES) $(NNUEFILES) -o $(FILE)-popcnt-linux
debug:
	gcc $(FILES) $(NNUEFILES) -pthread -o $(FILE)-debug
gdb:
	gcc $(COMMONFLAGS) -DAVX2 -mavx2 $(FILES) $(NNUEFILES) -g
prof:
	gcc -pg $(FILES) $(NNUEFILES) -o $(FILE)-prof
win_avx2:
	x86_64-w64-mingw32-gcc -o $(FILE)-avx2.exe $(FILES) $(NNUEFILES) -mavx2 -DAVX2 $(COMMONFLAGS)
win_avx:
	x86_64-w64-mingw32-gcc -o $(FILE)-avx.exe $(FILES) $(NNUEFILES) -mavx -DAVX $(COMMONFLAGS)
win_sse:
	x86_64-w64-mingw32-gcc -o $(FILE)-sse.exe $(FILES) $(NNUEFILES) -msse -DSSE $(COMMONFLAGS)
win_sse2:
	x86_64-w64-mingw32-gcc -o $(FILE)-sse2.exe $(FILES) $(NNUEFILES) -msse2 -DSSE2 $(COMMONFLAGS)
win_popcnt:
	x86_64-w64-mingw32-gcc -o $(FILE)-sse2.exe $(FILES) $(NNUEFILES) -mpopcnt -DPOPCNT $(COMMONFLAGS)
