import bz2
import os
import sys

(
	script,
	output_cpp_path,
	output_h_path,
	output_dep_path,
	input_path,
	symbol_name,
	compression,
) = sys.argv

script_path = os.path.realpath(__file__)

with open(input_path, 'rb') as input_f:
	data = input_f.read()
if compression == 'bzip2':
	data = bz2.compress(data)
else:
	assert(compression == 'none')
data_size = len(data)
bytes_str = ', '.join([ str(ch) for ch in data ])

with open(output_cpp_path, 'w') as output_cpp_f:
	output_cpp_f.write(f'''
#include "{output_h_path}"

const struct {symbol_name}Resource {symbol_name} = {{{{{{ {bytes_str} }}}}}};
''')

with open(output_h_path, 'w') as output_h_f:
	output_h_f.write(f'''
#pragma once
#include "ResourceCommon.h"

extern const struct {symbol_name}Resource
{{
	std::array<unsigned char, {data_size}> data;
	static constexpr ResourceCompression compression = ResourceCompression::{compression};

	std::span<const char> AsCharSpan() const
	{{
		return std::span(reinterpret_cast<const char *>(data.data()), data.size());
	}}

	std::span<const unsigned char> AsUcharSpan() const
	{{
		return std::span(data.data(), data.size());
	}}

	std::string AsString() const
	{{
		auto s = AsCharSpan();
		if (compression == ResourceCompression::bzip2)
		{{
			std::vector<char> dest;
			assert(BZ2WDecompress(dest, s) == BZ2WDecompressOk);
			return std::string(dest.begin(), dest.end());
		}}
		return std::string(s.begin(), s.end());
	}}
}} {symbol_name};
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
