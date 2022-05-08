import chess.engine
import pickle
import logging

logging.basicConfig(level=logging.CRITICAL)

def run(value : int):

    #download this large file with `wget https://database.lichess.org/lichess_db_puzzle.csv.bz2`
    data = pickle.load(open("data.bin", 'rb'))

    engine = chess.engine.SimpleEngine.popen_uci("../bin/eggnog-chess-engine-avx2-linux")
    engine.configure({"Tune" : value})

    board = chess.Board()

    score = 0
    scorecount = 0

    for sample in data:
        board.set_fen(sample[0])

        moves : list = sample[1]
        evals = sample[2]

        result = engine.play(board, chess.engine.Limit(time=.25))
        move = result.move
        index = moves.index(move)
        score += evals[index]
        print(scorecount)
        scorecount+=1

    engine.quit()
    return score / len(data)

if __name__ == '__main__':
    run(2000)
