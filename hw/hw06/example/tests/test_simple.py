# test_simple.py : Unit test for simple.py

import unittest

# If you run your unit test from the upper directory (one up), this import will work due
# to the presence of the __init__.py file in the tests directory
#
# $ python3 -m unittest discover -v

from simple import calcAdd

class TestSimple(unittest.TestCase):
    def test_sum(self):
            self.assertEqual(calcAdd(5,5), 10, 'The sum is wrong.')

if __name__ == '__main__':
    print('Starting the unit test')
    unittest.main(verbosity=2)

