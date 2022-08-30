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
Instead of using HalfKp or somthing similar, the positions of the pieces are fed directly to the neural network as a sparse array.
There is another set of features that traditional NNUE does not have, which is the side to move, and the material count.
The accumulator is much smaller than a traditional NNUE accumulator. It is only 128 neurons.
Training code for the eggnog NNUE can be found [here](https://github.com/LeviGibson/nnue-trainer-2.git).

## NNOM
eggnog-chess-engine uses NNOM, or Move Ordering Neural Network. It functions very simialrly to NNUE, but only has one hidden layer. 
The only hidden layer is updated the same wasy as NNUE. 
labels are 384 (6*64) values, each corrosponding to a move.
Only the output neurons that correspond to legal moves are calculated. 
This makes the process of running NNOM as quick as updating the first layer, and calculating a few output neurons.
Training code for the eggnog NNOM can be found [here](https://github.com/LeviGibson/nnom-trainer.git).
