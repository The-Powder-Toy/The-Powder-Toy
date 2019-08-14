#!/usr/bin/env python3
import sys
import re
import os

if len(sys.argv) != 2:
	name = input('element name: ')
else:
	name = sys.argv[1]

if re.search('[^A-Z0-9-]', name):
	sys.exit('element names should only contain uppercase letters, digits and hyphens (you can change the Name property of the element to whatever later though, which is what shows up in menus)')

path = 'src/simulation/elements/' + name + '.cpp'

if os.path.isfile(path):
	sys.exit('element already exists')

with open("generator.py") as f:
	exec(compile(f.read(), "generator.py", 'exec'))

max_id = 0
with open('generated/ElementClasses.h', 'r') as classes:
	for pt in re.findall('#define PT_\\S+ (\\d+)', classes.read()):
		pt_id = int(pt)
		if max_id < pt_id:
			max_id = pt_id

with open(path, 'w') as elem:
	elem.write(r"""#include "simulation/ElementCommon.h"

//#TPT-Directive ElementClass Element_{0} PT_{0} {1}
Element_{0}::Element_{0}()
{{
	Identifier = "DEFAULT_PT_{0}";
	Name = "{0}";
	Colour = PIXPACK(0xFFFFFF);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	// element properties here

	Update = &Element_{0}::update;
	Graphics = &Element_{0}::graphics;
}}

//#TPT-Directive ElementHeader Element_{0} static int update(UPDATE_FUNC_ARGS)
int Element_{0}::update(UPDATE_FUNC_ARGS)
{{
	// update code here

	return 0;
}}

//#TPT-Directive ElementHeader Element_{0} static int graphics(GRAPHICS_FUNC_ARGS)
int Element_{0}::graphics(GRAPHICS_FUNC_ARGS)
{{
	// graphics code here
	// return 1 if nothing dymanic happens here

	return 0;
}}

Element_{0}::~Element_{0}() {{}}
""".format(name, str(max_id + 1)))
	elem.close()

with open("generator.py") as f:
	exec(compile(f.read(), "generator.py", 'exec'))
