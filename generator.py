import re, os, shutil, string

if os.path.isdir("generated/"):
	shutil.rmtree("generated/")
os.mkdir("generated")

elementClasses = dict()

elementHeader = """#ifndef ELEMENTCLASSES_H
#define ELEMENTCLASSES_H
#include <vector>
#include "simulation/Element.h"
#include "simulation/elements/Element.h"
"""

directives = []

elementFiles = os.listdir("src/simulation/Elements")
for elementFile in elementFiles:
	f = open("src/simulation/Elements/"+elementFile, "r")
	fileData = f.read()
	f.close()

	directiveMatcher = '//#TPT-Directive\s+([^\r\n]+)'
	matcher = re.compile(directiveMatcher)
	directiveMatches = matcher.findall(fileData)

	for match in directiveMatches:
		directives.append(match.split(" "))

classDirectives = []
for d in directives:
	if d[0] == "ElementClass":
		elementClasses[d[1]] = []
		elementHeader += "#define %s %s\n" % (d[2], d[3])
		d[3] = string.atoi(d[3])
		classDirectives.append(d)
	
for d in directives:
	if d[0] == "ElementHeader":
		elementClasses[d[1]].append(string.join(d[2:], " ")+";")

for className, classMembers in elementClasses.items():
	elementHeader += """class {0}: public Element
{{
	public:
		{0}();
		virtual ~{0}();
		{1}
}};
""".format(className, string.join(classMembers, "\n"))

elementHeader += """std::vector<Element> GetElements();
#endif
"""

elementContent = """#include "ElementClasses.h"
std::vector<Element> GetElements()
{
	std::vector<Element> elements;
""";

elementIDs = sorted(classDirectives, key=lambda directive: directive[3])
for d in elementIDs:
	elementContent += """	elements.push_back(%s());
""" % (d[1])

elementContent += """	return elements;
}
""";

f = open("generated/ElementClasses.h", "w")
f.write(elementHeader)
f.close()

f = open("generated/ElementClasses.cpp", "w")
f.write(elementContent)
f.close()