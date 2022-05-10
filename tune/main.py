import puzzles
import threading
import numpy as np
import matplotlib.pyplot as plt

p1 = 600
p2 = 25

threadCount = 12
threadsFinished = 0

scores = np.zeros((p1 // p2))
threads = []

testingvalue = 0

def run(x, y):
    scores[y] = puzzles.run(x)

for i in range(p1//p2):
    index = i - threadsFinished
    threads.append(threading.Thread(target=run, args=[testingvalue, i]))
    threads[index].start()
    testingvalue += p2

    if (i and i % threadCount == 0) or i == (p1//p2)-1:
        for t in threads:
            t.join()
            threadsFinished+=1
        threads.clear()

plt.plot(scores)
plt.savefig("plot.png")
