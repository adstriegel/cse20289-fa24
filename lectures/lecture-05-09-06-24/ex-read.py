# ex-read.py : Read a file and output its contents to the screen

# Read the file in one massive string
theFile = open('PktQueue.cc', 'r')
content = theFile.read()
print(len(content) , "characters read from file")
print(content)
theFile.close()

# Read the file in as a list of lines
theFile = open('PktQueue.cc', 'r')
content = theFile.readlines()
print(len(content) , "lines read from file")
print(content)
theFile.close()

# Read the file in one line at a time
theFile = open('PktQueue.cc', 'r')
for theLine in theFile:
    print(theLine)
theFile.close()

