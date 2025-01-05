import csv

original_sequence_file = open('input0.csv', 'r')
original_sequence_csv = csv.reader(original_sequence_file)
output_file = open('output.txt', 'w')

def parse_line(line):
    current_values = []
    for output_str in line:
        output_val = int(output_str)
        current_values.append(output_val)
    return current_values

file_size = 0

for output_nr, output_sequence in enumerate(zip(original_sequence_csv)):

    previous_output = output_sequence[0][0]
    running_length = 1

    for output in output_sequence[0]:
        if output == previous_output and running_length < 255:
            running_length += 1
        else:
            file_size += 2
            output_file.write(f'{previous_output}:{running_length} ')
            running_length = 1
        previous_output = output
    file_size += 3
    output_file.write(f'{previous_output}:{running_length} ')
    output_file.write('\n')

output_file.close()

print(f"Theoretical file size: {file_size} bytes")

compression_ratio = 1 - file_size / original_sequence_file.seek(0, 2)
print(f"Compression ratio: {compression_ratio * 100:.2f}%")