/*
 * Tool.h
 *
 *  Created on: Jan 22, 2012
 *      Author: Simon
 */

#ifndef TOOL_H_
#define TOOL_H_

#include <iostream>

using namespace std;

class Tool
{
protected:
	int toolID, colRed, colBlue, colGreen;
	string toolName;
public:
	Tool(int id, string name, int r, int b, int g):
		toolID(id),
		toolName(name),
		colRed(r),
		colGreen(g),
		colBlue(b)
	{
	}
	virtual ~Tool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) {}
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {}
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {}
};

class ElementTool: public Tool
{
public:
	ElementTool(int id, string name, int r, int b, int g):
		Tool(id, name, r, g, b)
	{
	}
	virtual ~ElementTool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position){
		sim->create_parts(position.X, position.Y, 1, 1, toolID, 0, brush);
	}
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		std::cout << position1.X << toolID << brush << std::endl;
		sim->create_line(position1.X, position1.Y, position2.X, position2.Y, 1, 1, toolID, 0, brush);
	}
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {}
};

#endif /* TOOL_H_ */
