infile = "edgeList1.txt"

with open(infile) as f:
    lines = f.readlines()

    for line in lines:
        line = line.split('\t');
        print line[0],'\t',line[1]
