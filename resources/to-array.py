import os
import sys

(
	script,
	output_cpp_path,
	output_h_path,
	output_dep_path,
	input_path,
	symbol_name,
) = sys.argv

script_path = os.path.realpath(__file__)

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

def dep_escape(s):
	t = ''
	for c in s:
		if c in [ ' ', '\\', ':', '$' ]:
			t += '\\'
		t += c
	return t

with open(output_dep_path, 'w') as output_dep_f:
	output_dep_f.write(f'''
{dep_escape(output_cpp_path)} {dep_escape(output_h_path)}: {dep_escape(input_path)} {dep_escape(script_path)}
''')
