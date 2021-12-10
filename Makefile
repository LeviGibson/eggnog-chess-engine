FILES = main.c\
 syzygy.c\
 bitboard.c\
 board.c\
 perft.c\
 uci.c\
 search.c\
 timeman.c\
 transposition.c\
 moveOrder.c\
 Fathom/*.c

NNUEFILES = nnue/load.c nnue/propogate.c
FILE = ./bin/eggnog-chess-engine
COMMONFLAGS = -O3 -pthread
WINFLAGS = -l:libwinpthread.a

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
	mv ./a.out ./bin/a.out
prof:
	gcc -pg -DAVX2 -mavx2 -pthread -O3 $(FILES) $(NNUEFILES) -o $(FILE)-prof
win_avx2:
	x86_64-w64-mingw32-gcc -o $(FILE)-avx2.exe $(FILES) $(NNUEFILES) -mavx2 -DAVX2 $(COMMONFLAGS) $(WINFLAGS)
win_avx:
	x86_64-w64-mingw32-gcc -o $(FILE)-avx.exe $(FILES) $(NNUEFILES) -mavx -DAVX $(COMMONFLAGS) $(WINFLAGS)
win_sse:
	x86_64-w64-mingw32-gcc -o $(FILE)-sse.exe $(FILES) $(NNUEFILES) -msse -DSSE $(COMMONFLAGS) $(WINFLAGS)
win_sse2:
	x86_64-w64-mingw32-gcc -o $(FILE)-sse2.exe $(FILES) $(NNUEFILES) -msse2 -DSSE2 $(COMMONFLAGS) $(WINFLAGS)
win_popcnt:
	x86_64-w64-mingw32-gcc -o $(FILE)-sse2.exe $(FILES) $(NNUEFILES) -mpopcnt -DPOPCNT $(COMMONFLAGS) $(WINFLAGS)
clean:
	rm -f *.gch
	rm -f nnue/*.gch
	rm -f Fathom/*.gch
	rm -f ./bin/a.out
	rm -f ./bin/eggnog-chess-engine*