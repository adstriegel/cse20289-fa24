# grade-repos.py : Check to see if all repos are present and as guided by the
#  argument, determine if a homework is present and when it was last checked in,
#  and grade if requested for the respective repositories.
# 

import os
import sys
import argparse
import csv
import subprocess

# Import and create the list of students from the CSV file
def import_studentcsv (filename):
    # Import the CSV file and return a list of students
    students = []

    with open(filename, 'r') as f:
        reader = csv.DictReader(f)

        for row in reader:    
            TheStudent = {}
            TheStudent['Name'] = row['Name']
            TheStudent['NetID'] = row['NetID']
            students.append(TheStudent)


    return students

# Check for the presence of the respective repositories
def check_studentrepos (students, directory, prefix):
    # Check for the presence of the respective repositories
    for student in students:
        student['Repo'] = os.path.join(directory, prefix + student['NetID'])
        student['Exists'] = os.path.exists(student['Repo'])

    return



# Define the various arguments via argparse
parser = argparse.ArgumentParser(description='Repository confirmation')
parser.add_argument('list', type=str, help='The list of students to check (CSV file)')
parser.add_argument('--homework', type=str, help='The homework to check for', default='none')
parser.add_argument('--dir', type=str, help='The directory containing the directories to process', default='.')

parser.add_argument('--group', help='Operate in group mode', action='store_true')
parser.add_argument('--checkrepos', help='Check for the presence of the repositories', action='store_true')
parser.add_argument('--updaterepos', help='Update the repositories', action='store_true')

args = parser.parse_args()

TheStudents = import_studentcsv(args.list)

print('Students: ', str(len(TheStudents)))

if args.checkrepos:
    print('Checking repositories')
    check_studentrepos(TheStudents, args.dir, 'student-cse20289-fa24-')

    for student in TheStudents:
        if not student['Exists']:
            print('Student: ', student['Name'], ' NetID: ', student['NetID'], ' Repo: ', student['Repo'], ' Exists: ', student['Exists'])

    print('Done checking repositories')

if args.updaterepos:
    for student in TheStudents:
        if student['Exists']:
            print('Updating: ', student['Name'], ' NetID: ', student['NetID'], ' Repo: ', student['Repo'])
            subprocess.run(["git", "-C", student['Repo'], "pull"])            



if args.homework != 'none':
    for student in TheStudents:
        print('--------------------------')
        print('Checking homework: ', args.homework, ' for student: ', student['Name'], ' NetID: ', student['NetID'])

        if not student['Exists']:
            print('  Error: Student: ', student['Name'], ' NetID: ', student['NetID'], ' Repo: ', student['Repo'], ' Does not have a repository')
        else:

            # Check for the presence of the homework
            HomeworkRepo = os.path.join(student['Repo'], 'hw/hw' + args.homework)

            if os.path.exists(HomeworkRepo):
                pass
            else:
                print('Homework: ', args.homework, ' does not exist for student: ', student['Name'], ' NetID: ', student['NetID'])

            subprocess.run(["python3", "grading/grade-hw" + args.homework + '.py', args.dir, student['NetID']])

        print('--------------------------')
