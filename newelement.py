#!/usr/bin/env python3
import sys
import re

if len(sys.argv) != 2:
	name = input('element name: ')
else:
	name = sys.argv[1]

if re.search('[^A-Z0-9-]', name):
	sys.exit('element names should only contain uppercase letters, digits and hyphens (you can change the Name property of the element to whatever later though, which is what shows up in menus)')

path = 'src/simulation/elements/' + name + '.cpp'

def get_elements():
	elements = dict()
	with open('src/simulation/ElementNumbers.h', 'r') as numbers:
		for nm, pt in re.findall('ELEMENT_DEFINE\\s*\\(\\s*(\\S+)\\s*,\\s*(\\d+)\\s*\\)', numbers.read()):
			elements[nm] = int(pt)
	return elements

elements = get_elements()
if name in elements:
	sys.exit('element already exists')
max_id = 0
for nm, pt in elements.items():
	pt_id = int(pt)
	if max_id < pt_id:
		max_id = pt_id
new_id = max_id + 1

with open(path, 'w') as elem:
	elem.write(r"""#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_{0}()
{{
	Identifier = "DEFAULT_PT_{0}";
	Name = "{0}";
	Colour = PIXPACK(0xFFFFFF);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	// element properties here

	Update = &update;
	Graphics = &graphics;
}}

static int update(UPDATE_FUNC_ARGS)
{{
	// update code here

	return 0;
}}

static int graphics(GRAPHICS_FUNC_ARGS)
{{
	// graphics code here
	// return 1 if nothing dynamic happens here

	return 0;
}}

""".format(name))
	elem.close()

print('element file \'{0}\' successfully created '.format(path))
input('now add \'ELEMENT_DEFINE({0}, {1});\' to \'src/simulation/ElementNumbers.h\', then press enter'.format(name, str(new_id)))
while True:
	elements = get_elements()
	if name in elements and elements[name] == new_id:
		break
	input('nope; try doing that again, then press enter')
