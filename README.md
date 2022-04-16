
# eggnog-chess-engine

  

[Challenge me on Lichess](https://lichess.org/@/eggnog-chess-engine)

  

eggnog-chess-engine is an NNUE-based chess engine written by Levi Gibson.

  

## Build

  

Find out which instruction set you have [here](https://www.intel.com/content/www/us/en/support/articles/000057621/processors.html).

  

build for all CPUs:

  

linux:`make`

  

mac:`make OS=mac`

  

windows:`make OS=win`

  

### build for spesific instruction set

  

`make avx2`

  

`make popcnt`

  

`make avx`

  

`make sse`

  

`make sse2`

## Neural Networks
### NNOM (move ordering neural network)
A neural network used for move ordering. Training code can be found [here](https://github.com/LeviGibson/policy-network).
### NNUE (experimental)
This is a rather new development. For the longest time Eggnog has used Stockfish nets. I've been recently been learning Tensorflow though and am attempting to train comperable networks. 
The stockfish net version is still available at the branch `sf-nnue`.

## Run

eggnog-chess-engine uses the UCI protocol. It must be run with a GUI such as [Arena](http://www.playwitharena.de/).

  

If you move the executable (bin/eggnog-chess-engine-xxx) to a different folder, be sure to move these files as well:

  

bin/network.nnue
bin/network.nnom
