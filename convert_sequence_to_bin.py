import csv
import os
import sys

input_filename = sys.argv[1]

script_dir = os.path.dirname(__file__)
output_filename = os.path.join(script_dir, 'receiver', 'data', 'sequence.bin')
output_bin_file = open(output_filename, 'wb')

def parse_line(line):
    current_values = []
    for output_str in line:
        output_val = int(output_str)
        current_values.append(output_val)
    return current_values

NUM_OUTPUTS = 90

file_size = 0
running_lengths = [1] * NUM_OUTPUTS
all_running_lengths_and_outputs = []
for i in range(NUM_OUTPUTS):
    all_running_lengths_and_outputs.append([])
previous_outputs = [None] * NUM_OUTPUTS

deltas_running_lengths_and_outputs = []
previous_outputs = [None] * NUM_OUTPUTS
running_lengths = [0] * NUM_OUTPUTS
original_sequence_file = open(input_filename, 'r')
original_sequence_csv = csv.reader(original_sequence_file)
delta_indices = [0] * NUM_OUTPUTS
for i, outputs in enumerate(original_sequence_csv):

    for output_nr, output in enumerate(outputs):
        
        if output == previous_outputs[output_nr] and running_lengths[output_nr] < 255:
            deltas_running_lengths_and_outputs[delta_indices[output_nr]][1] += 1
        else:
            deltas_running_lengths_and_outputs.append([output, 1])
            delta_indices[output_nr] = len(deltas_running_lengths_and_outputs) - 1
            running_lengths[output_nr] = 0
        running_lengths[output_nr] += 1
        previous_outputs[output_nr] = output

for delta in deltas_running_lengths_and_outputs:
    file_size += 2
    output_bin_file.write(bytes([int(delta[0]), int(delta[1])]))

output_bin_file.close()

if file_size > 4 * 1e6:
    print("File size exceeds 4MB. Not possible to store on the ESP32!")
    exit(1)

print(f"Theoretical file size: {file_size} bytes")
compression_ratio = 1 - file_size / (6241 * 90)
print(f"Compression ratio: {compression_ratio * 100:.2f}%")