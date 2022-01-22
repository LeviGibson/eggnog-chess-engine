#eggnog-chess-engine move ordering

This engine uses a `15 x 12 x 64 x 14 x 64 array`.

The values for this array are generated in `generateMoveTable.py`

Moves are scored in a similar method to piece square tables

The first index of the array is the number of pices left (excluding pawns and the king)

The next two indicies are the move we want to score (the piece type and the target square)

After you find this part of the array, you end up with a `14x64 array of floats`

This is a map of `where the pieces usually are when the said move is played`

Example:

Scoring the move Ng5 when there are 14 pieces left on the board:

```C
float score_move(int movePiece, int moveTarget, int pieceCount, Board *board){
    int score = 0;
    
    for (int piece = 0; piece < 12; piece++){
        for (int square = 0; square < 64; square++){
            
            //if there is a said piece at this square
            if (board.pieces[piece][square] == 1)
                score += moveScoreTable[pieceCount][movePiece][moveTarget][piece][target]
            else
                score -= moveScoreTable[pieceCount][movePiece][moveTarget][piece][target]
            
        }
    }
    
    return score;
}

int movescore = score_move(N, G5, 14, board)
```

There are two more "types of pieces" in this array, which represent the undefended pieces of black and white.

This helps with tactics.

This table is precompiled: `moveOrderData.c.linux.o` and `moveOrderData.c.win.o` and `moveOrderData.c.mac.o`

If you want to compile it yourself, you can download the C file [here](https://drive.google.com/file/d/1jTiHDq4BLt-bLkvjkR5gUyJp2_f8uNXF/view?usp=sharing)

Have fun :)

There's been so many people downloading and contributing, and it gives me a lot of motivation to keep making this engine. Thanks if you're reading this!