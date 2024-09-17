#include "Menu.h"

#include <utility>

Menu::Menu(String::value_type icon_, String description_, int visible_):
	icon(icon_),
	description(std::move(description_)),
	tools(std::vector<Tool*>()),
	visible(visible_ ? true : false)
{

}

Menu::~Menu()
{
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
