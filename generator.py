import re, os, shutil, string

def generateElements():
	elementClasses = dict()
	baseClasses = dict()

	elementHeader = """#ifndef ELEMENTCLASSES_H
#define ELEMENTCLASSES_H

#include <vector>

#include "simulation/Element.h"
#include "simulation/elements/Element.h"

"""

	directives = []

	elementFiles = os.listdir("src/simulation/elements")
	for elementFile in elementFiles:
		f = open("src/simulation/elements/"+elementFile, "r")
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
			d[3] = string.atoi(d[3])
			classDirectives.append(d)

	elementIDs = sorted(classDirectives, key=lambda directive: directive[3])

	for d in elementIDs:
		tmpClass = d[1]
		newClass = ""
		baseClass = "Element"
		if ':' in tmpClass:
			classBits = tmpClass.split(':')
			newClass = classBits[0]
			baseClass = classBits[1]
		else:
			newClass = tmpClass

		elementClasses[newClass] = []
		baseClasses[newClass] = baseClass
		elementHeader += "#define %s %s\n" % (d[2], d[3])

	for d in directives:
		if d[0] == "ElementHeader":
			tmpClass = d[1]
			newClass = ""
			baseClass = "Element"
			if ':' in tmpClass:
				classBits = tmpClass.split(':')
				newClass = classBits[0]
				baseClass = classBits[1]
			else:
				newClass = tmpClass
			elementClasses[newClass].append(string.join(d[2:], " ")+";")

	#for className, classMembers in elementClasses.items():
	for d in elementIDs:
		tmpClass = d[1]
		newClass = ""
		baseClass = "Element"
		if ':' in tmpClass:
			classBits = tmpClass.split(':')
			newClass = classBits[0]
			baseClass = classBits[1]
		else:
			newClass = tmpClass

		className = newClass
		classMembers = elementClasses[newClass]
		elementBase = baseClass
		elementHeader += """
class {0}: public {1}
{{
public:
	{0}();
	virtual ~{0}();
	{2}
}};
	""".format(className, elementBase, string.join(classMembers, "\n\t"))

	elementHeader += """
std::vector<Element> GetElements();

#endif
	"""

	elementContent = """#include "ElementClasses.h"

std::vector<Element> GetElements()
{
	std::vector<Element> elements;
	""";

	for d in elementIDs:
		tmpClass = d[1]
		newClass = ""
		baseClass = "Element"
		if ':' in tmpClass:
			classBits = tmpClass.split(':')
			newClass = classBits[0]
			baseClass = classBits[1]
		else:
			newClass = tmpClass
		elementContent += """elements.push_back(%s());
	""" % (newClass)

	elementContent += """return elements;
}
	""";

	f = open("generated/ElementClasses.h", "w")
	f.write(elementHeader)
	f.close()

	f = open("generated/ElementClasses.cpp", "w")
	f.write(elementContent)
	f.close()

def generateTools():
	toolClasses = dict()
	
	toolHeader = """#ifndef TOOLCLASSES_H
		#define TOOLCLASSES_H
		#include <vector>
		#include "simulation/Tools.h"
		#include "simulation/tools/SimTool.h"
		"""
	
	directives = []

	toolFiles = os.listdir("src/simulation/tools")
	for toolFile in toolFiles:
		f = open("src/simulation/tools/"+toolFile, "r")
		fileData = f.read()
		f.close()
		
		directiveMatcher = '//#TPT-Directive\s+([^\r\n]+)'
		matcher = re.compile(directiveMatcher)
		directiveMatches = matcher.findall(fileData)
		
		for match in directiveMatches:
			directives.append(match.split(" "))
	
	classDirectives = []
	for d in directives:
		if d[0] == "ToolClass":
			toolClasses[d[1]] = []
			toolHeader += "#define %s %s\n" % (d[2], d[3])
			d[3] = string.atoi(d[3])
			classDirectives.append(d)
	
	for d in directives:
		if d[0] == "ToolHeader":
			toolClasses[d[1]].append(string.join(d[2:], " ")+";")
	
	for className, classMembers in toolClasses.items():
		toolHeader += """class {0}: public SimTool
			{{
			public:
			{0}();
			virtual ~{0}();
			virtual int Perform(Simulation * sim, Particle * cpart, int x, int y, float strength);
			{1}
			}};
			""".format(className, string.join(classMembers, "\n"))
	
	toolHeader += """std::vector<SimTool*> GetTools();
		#endif
		"""
	
	toolContent = """#include "ToolClasses.h"
		std::vector<SimTool*> GetTools()
		{
		std::vector<SimTool*> tools;
		""";
	
	toolIDs = sorted(classDirectives, key=lambda directive: directive[3])
	for d in toolIDs:
		toolContent += """	tools.push_back(new %s());
			""" % (d[1])
	
	toolContent += """return tools;
		}
		""";
	
	f = open("generated/ToolClasses.h", "w")
	f.write(toolHeader)
	f.close()
	
	f = open("generated/ToolClasses.cpp", "w")
	f.write(toolContent)
	f.close()

generateElements()
generateTools()
