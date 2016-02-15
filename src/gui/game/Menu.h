#ifndef MENU_H_
#define MENU_H_

#include "Tool.h"

class Menu
{
	wchar_t icon;
	wstring description;
	vector<Tool*> tools;
public:
	Menu(wchar_t icon_, wstring description_):
		icon(icon_),
		description(description_),
		tools(vector<Tool*>())
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

	wchar_t GetWIcon()
	{
		return icon;
	}

	wstring GetDescription()
	{
		return description;
	}

	void AddTool(Tool * tool_)
	{
		tools.push_back(tool_);
	}
};


#endif /* MENU_H_ */
