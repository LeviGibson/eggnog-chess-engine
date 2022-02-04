infile = open("data.pgn")
outfile = open("analyed.pgn", 'w')
gamesFound = 0

pgn = ""
for id, line in enumerate(infile):
    if line[0] != "[":
        if "%eval" in line:
            pgn += line
            outfile.write(pgn)
            gamesFound+=1
            print(gamesFound)
        else:
            pgn = ""
            continue
