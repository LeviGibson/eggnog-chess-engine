import puzzles

scores = []

testingvalue = 0
while testingvalue < 10:
    print(testingvalue)
    score = puzzles.run(testingvalue)
    scores.append(score)

    print(scores)
    testingvalue += 1
