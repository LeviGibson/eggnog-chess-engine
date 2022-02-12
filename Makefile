OBJS = main.c.o\
 syzygy.c.o\
 bitboard.c.o\
 perft.c.o\
 uci.c.o\
 timeman.c.o\
 transposition.c.o\
 moveOrder.c.o\
 Fathom/tbchess.c.o\
 Fathom/tbprobe.c.o

CFILES = main.c\
 syzygy.c\
 bitboard.c\
 board.c\
 perft.c\
 uci.c\
 search.c\
 timeman.c\
 transposition.c\
 moveOrder.c\
 Fathom/*.c\
 nnue/nnue.c

AVX2_OBJS = search.c.avx2.o \
       nnue/nnue.c.avx2.o \
       board.c.avx2.o
AVX_OBJS = search.c.avx.o \
       nnue/nnue.c.avx.o \
       board.c.avx.o
SSE2_OBJS = search.c.sse2.o \
       nnue/nnue.c.sse2.o \
       board.c.sse2.o
SSE_OBJS = search.c.sse.o \
       nnue/nnue.c.sse.o \
       board.c.sse.o
POPCNT_OBJS = search.c.popcnt.o \
       nnue/nnue.c.popcnt.o \
       board.c.popcnt.o

OS = linux
FILE = ./bin/eggnog-chess-engine
COMMONFLAGS = -O3 -fcommon

ifeq ($(OS), linux)

EXECUTABLE_FILENAME =
CC=clang
LINK_OPTS = -lpthread

else ifeq ($(OS), mac)

EXECUTABLE_FILENAME =.dmg
CC=gcc
LINK_OPTS = -lpthread

else
EXECUTABLE_FILENAME =.exe
CC=x86_64-w64-mingw32-gcc
LINK_OPTS = -l:libwinpthread.a
endif


all: avx2 avx sse sse2 popcnt
avx2: $(OBJS) $(AVX2_OBJS)
	$(CC) $(OBJS) $(AVX2_OBJS) $(LINK_OPTS) -o $(FILE)-avx2-$(OS)$(EXECUTABLE_FILENAME)
avx:  $(OBJS) $(AVX_OBJS)
	$(CC) $(OBJS) $(AVX_OBJS) $(LINK_OPTS) -o $(FILE)-avx-$(OS)$(EXECUTABLE_FILENAME)
sse: $(OBJS) $(SSE_OBJS)
	$(CC) $(OBJS) $(SSE_OBJS) $(LINK_OPTS) -o $(FILE)-sse-$(OS)$(EXECUTABLE_FILENAME)
sse2: $(OBJS) $(SSE2_OBJS)
	$(CC) $(OBJS) $(SSE2_OBJS) $(LINK_OPTS) -o $(FILE)-sse2-$(OS)$(EXECUTABLE_FILENAME)
popcnt: $(OBJS) $(POPCNT_OBJS)
	$(CC) $(OBJS) $(POPCNT_OBJS) $(LINK_OPTS) -o $(FILE)-popcnt-$(OS)$(EXECUTABLE_FILENAME)

%.c.o: %.c
	$(CC) $< $(COMMONFLAGS) -c -o $@
%.c.avx2.o: %.c
	$(CC) $< $(COMMONFLAGS) -DAVX2 -mavx2 -c -o $@
%.c.avx.o: %.c
	$(CC) $< $(COMMONFLAGS) -DSSE -msse -c -o $@
%.c.sse2.o: %.c
	$(CC) $< $(COMMONFLAGS) -DSSE2 -msse2 -c -o $@
%.c.sse.o: %.c
	$(CC) $< $(COMMONFLAGS) -DAVX -mavx -c -o $@
%.c.popcnt.o: %.c
	$(CC) $< $(COMMONFLAGS) -DPOPCNT -mpopcnt -c -o $@
mingw:
	make OS=win
mingwj:
	make OS=win -j
debug:
	$(CC) $(FILES) -pthread -o $(FILE)-debug
gdb:
	$(CC) $(COMMONFLAGS) -DAVX2 -mavx2 $(CFILES) -g
	mv ./a.out ./bin/a.out
prof:
	$(CC) -pg -DAVX2 -mavx2 -pthread -O3 $(CFILES) -o $(FILE)-prof
clean:
	rm -f ./bin/a.out
	rm -f ./bin/eggnog-chess-engine*
	rm -f *.s
	rm -f *.o nnue/*.o Fathom/*.o
	rm -f $(AVX2_OBJS) $(AVX_OBJS) $(SSE2_OBJS) $(SSE_OBJS) $(POPCNT_OBJS)
