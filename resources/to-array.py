import sys

(
	script,
	output_cpp_path,
	output_h_path,
	input_path,
	symbol_name,
) = sys.argv

with open(input_path, 'rb') as input_f:
	data = input_f.read()
data_size = len(data)
bytes_str = ', '.join([ str(ch) for ch in data ])

with open(output_cpp_path, 'w') as output_cpp_f:
	output_cpp_f.write(f'''
#include "{output_h_path}"
const unsigned char {symbol_name}[] = {{ {bytes_str} }};
const unsigned int {symbol_name}_size = {data_size};
''')

with open(output_h_path, 'w') as output_h_f:
	output_h_f.write(f'''
#pragma once
extern const unsigned char {symbol_name}[];
extern const unsigned int {symbol_name}_size;
''')
