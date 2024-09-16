# ex-yaml.py : Example code parsing a YAML file

import yaml

with open('file-ex.yaml') as f:
    data = yaml.safe_load(f)

    # Print out the data as loaded
    print(data)

    # Examples referencing the data
    print(data.keys())
    print(data['Game3'].keys())
    print(data['Game3']['Date'])

    # Print all keys and values
    for key, value in data['Game3'].items():
        print(key, value)

