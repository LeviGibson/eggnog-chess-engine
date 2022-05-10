from stockfish import Stockfish
import chess
import chess.pgn
from math import pow
import numpy as np
import pickle

stockfish = Stockfish(path="./sf14")
stockfish.set_depth(15)

def sigmoid(x : float):
    return 1/(1+pow(2.71828, -x))

def process_move_scores(moves, scores, board):
    scores = np.array(scores)
    if board.turn == chess.WHITE:
        scores = 1-scores

    return moves, scores - min(scores)

def get_move_scores(board : chess.Board):
    legalMoves = list(board.legal_moves)
    moveScores = []

    for move in legalMoves:
        board.push(move)

        stockfish.set_fen_position(board.fen())
        sfeval = stockfish.get_evaluation()
        movescore = sigmoid(float(sfeval['value']) / 100)

        if sfeval['type'] == 'mate':
            movescore = 1 if sfeval['value'] > 0 else 0

        moveScores.append(movescore)
        board.pop()

    return process_move_scores(legalMoves, moveScores, board)

if __name__ == '__main__':
    data = []

    infile = open("data.pgn", 'r')
    mid = 0

    for gid in range(10):
        game = chess.pgn.read_game(infile)
        mainline = list(game.mainline_moves())
        board = chess.Board()
        for move in mainline:
            moves, scores = get_move_scores(board)
            data.append([board.fen(), moves, scores])
            board.push(move)
            mid += 1
            print(mid)

    with open('data.bin', 'wb') as fh:
        pickle.dump(data, fh)
