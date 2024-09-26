# toinfinity.py : Example of using an infinite generator

import inspect

def normal_func ():
    return 42

def infinity_and_beyond():
    print('IAB: Entering the function')
    num = 0
    while True:
        print('IAB: Just before the yield')
        yield num
        num += 1.0
        print('IAB: Increase by one')

if __name__ == '__main__':
    print('** Starting the code **')

    print(' info - NF:  ', normal_func)
    print(' info - IAB: ', infinity_and_beyond)

    print(' type - NF:  ', type(normal_func))
    print(' type - IAB: ', type(infinity_and_beyond))

    print(' dir - NF:  ', dir(normal_func))
    print(' dir - IAB: ', dir(infinity_and_beyond))

    print(' Ask the inspect module if these are generator functions')
    print(' isGenerator - NF: ', inspect.isgeneratorfunction(normal_func))
    print(' isGenerator - IAB: ', inspect.isgeneratorfunction(infinity_and_beyond))

    print('** About to invoke the generator')
    gen = infinity_and_beyond()
    print(gen)

    print('Information about the generator')
    print(' Type: ', type(gen))
    print(' Dir: ', dir(gen))
    print('')

    print('== Looping through the generator ==')

    for i in range(10):
        print('- Loop: i is ', str(i))

        theNext = next(gen)
        print('  ', theNext)
        print('  ', gen)

        print('  i is ', str(i), ' and the generator yields ', theNext)
        print('')

