import puzzles
import threading
import numpy as np
import matplotlib.pyplot as plt

p1 = 30000
p2 = 2000

threadCount = 8
threadsFinished = 0

testingvalue = 1

scores = np.zeros(((p1-testingvalue) // p2))
threads = []

def run(x, y):
    z = puzzles.run(x)
    scores[y] = z
    print(y, x, z)

finalIndex = (p1-testingvalue)//p2 - 1

for i in range((p1-testingvalue)//p2):
    index = i - threadsFinished
    threads.append(threading.Thread(target=run, args=[testingvalue, i]))
    threads[index].start()
    testingvalue += p2

    if (i and i % threadCount == 0) or (i == finalIndex):
        for t in threads:
            t.join()
            threadsFinished += 1
        threads.clear()

    plt.plot(scores)
    plt.savefig("plot.png")
