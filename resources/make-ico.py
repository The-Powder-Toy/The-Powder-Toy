import struct
import sys

(
	script,
	output_ico,
	*input_pngs,
) = sys.argv

input_pngs_size = len(input_pngs)
assert(input_pngs_size <= 255)

ico_header = b''
ico_data = b''
data_offset = 6 + 16 * input_pngs_size
for input_png in input_pngs:
	with open(input_png, 'rb') as input_png_f:
		data = input_png_f.read()
	data_size = len(data)
	assert(data_size >= 0x21)
	magic, width, height, bit_depth, color_type = struct.unpack('>QxxxxxxxxLLBBxxxxxxx', data[0 : 0x21])
	assert(magic == 0x89504E470D0A1A0A)
	assert(width <= 256)
	assert(height <= 256)
	assert(bit_depth == 8)
	assert(color_type == 6)
	if width == 256:
		width = 0
	if height == 256:
		height = 0
	ico_header += struct.pack('<BBxxHHLL', width, height, 1, 32, data_size, data_offset)
	data_offset += data_size
	ico_data += data

with open(output_ico, 'wb') as output_ico_f:
	output_ico_f.write(struct.pack('<xxHH', 1, input_pngs_size))
	output_ico_f.write(ico_header)
	output_ico_f.write(ico_data)
