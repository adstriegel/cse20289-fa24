# ex-args.py : Example showing the usage of argv

# Import the needed libraries
import sys
import os

# Display all arguments
index = 0
for argument in sys.argv:
    print('[', index, ']: ', argument)
    index += 1

# Which directory are we in?
print('Cur Dir: ' + os.curdir)
# What is the current directory?
print('CWD: ' + os.getcwd())
# Print the result of os.listdir
print('List Dir on .', os.listdir('.'))
