# generatethis.py

def doubleYield ():
    yield 10
    yield 15

def tripleYield ():
    yield 'A'
    yield 'B'
    yield 'C'

def yieldFrom ():
    print('Hello from yieldFrom')
    yield from doubleYield()
    yield from tripleYield()

if __name__ == '__main__':
    print('Starting the script')
    x = yieldFrom()
    print('x is ', x)
    theList = list(x)
    print(theList)
