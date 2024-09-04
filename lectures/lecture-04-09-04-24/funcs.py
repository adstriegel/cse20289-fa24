# funcs.py : Example functions

def letsGetCrazy (Wild,Crazy=False):
    if Crazy:
        return Wild, Wild, 'Whoa'
    else:
        return Wild, 'Boring'


# Will this statement work?
letsGetCrazy('Hmm')

# What about this one?
x = letsGetCrazy('Maybe')
print(type(x))
print(x)

# And this one?
y = letsGetCrazy('Possibly', False)
print(type(y))
print(y)

# To infinity and beyond?
theDict = { 'Wild': 'Yes', 'Crazy': True, 3.14 : 'Pi' }
z = letsGetCrazy(theDict, True)
print(type(z))
print(z)
print(z[0])
print(z[0][3.14])

