import random

import chess
from chess import Board

infile = open("s_chessData.csv", 'r')
outfile = open("quietPositions.csv", 'w')
infile.readline()

def process_line(line):
    return (line.split(',')[1], int(line.split(',')[2]))

board = Board()
linesProcessed = 0
succeededLines = 0

for line in infile:

    if line.split(',')[2][0] == '#':
        continue

    try:
        lineList = process_line(line)
    except:
        continue
    board.set_fen(lineList[0])

    if board.is_check() == False and len(list(board.generate_pseudo_legal_captures())) == 0:
        if random.randint(0, len(board.piece_map())) != 1:
            outfile.write(line)
            succeededLines += 1

    linesProcessed += 1
    if linesProcessed % 10000 == 0:
        print(linesProcessed / 12958035, end='')
        print(" : ", end='')
        print(succeededLines)