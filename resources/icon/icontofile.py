import sys
from PIL import Image
image = Image.open(sys.argv[1])

output = ""
formatted = []
for pixel in image.getdata():
	formatted.extend("0x{0:02X}".format(byte) for byte in pixel)
for i in range(len(formatted)/16 + 1):
	print(", ".join(formatted[i*16:(i+1)*16]) + ",")

"""with open(sys.argv[1], "rb") as icon:
	icondata = icon.read()
output = ["0x{0:02X}".format(ord(byte)) for byte in icondata]
for line in range(len(output)/16+1):
	print(", ".join(output[line*16:(line+1)*16])+",")"""
