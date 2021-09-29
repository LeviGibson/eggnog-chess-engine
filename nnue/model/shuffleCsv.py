import pandas as pd

dframe = pd.read_csv("chessData.csv")
dframe = dframe.sample(frac=1)

print('data shuffled')

with open("s_chessData.csv", 'w') as outfile:
    outfile.write(dframe.to_csv())