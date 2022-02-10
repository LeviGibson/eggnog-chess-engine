import puzzles

scores = []

testingvalue = 15
while testingvalue < 35:
    print(testingvalue)
    score = puzzles.run(testingvalue)
    scores.append(score)

    print(scores)
    testingvalue += 1
