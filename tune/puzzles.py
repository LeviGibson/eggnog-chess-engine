import chess.engine
import random

import logging
logging.basicConfig(level=logging.CRITICAL)

def run(value : int, default : int):
    dengine = chess.engine.SimpleEngine.popen_uci("../bin/eggnog-chess-enging-avx2-linux")
    tengine = chess.engine.SimpleEngine.popen_uci("../bin/eggnog-chess-enging-avx2-linux")

    # dengine.configure({"Threads" : 6})
    dengine.configure({"Tune" : default})
    dengine.configure({"SyzygyPath" : "/home/levigibson/Documents/static/arenalinux_64bit_3.10beta/TB/syzygy"})

    # tengine.configure({"Threads": 6})
    tengine.configure({"Tune": value})
    tengine.configure({"SyzygyPath": "/home/levigibson/Documents/static/arenalinux_64bit_3.10beta/TB/syzygy"})
    score = 0

    for game in range(20):
        board = chess.Board()

        while not board.is_game_over():

            result = None
            if board.turn == chess.WHITE:
                result = tengine.analyse(board, chess.engine.Limit(time=random.uniform(.4, .6)))
            else:
                result = tengine.analyse(board, chess.engine.Limit(time=random.uniform(.4, .6)))

            try:
                if result['score'].relative.score() > 300:
                    if board.turn == chess.BLACK: score -= 1; break
                    elif board.turn == chess.WHITE: score += 1; break
            except:
                break

            if result['score'].relative.score() < -300:
                if board.turn == chess.WHITE: score -= 1; break
                elif board.turn == chess.BLACK: score += 1; break

            board.push(result['pv'][0])
            if board.is_checkmate():
                if board.turn == chess.WHITE: score -= 1; break
                elif board.turn == chess.BLACK: score += 1; break

            if board.is_stalemate() or board.is_repetition(3) or board.is_fifty_moves():
                break
        print(game)
    print()
    print(value, score)
    tengine.quit()
    dengine.quit()

    # board = chess.Board()
    #
    # total = 0
    # correct = 0
    # index = 0
    #
    # while index < 100:
    #
    #     command = puzzleFile.readline()
    #     # if 'kingsideAttack' not in command:continue
    #
    #     command = command.split(',')
    #
    #     if (int(command[3]) < 2600):
    #         continue
    #
    #     index += 1
    #
    #     board.set_fen(command[1])
    #     board.push_uci(command[2].split(' ')[0])
    #
    #     result = engine.play(board, chess.engine.Limit(time=.75))
    #
    #     total += 1
    #     if str(result.move) == command[2].split(' ')[1]:
    #         correct += 1
    #
    # engine.quit()
    #
    # return correct

# if __name__ == '__main__':
#     run(2000)
