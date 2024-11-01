# simple.py : Simple code for unit testing

import argparse

def calcAdd (a, b):
    return a + b

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Simple code for unit testing')
    parser.add_argument('a', type=int, help='First number')
    parser.add_argument('b', type=int, help='Second number')
    parser.add_argument('--file', type=str, help='File to read')
    args = parser.parse_args()

    print(calcAdd(args.a, args.b))

    if args.file:
        with open(args.file, 'r') as f:
            print(f.read())
            