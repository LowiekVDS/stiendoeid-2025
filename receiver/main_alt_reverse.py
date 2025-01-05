import csv

input_file = open('output.txt', 'r')

NUM_OUTPUTS = 90

current_value_to_output = [0] * NUM_OUTPUTS # Also fetchable from the size of the first line
current_length_to_output = [0] * NUM_OUTPUTS # Also fetchable from the size of the first line

output_file = open('output.csv', 'w')

line = ' '.join(input_file.readlines())
num_deltas = len(line.split(':')) - 1
current_delta = 0
current_step = 0
while current_delta < num_deltas:

    for output_nr in range(NUM_OUTPUTS):
        if current_length_to_output[output_nr] == current_step:
            
            delta =  line.split()[current_delta].split(':')
            current_value_to_output[output_nr], current_length_to_output[output_nr] = map(int, delta)
            current_length_to_output[output_nr] += current_step            
            current_delta += 1

        output_file.write(f'{current_value_to_output[output_nr]:03d}')        
        if output_nr != NUM_OUTPUTS - 1:
            output_file.write(',')
    
    current_step += 1

    output_file.write('\n')

output_file.close()

