# eggnog-chess-engine

[Challenge me on Lichess](https://lichess.org/@/eggnog-chess-engine)

eggnog-chess-engine is an NNUE-based chess engine written by Levi Gibson.

## Build

Find out which instruction set you have [here](https://www.intel.com/content/www/us/en/support/articles/000057621/processors.html).

build for all CPUs:
`make all`

### build for spesific instruction set

`make avx2`

`make avx`

`make sse`

`make sse2`

## Run
eggnog-chess-engine uses the UCI protocol. It must be run with a GUI such as [Arena](http://www.playwitharena.de/).

If you move the executable (bin/eggnog-chess-engine-xxx) to a different folder, be sure to move these files as well:

    bin/defualt-nn.nnue
    bin/moveTable

