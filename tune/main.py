import puzzles
import matplotlib.pyplot as plt

scores = []

testingvalue = 10
while testingvalue < 1000:
    print(testingvalue)

    score = puzzles.run(testingvalue)
    scores.append(score)

    plt.plot(scores)
    plt.savefig("plot.png")

    print(scores)
    testingvalue += 10