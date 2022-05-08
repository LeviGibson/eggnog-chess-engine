import puzzles
import threading
import numpy as np
import matplotlib.pyplot as plt

p1 = 600
p2 = 50

scores = np.zeros((p1 // p2))
threads = []

testingvalue = 0

def run(x, y):
    scores[y] = puzzles.run(x)

for i in range(p1//p2):
    threads.append(threading.Thread(target=run, args=[testingvalue, i]))
    threads[i].start()

    testingvalue += p2

for t in threads:
    t.join()

plt.plot(scores)
plt.savefig("plot.png")
