import puzzles

scores = []

testingvalue = 10
while testingvalue < 10000:
    print(testingvalue)
    score = puzzles.run(testingvalue)
    scores.append(score)

    print(scores)
    testingvalue += 50
