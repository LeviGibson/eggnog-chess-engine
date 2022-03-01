import puzzles
import matplotlib.pyplot as plt
import numpy as np
import multiprocessing

processes = []
scores = []

testingvalue = 0
while testingvalue < 5000:
    print(testingvalue)
    processes.append(multiprocessing.Process(target=puzzles.run, args=(testingvalue, 1700)))
    # scores.append(score)
    #
    # plt.plot(scores)
    # plt.savefig("plot.png")
    
    # print(scores)
    testingvalue += 1000

for id, obj in enumerate(processes):
    processes[id].start()

for id, obj in enumerate(processes):
    processes[id].join()