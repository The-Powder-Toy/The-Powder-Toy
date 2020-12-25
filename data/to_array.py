import sys

build_dir = sys.argv[1]
output_cpp = sys.argv[2]
output_h = sys.argv[3]
input_any = sys.argv[4]
symbol_name = sys.argv[5]

with open(input_any, 'rb') as input_any_f:
	data = input_any_f.read()

with open(output_cpp, 'w') as output_cpp_f:
	output_cpp_f.write('#include "{0}"\nconst unsigned char {1}[] = {{ {2} }}; const unsigned int {1}_size = {3};\n'.format(output_h, symbol_name, ','.join([ str(b) for b in data ]), len(data)))

with open(output_h, 'w') as output_h_f:
	output_h_f.write('#pragma once\nextern const unsigned char {0}[]; extern const unsigned int {0}_size;\n'.format(symbol_name))
