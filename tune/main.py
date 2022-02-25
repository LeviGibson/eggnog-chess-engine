import puzzles
import matplotlib.pyplot as plt
import numpy as np
import multiprocessing

processes = []
scores = []

testingvalue = 10
while testingvalue < 200:
    print(testingvalue)
    processes.append(multiprocessing.Process(target=puzzles.run, args=(testingvalue, 170)))
    # scores.append(score)
    #
    # plt.plot(scores)
    # plt.savefig("plot.png")
    
    # print(scores)
    testingvalue += 25

for id, obj in enumerate(processes):
    processes[id].start()

for id, obj in enumerate(processes):
    processes[id].join()