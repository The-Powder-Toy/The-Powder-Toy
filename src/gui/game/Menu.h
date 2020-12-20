#ifndef MENU_H_
#define MENU_H_
#include "Config.h"

#include "common/String.h"

class Tool;
class Menu
{
	String::value_type icon;
	String description;
	std::vector<Tool*> tools;
	bool visible;
public:
	Menu(String::value_type icon_, String description_, int visible_);
	virtual ~Menu();

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

	void AddTool(Tool * tool_);
	void ClearTools();
};


#endif /* MENU_H_ */
