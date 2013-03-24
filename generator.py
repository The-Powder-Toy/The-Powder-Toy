import re, os, shutil, string, sys

def generateElements(elementFiles, outputCpp, outputH):

	elementClasses = {}
	baseClasses = {}

	elementHeader = """#ifndef ELEMENTCLASSES_H
#define ELEMENTCLASSES_H

#include <vector>

#include "simulation/Element.h"
#include "simulation/elements/Element.h"

"""

	directives = []

	for elementFile in elementFiles:
		try:
			f = open(elementFile, "r")
		except:
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

	outputPath, outputFile = os.path.split(outputH)
	if not os.path.exists(outputPath):
		os.makedirs(outputPath)

	f = open(outputH, "w")
	f.write(elementHeader)
	f.close()

	f = open(outputCpp, "w")
	f.write(elementContent)
	f.close()

def generateTools(toolFiles, outputCpp, outputH):
	toolClasses = {}
	
	toolHeader = """#ifndef TOOLCLASSES_H
		#define TOOLCLASSES_H
		#include <vector>
		#include "simulation/Tools.h"
		#include "simulation/tools/SimTool.h"
		"""
	
	directives = []

	for toolFile in toolFiles:
		try:
			f = open(toolFile, "r")
		except:
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
	
	toolContent += """	return tools;
		}
		""";

	outputPath, outputFile = os.path.split(outputH)
	if not os.path.exists(outputPath):
		os.makedirs(outputPath)

	f = open(outputH, "w")
	f.write(toolHeader)
	f.close()
	
	f = open(outputCpp, "w")
	f.write(toolContent)
	f.close()

if(len(sys.argv) > 3):
    if(sys.argv[1] == "elements"):
    	generateElements(sys.argv[4:], sys.argv[2], sys.argv[3])
    elif(sys.argv[1] == "tools"):
    	generateTools(sys.argv[4:], sys.argv[2], sys.argv[3])
else:
	generateElements(os.listdir("src/simulation/elements"), "generated/ElementClasses.cpp", "generated/ElementClasses.h")
	generateTools(os.listdir("src/simulation/tools"), "generated/ToolClasses.cpp", "generated/ToolClasses.h")
