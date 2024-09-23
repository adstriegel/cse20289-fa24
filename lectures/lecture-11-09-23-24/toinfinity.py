# toinfinity.py : Example of using an infinite generator

def infinity_and_beyond():
    num = 0
    while True:
        yield num
        num += 1.0

if __name__ == '__main__':
    gen = infinity_and_beyond()
    for i in range(10):
        print('i is ', str(i), ' and the generator yields ', next(gen))
