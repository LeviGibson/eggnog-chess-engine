import chess
import chess.pgn
import numpy as np

infile = open("data.pgn")

WWS = 12
BWS = 13

def count_set_bits(n):
    count = 0
    while (n):
        count += n & 1
        n >>= 1
    return count

w_pers = [
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
    8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 2, 3, 4, 5, 6, 7
]

#data.pgn is any large pgn file with a bunch of games
#in production, the lichess database was used (https://database.lichess.org/standard/lichess_db_standard_rated_2017-05.pgn.bz2)

pieceSampleCount = 0
pieceAverages = np.zeros((15, 14, 64))

moveSampleCount = np.zeros((15, 12, 64))
moveAverages = np.zeros((15, 12, 64, 14, 64))

def piece_to_ordinal(piece : chess.Piece):
    return (piece.piece_type-1) + (6*(not piece.color))

def writearray(arr) -> str:
    res = '{'
    for val in arr:
        if type(val) == np.float64:
            res += str(val)
        else:
            res += writearray(val)
        res += ','

    res += '}'
    return res

def save():

    global moveAverages
    for c in range(15):
        for p in range(12):
            for s in range(64):
                if moveSampleCount[c][p][s] == 0:
                    moveAverages[c][p][s] *= 0
                    continue
                moveAverages[c][p][s] /= moveSampleCount[c][p][s]
                moveAverages[c][p][s] -= (pieceAverages[c] / pieceSampleCount)

    moveAverages *= 100000

    outfile = open("../bin/moveOrderData.bin", 'wb')

    # outfile.write("#include \"moveOrderData.h\"\n")
    # outfile.write("const float moveOrderData[15][12][64][14][64] = ")
    #
    # outfile.write(writearray(moveAverages))
    #
    # outfile.write(";\n")
    # outfile.close()
    outfile.write(bytes(moveAverages))

    np.savez_compressed("data", moveAverages)
    infile.close()

def write_move(board : chess.Board, move : chess.Move):
    global moveAverages
    global pieceSampleCount

    piececount = count_set_bits(board.rooks | board.knights | board.bishops | board.queens)
    piececount = min(14, piececount)

    if board.is_capture(move):
        return

    pmap = board.piece_map()
    movedPiece = piece_to_ordinal(pmap[move.from_square])

    for sq in pmap:
        if pmap[sq].color == chess.WHITE and not board.is_attacked_by(chess.WHITE, sq):
            moveAverages[piececount][movedPiece][w_pers[move.to_square]][WWS][w_pers[sq]] += 1
            pieceAverages[piececount][WWS][w_pers[sq]] += 1
        if pmap[sq].color == chess.BLACK and not board.is_attacked_by(chess.BLACK, sq):
            moveAverages[piececount][movedPiece][w_pers[move.to_square]][BWS][w_pers[sq]] += 1
            pieceAverages[piececount][BWS][w_pers[sq]] += 1

    for id in pmap:
        piece = piece_to_ordinal(pmap[id])
        id = w_pers[id]

        moveAverages[piececount][movedPiece][w_pers[move.to_square]][piece][id] += 1
        pieceAverages[piececount][piece][id] += 1

        moveSampleCount[piececount][movedPiece][w_pers[move.to_square]] += 1
        pieceSampleCount += 1

def main():
    global moveAverages
    global pieceSampleCount

    for gameid in range(100000):
        print(gameid)
        game = chess.pgn.read_game(infile)
        moves = game.mainline_moves()

        board = chess.Board()

        for move in moves:
            write_move(board, move)
            board.push(move)

    save()

if __name__ == '__main__':
    main()