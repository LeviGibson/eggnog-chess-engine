import chess
import generateMoveTable

def read_evaluations(line):
    new = ""
    inComment = False

    for ch in line:
        if ch == '{': inComment = True

        if not inComment:
            new += ch

        if ch == '}': inComment = False

    moves = []
    new = new.split(" ")

    for id, obj in enumerate(new):
        if not ((obj == '') or ('.' in obj) or (obj[0] == "$") or (obj[-1] == "\n")):
            moves.append(obj)

    return moves

def clean_move(move : str):
    move = move.replace("?", "")
    move = move.replace("!", "")
    return move

gameId = 0
infile = open("analyed.pgn")
for line in infile:
    gameId+=1

    if gameId % 100 == 0:print(gameId)
    if gameId == 80000: break

    moves = read_evaluations(line)

    board = chess.Board()

    for id, sMove in enumerate(moves):
        move = board.parse_san(clean_move(sMove))
        if "?" not in sMove:
            generateMoveTable.write_move(board, move)
        board.push(move)

generateMoveTable.save()
