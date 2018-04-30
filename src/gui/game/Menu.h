#ifndef MENU_H_
#define MENU_H_

#include "common/String.h"
#include "Tool.h"

class Menu
{
	String::value_type icon;
	String description;
	std::vector<Tool*> tools;
	bool visible;
public:
	Menu(String::value_type icon_, String description_, int visible_):
		icon(icon_),
		description(description_),
		tools(std::vector<Tool*>()),
		visible(visible_ ? true : false)
	{

	}

	virtual ~Menu()
	{
		for(unsigned int i = 0; i < tools.size(); i++)
		{
			delete tools[i];
		}
		tools.clear();
	}

	std::vector<Tool*> GetToolList()
	{
		return tools;
	}

	String::value_type GetIcon()
	{
		return icon;
	}

	String GetDescription()
	{
		return description;
	}

	bool GetVisible()
	{
		return visible;
	}

	void AddTool(Tool * tool_)
	{
		tools.push_back(tool_);
	}

	void ClearTools()
	{
		tools.clear();
	}
};


#endif /* MENU_H_ */
