# example.py

import os
import sys
import argparse
import csv

# Import and create the list of students from the CSV file
def import_studentcsv (filename):
    # Import the CSV file and return a list of students
    students = []
    with open(filename, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:    
            TheStudent = {}
            TheStudent['FirstName'] = row['FirstName']
            TheStudent['LastName'] = row['LastName']
            TheStudent['NetID'] = row['NetID']
            students.append(TheStudent)
    return students

TheList = import_studentcsv('StudentFile.csv')

# Are there any students with the same first name in the list?
for student in TheList:
    for otherstudent in TheList:
        if student['FirstName'] == otherstudent['FirstName'] and student['NetID'] != otherstudent['NetID']:
            print('Student: ', student['FirstName'], ' ', student['LastName'], ' NetID: ', student['NetID'], ' shares first name with Student: ', otherstudent['FirstName'], ' ', otherstudent['LastName'], ' NetID: ', otherstudent['NetID'])



# How could we identify if any two students shared the same first name?

# Extra: How could we identify the first name that occurs the most?
        
