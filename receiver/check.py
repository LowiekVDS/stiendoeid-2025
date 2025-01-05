import csv

# Now compare the two files line by line and halt which line is different
input_file = open('output.csv', 'r')
original_sequence_file = open('input0.csv', 'r')
input_csv = csv.reader(input_file)
original_sequence_csv = csv.reader(original_sequence_file)

for i, (input_line, original_line) in enumerate(zip(input_csv, original_sequence_csv)):
    if input_line != original_line:
        print(f"Line {i+1} is different")

        # What is different?
        for j, (input_output, original_output) in enumerate(zip(input_line, original_line)):
            if input_output != original_output:
                print(f"Output {j+1} is different: {input_output} != {original_output}")
        break