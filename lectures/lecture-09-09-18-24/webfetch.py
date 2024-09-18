# webfetch.py

import requests
import re

# Fetch the main CSE website
result = requests.get('https://cse.nd.edu')

print(len(result.text), ' bytes received')
print('=====================')
print(result.text)

print('=====================')
theSearch = result.text.find('@nd.edu')
print('The string @nd.edu is found at position', theSearch)
print('')

print('=====================')
print('Finding just the @nd.edu part of the address')
theSearch = re.search(r'@nd\.edu', result.text)
print(theSearch)

print('=====================')
print('Finding a complete mail address')
theSearch = re.search(r'\w*(@nd\.edu)\b', result.text)
print(theSearch)

print('=====================')
print('Fetch a different page')
result = requests.get('https://cse.nd.edu/about-cse/administration-and-staff/')
theSearch = re.search(r'\w*(@nd\.edu)\b', result.text)
print(theSearch)

