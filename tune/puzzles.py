import chess.engine
import pickle
import logging

import matplotlib.pyplot as plt

logging.basicConfig(level=logging.CRITICAL)

def run(value : int):

    #download this large file with `wget https://database.lichess.org/lichess_db_puzzle.csv.bz2`
    inf = open("data.bin", 'rb')
    inf.seek(0)
    data = pickle.load(inf)

    engine = chess.engine.SimpleEngine.popen_uci("../bin/eggnog-chess-engine-avx2-linux")
    engine.configure({"Tune" : value})

    board = chess.Board()

    score = 0
    scorecount = 0

    allscores = []

    for sample in data:
        try:
            board.set_fen(sample[0])

            moves : list = sample[1]
            evals = sample[2]

            result = engine.play(board, chess.engine.Limit(nodes=10000))
            move = result.move
            index = moves.index(move)
            score += evals[index]
            scorecount+=1
            print(scorecount, score/scorecount)
            allscores.append(score/scorecount)
            # if scorecount > 100:
            #     break
        except:
            pass

    engine.quit()
    # plt.plot(allscores)
    # plt.show()
    return score / scorecount

if __name__ == '__main__':
    print(run(2000))
