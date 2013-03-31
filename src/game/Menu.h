#ifndef MENU_H_
#define MENU_H_

#include "Tool.h"

class Menu
{
	char icon;
	string description;
	vector<Tool*> tools;
public:
	Menu(char icon_, string description_):
		icon(icon_),
		description(description_),
		tools(vector<Tool*>())
	{

	}

	virtual ~Menu()
	{
		for(int i = 0; i < tools.size(); i++)
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

	void AddTool(Tool * tool_)
	{
		tools.push_back(tool_);
	}
};


#endif /* MENU_H_ */
