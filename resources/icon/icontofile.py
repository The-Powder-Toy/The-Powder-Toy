import sys

with open(sys.argv[1], "rb") as icon:
	icondata = icon.read()
output = ["0x{0:02X}".format(ord(byte)) for byte in icondata]
for line in range(len(output)/16+1):
	print(", ".join(output[line*16:(line+1)*16])+",")
