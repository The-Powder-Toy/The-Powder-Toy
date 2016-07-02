#ifndef MENU_H_
#define MENU_H_

#include "Tool.h"

class Menu
{
	char icon;
	string description;
	vector<Tool*> tools;
	bool visible;
public:
	Menu(char icon_, string description_, int visible_):
		icon(icon_),
		description(description_),
		tools(vector<Tool*>()),
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

	vector<Tool*> GetToolList()
	{
		return tools;
	}

	char GetIcon()
	{
		return icon;
	}

	string GetDescription()
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
