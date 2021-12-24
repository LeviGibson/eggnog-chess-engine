import chess
import chess.pgn
import numpy as np
import matplotlib.pyplot as plt

P = 0
N = 1
B = 2
R = 3
Q = 4
K = 5
p = 6
n = 7
b = 8
r = 9
q = 10
k = 11

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
#in production, the lichess database was used (https://database.lichess.org/standard/lichess_db_standard_rated_2014-01.pgn.bz2)
infile = open("data.pgn")

averageSampleCount = 0
pieceAverages = np.zeros((12, 64))

moveSampleCount = np.zeros((12, 64))
moveAverages = np.zeros((12, 64, 12, 64))

def piece_to_ordinal(piece : chess.Piece):
    return (piece.piece_type-1) + (6*(not piece.color))

for gameid in range(10000):
    print(gameid)
    game = chess.pgn.read_game(infile)
    moves = game.mainline_moves()

    board = chess.Board()

    for move in moves:
        pmap = board.piece_map()

        for id in pmap:
            piece = piece_to_ordinal(pmap[id])
            id = w_pers[id]

            pieceAverages[piece][id] += 1
            averageSampleCount += 1

            moveAverages[piece_to_ordinal(pmap[move.from_square])][w_pers[move.to_square]][piece][id] += 1
            moveSampleCount[piece_to_ordinal(pmap[move.from_square])][w_pers[move.to_square]] += 1

        board.push(move)

def show(ptype, dst):
    dst = w_pers[dst]

    movetotals = moveAverages[ptype][dst]
    movetotals /= moveSampleCount[ptype][dst]

    movetotals -= (pieceAverages/averageSampleCount)

    for i in range(12):
        plt.imshow(movetotals[i].reshape(8,8,1), cmap='gray')
        plt.show()

# show(P, chess.E5)

if __name__ == '__main__':

    for p in range(12):
        for s in range(64):
            if moveSampleCount[p][s] == 0:
                moveAverages[p][s] *= 0
                continue
            moveAverages[p][s] /= moveSampleCount[p][s]
            moveAverages[p][s] -= (pieceAverages/averageSampleCount)

    moveAverages *= 100000

    # plt.imshow(moveAverages[P][w_pers[chess.E4]][P].reshape(8,8,1), cmap='gray')
    # plt.show()


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



    outfile = open("moveOrderData.c", 'w')

    outfile.write("#include \"moveOrderData.h\"\n")
    outfile.write("float moveOrderData[12][64][12][64] = ")

    outfile.write(writearray(moveAverages))

    outfile.write(";\n")