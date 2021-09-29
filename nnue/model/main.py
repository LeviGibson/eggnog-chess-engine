import chess
from chess import Board, WHITE, BLACK
import numpy as np

scores = [100, 300, 300, 500, 900, 0]
piece_indicies = [[0, 1, 2, 3, 4, 5], [6, 7, 8, 9, 10, 11]]

features = []
labels = []
fens = []


def evaluate(f_board):
    eval = 0

    pm = f_board.piece_map()
    for piece in pm:
        if pm[piece].color == WHITE:
            eval += scores[pm[piece].piece_type - 1]
        else:
            eval -= scores[pm[piece].piece_type - 1]
    return eval


def process_line(line):
    return (line.split(',')[1], int(line.split(',')[2]))


def get_feature_index(sq, pt):
    return (pt * 64) + sq


def process_eval(x):
    # Modified sigmoid
    x = x / 100
    fx = (1 / (2.71828 ** (-.4 * x) + 1)) * 2 - 1
    return int(fx * 100)


def generate_features(board):
    pmap = board.piece_map()

    features = np.zeros(shape=(768,), dtype=bool)

    for id in pmap:
        features[get_feature_index(id, piece_indicies[int(pmap[id].color)][pmap[id].piece_type - 1])] = True

    return features

def generate_dataset():
    board = Board()
    infile = open("quietPositions.csv", 'r')

    infile.readline()
    linesProcessed = 0

    for line in infile:

        if line.split(',')[2][0] == '#':
            continue

        linesProcessed += 1
        line = process_line(line)

        board.set_fen(line[0])

        features.append(generate_features(board))
        labels.append(process_eval(line[1]))
        fens.append(line[0])

        if linesProcessed % 1000 == 0:
            print(linesProcessed)

        if linesProcessed == 1300000:
            np.savez_compressed("train/features", np.array(features))
            np.save("train/labels", np.array(labels))
            np.save("train/fens", np.array(fens))
            exit()

generate_dataset()
