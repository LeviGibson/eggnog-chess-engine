from generateMoveTable import w_pers
import numpy as np
import matplotlib.pyplot as plt
import chess

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

moveAverages = np.load("data.npz")['arr_0']
print(moveAverages)

def show(ptype, dst):
    dst = w_pers[dst]

    movetotals = moveAverages[ptype][dst]

    for i in range(14):
        plt.imshow(movetotals[i].reshape(8,8,1), cmap='gray')
        plt.show()

show(N, chess.F5)