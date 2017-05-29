from sets import Set
import sys
infile = "edgeList2.txt"
outfile = "edgeList_rst.txt"
total_counter = 0
type_counter = 0
threshold = 500
seen_types = Set([])

output = open(outfile, 'w');

with open(infile) as f:
		lines = f.readlines()

		for line in lines:
			new_line = line.split('\t')
			types = new_line[2]
			if types in seen_types:
				type_counter += 1
				total_counter += 1
				output.write(line)
				if type_counter == threshold:
					output.close()
					print type_counter
					print total_counter
					sys.exit()
			else:
				seen_types.add(types)
				type_counter = 0
				total_counter += 1
				output.write(line)
				output.write("\n")
print type_counter
print total_counter