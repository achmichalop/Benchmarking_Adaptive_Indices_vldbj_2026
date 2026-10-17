import sys
import os


# usage python3 convertDataToAVFormat DATA_FILE QUERY_FILE DATASIZE QUERYSIZE DIMENSIONS OUTPUTFOLDER


dataFile = sys.argv[1]
queryFile = sys.argv[2]
dataSize = sys.argv[3]
querySize = sys.argv[4]
dimensions = eval(sys.argv[5])
outputFolder = sys.argv[6]

outFileName = os.path.basename(os.path.normpath(dataFile))

# out = open('datasets/'+outFileName,'w')
out = open(outputFolder+outFileName,'w')
out.write(f'{dimensions} {dataSize} 0\n')
with open(dataFile) as f:
    for line in f:
        out.write(line)

out.close()

outFileName = os.path.basename(os.path.normpath(queryFile))
# out = open('datasets/'+outFileName,'w')
out = open(outputFolder+outFileName,'w')
out.write(f'{querySize}\n')
with open(queryFile) as f:
    for line in f:
        out.write("0 "+line);
        # out.write("0 ")
        # items = line.split()
        # for i in range(dimensions):
        #     # print('Using ',items[i],' ',items[i+dimensions])
        #     val = (eval(items[i])+eval(items[i+dimensions]))/2
        #     out.write(str(val)+" ")
        # out.write('\n')

out.close()

