import re, os, shutil, string, sys

def generateElements(elementFiles, outputCpp, outputH):

	elementClasses = dict()

	elementHeader = """#ifndef ELEMENTCLASSES_H
	#define ELEMENTCLASSES_H
	#include <vector>
	#include "simulation/Element.h"
	#include "simulation/elements/Element.h"
	"""

	directives = []

	for elementFile in elementFiles:
		f = open(elementFile, "r")
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

	f = open(outputH, "w")
	f.write(elementHeader)
	f.close()

	f = open(outputCpp, "w")
	f.write(elementContent)
	f.close()

def generateTools(toolFiles, outputCpp, outputH):
	toolClasses = dict()
	
	toolHeader = """#ifndef TOOLCLASSES_H
		#define TOOLCLASSES_H
		#include <vector>
		#include "simulation/Tools.h"
		#include "simulation/tools/SimTool.h"
		"""
	
	directives = []

	for toolFile in toolFiles:
		f = open(toolFile, "r")
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
	
	f = open(outputH, "w")
	f.write(toolHeader)
	f.close()
	
	f = open(outputCpp, "w")
	f.write(toolContent)
	f.close()

if(sys.argv[1] == "elements"):
	generateElements(sys.argv[4:], sys.argv[2], sys.argv[3])
elif(sys.argv[1] == "tools"):
	generateTools(sys.argv[4:], sys.argv[2], sys.argv[3])
