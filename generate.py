# Generate NUM_OUTPUTS zeroes to a csv

import csv
import os
import sys

script_dir = os.path.dirname(__file__)
output_filename = os.path.join(script_dir, 'sequence.csv')
output_csv_file = open(output_filename, 'w')

NUM_OUTPUTS = 900

for i in range(NUM_OUTPUTS):
    output_csv_file.write('0')
    if i < NUM_OUTPUTS - 1:
        output_csv_file.write(',')
output_csv_file.write('\n')

output_csv_file.close()