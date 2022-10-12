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

## Run
eggnog-chess-engine uses the UCI protocol. It must be run with a GUI such as [Arena](http://www.playwitharena.de/).

If you move the executable (bin/eggnog-chess-engine-xxx) to a different folder, be sure to move these files as well:

    bin/network.nnue
    bin/network.nnom

## NNUE
eggnog-chess-engine uses a strage but functional type of NNUE.
As of October 12, 2022, eggnog-chess-engine is using a small network with a split accumulator. Instead of using HalfKp, the network feeds the positions of the pieces into the network as a sparse array with a size of 784 (12*64). This is then duplicated for the black perspective and fed through the accumulator (128 neurons per side). The two accumulators are then combined into 1 layer with the size of 256, and then it's a simple 32x32x1 to finish. In short, it's a more basic and dumb-downed version of HalfKp. I have an old laptop generating training data around the clock though so hopefully I should have enough for a proper large network soon! The training data is generated with Stockfish which is kind of cheating but whose counting?

## NNOM
eggnog-chess-engine uses NNOM, or Move Ordering Neural Network. It functions very simialrly to NNUE, but only has one hidden layer. 
The only hidden layer is updated the same wasy as NNUE. 
labels are 384 (6*64) values, each corrosponding to a move.
Only the output neurons that correspond to legal moves are calculated. 
This makes the process of running NNOM as quick as updating the first layer, and calculating a few output neurons.
Training code for the eggnog NNOM can be found [here](https://github.com/LeviGibson/nnom-trainer.git).
