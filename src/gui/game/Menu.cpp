#include "Menu.h"

#include "Tool.h"

Menu::Menu(String::value_type icon_, String description_, int visible_):
	icon(icon_),
	description(description_),
	tools(std::vector<Tool*>()),
	visible(visible_ ? true : false)
{

}

Menu::~Menu()
{
	for(unsigned int i = 0; i < tools.size(); i++)
	{
		delete tools[i];
	}
	tools.clear();
}

void Menu::AddTool(Tool * tool_)
{
	tools.push_back(tool_);
}

void Menu::ClearTools()
{
	tools.clear();
}
