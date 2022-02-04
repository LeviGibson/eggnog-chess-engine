import puzzles

scores = []

testingvalue = 1000
while testingvalue < 3000:
    print(testingvalue)
    score = puzzles.run(testingvalue)
    scores.append(score)

    print(scores)
    testingvalue += 100
