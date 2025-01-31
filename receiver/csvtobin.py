import csv
import struct

# Read csv file, where each cell is a uint8, and write it to a binary file
def csv_to_bin(csv_file, bin_file):
    with open(csv_file, 'r') as csvfile, open(bin_file, 'wb') as binfile:
        reader = csv.reader(csvfile)
        for row in reader:
            for cell in row:
                binfile.write(struct.pack('B', int(cell)))

csv_to_bin('input0.csv', 'output_decom.bin')