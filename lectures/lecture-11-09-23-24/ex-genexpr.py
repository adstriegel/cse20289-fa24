# ex-genexpr.py : Example of using generator expressions

def genFunc():
    for n in range(4):
        yield n*2



# Create a list comprehension
listComp = [n*2 for n in range(4)]

# Create a generator expression
genExpr = (n*2 for n in range(4))

# What do we get when we print them?
print('listComp:', listComp)
print('genExpr:', genExpr)

print('genExpr:', list(genExpr))
print('')
# Pete and Repeat were in a boat, Pete fell out, who was left?
print('genExpr:', list(genExpr))

# Oh noes - maybe not

# Create a generator expression
genExpr = (n*2 for n in range(4))

for x in genExpr:
    print('x is', x)    

# Will this work?
try:
    # What about our generator function?
    print('genFunc:', list(genFunc))
except Exception as e:
    print('The error was ', e)

# Will this work?
try:
    # What about our generator function?
    print('genFunc:', list(genFunc()))
except Exception as e:
    print('The error was ', e)

