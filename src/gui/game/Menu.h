#pragma once
#include "common/String.h"
#include "graphics/Pixel.h"

class Tool;
class Menu
{
	String::value_type icon;
	String description;
	std::vector<Tool*> tools;
	bool visible;
	RGB colour;
public:
	Menu(String::value_type icon_, String description_, int visible_, RGB colour_ = 0xFFFFFF_rgb);
	virtual ~Menu();

	std::vector<Tool*> GetToolList()
	{
		return tools;
	}

	String::value_type GetIcon()
	{
		return icon;
	}

	RGB GetColour()
	{
		return colour;
	}

	String GetDescription()
	{
		return description;
	}

	bool GetVisible()
	{
		return visible;
	}

	void AddTool(Tool * tool_);
	void ClearTools();
};

