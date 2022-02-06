import puzzles

scores = []

testingvalue = 100
while testingvalue < 5000:
    print(testingvalue)
    score = puzzles.run(testingvalue)
    scores.append(score)

    print(scores)
    testingvalue += 100
