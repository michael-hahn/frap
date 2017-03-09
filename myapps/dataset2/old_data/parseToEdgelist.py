#This python program is used to transform a text file that contains edgelist information to edgelist format.

file = open("matlablist_progprofile.txt", "r")
data = file.readlines()

numberOfEdgelist = int(data[0].replace("[", "").replace("]", "")[-2:]) + 1

nodeTypeList = data[1].replace("[", "").replace("]", "").split(",")

edgeTypeList = data[2].replace("[", "").replace("]", "").split(",")

nodeMapList = list()

for num in range(numberOfEdgelist):
	nodeMapList.append({})

index = 0
dictNum = 0
while (index < len(nodeTypeList)):
	size = int(nodeTypeList[index])
	index2 = 0
	while (index2 < size):
		index = index + 1
		nodeMapList[dictNum][index2] = int(nodeTypeList[index].replace("\n", ""))
		index2 = index2 + 1
	index = index + 1
	dictNum = dictNum + 1

index = 0
dictNum = 0
while (index < len(edgeTypeList)):
	size = int(edgeTypeList[index])
	index2 = 0
	fileName = "edgeList" + str(dictNum) + ".txt"
	fp = open(fileName, "w")
	while (index2 < size):
		index = index + 1
		sourceIndex = edgeTypeList[index].replace("\n", "")
		fp.write(sourceIndex)
		fp.write("\t")
		index = index + 1
		destinationIndex = edgeTypeList[index].replace("\n", "")
		fp.write(destinationIndex)
		fp.write("\t")
		index = index + 1
		sourceType = str(nodeMapList[dictNum][int(sourceIndex)])
		destinationType = str(nodeMapList[dictNum][int(destinationIndex)])
		fp.write(sourceType)
		fp.write(":")
		fp.write(destinationType)
		fp.write(":")
		fp.write(edgeTypeList[index].replace("\n", ""))
		fp.write("\n")
		index2 = index2 + 1
	index = index + 1
	dictNum = dictNum + 1
	fp.close()
file.close()
