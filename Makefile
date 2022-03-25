CFILES = main.c\
 syzygy.c\
 bitboard.c\
 board.c\
 perft.c\
 uci.c\
 search.c\
 timeman.c\
 transposition.c\
 Fathom/*.c\
 nnue.c\
 see.c\
 nnom.c

AVX2_OBJS = search.c.avx2.o \
       nnue.c.avx2.o \
       board.c.avx2.o \
       main.c.avx2.o \
       syzygy.c.avx2.o \
       bitboard.c.avx2.o \
       perft.c.avx2.o \
       uci.c.avx2.o \
       timeman.c.avx2.o \
       transposition.c.avx2.o \
       Fathom/tbchess.c.avx2.o \
       Fathom/tbprobe.c.avx2.o \
       see.c.avx2.o\
       nnom.c.avx2.o

AVX_OBJS = search.c.avx.o \
       nnue.c.avx.o \
       board.c.avx.o \
	   main.c.avx.o \
	   syzygy.c.avx.o \
	   bitboard.c.avx.o \
	   perft.c.avx.o \
	   uci.c.avx.o \
	   timeman.c.avx.o \
	   transposition.c.avx.o \
	   Fathom/tbchess.c.avx.o \
	   Fathom/tbprobe.c.avx.o \
	   see.c.avx.o\
	   nnom.c.avx2.o

SSE2_OBJS = search.c.sse2.o \
       nnue.c.sse2.o \
       board.c.sse2.o \
       main.c.sse2.o \
       syzygy.c.sse2.o \
       bitboard.c.sse2.o \
       perft.c.sse2.o \
       uci.c.sse2.o \
       timeman.c.sse2.o \
       transposition.c.sse2.o \
       Fathom/tbchess.c.sse2.o \
       Fathom/tbprobe.c.sse2.o \
       see.c.sse2.o\
       nnom.c.sse2.o

SSE_OBJS = search.c.sse.o \
       nnue.c.sse.o \
       board.c.sse.o \
	   main.c.sse.o \
	   syzygy.c.sse.o \
	   bitboard.c.sse.o \
	   perft.c.sse.o \
	   uci.c.sse.o \
	   timeman.c.sse.o \
	   transposition.c.sse.o \
	   Fathom/tbchess.c.sse.o \
	   Fathom/tbprobe.c.sse.o \
	   see.c.sse.o\
	   nnom.c.sse.o

POPCNT_OBJS = search.c.popcnt.o \
       nnue.c.popcnt.o \
       board.c.popcnt.o \
	   main.c.popcnt.o \
	   syzygy.c.popcnt.o \
	   bitboard.c.popcnt.o \
	   perft.c.popcnt.o \
	   uci.c.popcnt.o \
	   timeman.c.popcnt.o \
	   transposition.c.popcnt.o \
	   Fathom/tbchess.c.popcnt.o \
	   Fathom/tbprobe.c.popcnt.o \
	   see.c.popcnt.o\
	   nnom.c.popcnt.o

OS = linux
RELEASE = false
FILE = ./bin/eggnog-chess-engine
COMMONFLAGS = -O3 -fcommon

ifeq ($(RELEASE), true)
COMMONFLAGS = -O3 -fcommon -DRELEASE
endif

ifeq ($(OS), linux)

EXECUTABLE_FILENAME =
CC=clang
LINK_OPTS = -lpthread -lm

else ifeq ($(OS), mac)

EXECUTABLE_FILENAME =.dmg
CC=gcc
LINK_OPTS = -lpthread -lm

else
EXECUTABLE_FILENAME =.exe
CC=x86_64-w64-mingw32-gcc
LINK_OPTS = -l:libwinpthread.a -lm
endif

release:
	mkdir ./bin/eggnog-windows
	mkdir ./bin/eggnog-linux
	cp ./bin/moveOrderData.bin ./bin/eggnog-windows/
	cp ./bin/moveOrderData.bin ./bin/eggnog-linux/
	cp ./bin/nn-eba324f53044.nnue ./bin/eggnog-linux/
	cp ./bin/nn-eba324f53044.nnue ./bin/eggnog-windows/

	make all
	mv ./bin/eggnog-chess-engine* ./bin/eggnog-linux/
	make clean
	make all OS=win
	mv ./bin/eggnog-chess-engine* ./bin/eggnog-windows/
	make clean


all: avx2 avx sse sse2 popcnt
avx2: $(AVX2_OBJS)
	$(CC) $(OBJS) $(AVX2_OBJS) $(LINK_OPTS) -o $(FILE)-avx2-$(OS)$(EXECUTABLE_FILENAME)
avx:  $(AVX_OBJS)
	$(CC) $(OBJS) $(AVX_OBJS) $(LINK_OPTS) -o $(FILE)-avx-$(OS)$(EXECUTABLE_FILENAME)
sse: $(SSE_OBJS)
	$(CC) $(OBJS) $(SSE_OBJS) $(LINK_OPTS) -o $(FILE)-sse-$(OS)$(EXECUTABLE_FILENAME)
sse2: $(SSE2_OBJS)
	$(CC) $(OBJS) $(SSE2_OBJS) $(LINK_OPTS) -o $(FILE)-sse2-$(OS)$(EXECUTABLE_FILENAME)
popcnt: $(POPCNT_OBJS)
	$(CC) $(OBJS) $(POPCNT_OBJS) $(LINK_OPTS) -o $(FILE)-popcnt-$(OS)$(EXECUTABLE_FILENAME)

%.c.avx2.o: %.c
	$(CC) $< $(COMMONFLAGS) -D AVX2 -mavx2 -c -o $@
%.c.sse.o: %.c
	$(CC) $< $(COMMONFLAGS) -D SSE -msse -c -o $@
%.c.sse2.o: %.c
	$(CC) $< $(COMMONFLAGS) -D SSE2 -msse2 -c -o $@
%.c.avx.o: %.c
	$(CC) $< $(COMMONFLAGS) -D AVX -mavx -c -o $@
%.c.popcnt.o: %.c
	$(CC) $< $(COMMONFLAGS) -D POPCNT -mpopcnt -c -o $@
mingw:
	make OS=win
mingwj:
	make OS=win -j
debug:
	$(CC) $(FILES) -pthread -o $(FILE)-debug
gdb:
	$(CC) $(COMMONFLAGS) -DAVX2 -mavx2 $(LINK_OPTS) $(CFILES) -g
	mv ./a.out ./bin/a.out
prof:
	$(CC) -pg $(LINK_OPTS) -fcommon -DAVX2 -mavx2 -O3 $(CFILES) -o $(FILE)-prof
clean:
	rm -f ./bin/a.out ./bin/gmon.out
	rm -f ./bin/eggnog-chess-engine*
	rm -f *.s
	rm -f *.o nnue/*.o Fathom/*.o
	rm -f $(AVX2_OBJS) $(AVX_OBJS) $(SSE2_OBJS) $(SSE_OBJS) $(POPCNT_OBJS)
