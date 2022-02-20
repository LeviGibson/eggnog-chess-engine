import chess.engine

import logging
logging.basicConfig(level=logging.CRITICAL)

def run(value : int):

    #download this large file with `wget https://database.lichess.org/lichess_db_puzzle.csv.bz2`
    puzzleFile = open("lichess_db_puzzle.csv", 'r')

    engine = chess.engine.SimpleEngine.popen_uci("../bin/eggnog-chess-engine-avx2-linux")
    engine.configure({"Tune" : value})

    board = chess.Board()

    total = 0
    correct = 0
    index = 0

    while index < 100:

        command = puzzleFile.readline()
        # if 'kingsideAttack' not in command:continue

        command = command.split(',')

        if (int(command[3]) < 2600):
            continue

        index += 1

        board.set_fen(command[1])
        board.push_uci(command[2].split(' ')[0])

        result = engine.play(board, chess.engine.Limit(time=.2))

        total += 1
        if str(result.move) == command[2].split(' ')[1]:
            correct += 1

    engine.quit()

    return correct

if __name__ == '__main__':
    run(2000)