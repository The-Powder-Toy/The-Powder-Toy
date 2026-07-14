#include "Menu.h"
#include "tool/Tool.h"

Menu::Menu(String::value_type icon_, String description_, int visible_, RGB colour_):
	icon(icon_),
	description(description_),
	tools(std::vector<Tool*>()),
	visible(visible_ ? true : false),
	colour(colour_)
{

}

Menu::~Menu()
{
	tools.clear();
}

void Menu::AddTool(Tool * tool_)
{
	auto it = std::ranges::upper_bound(tools, tool_, [](auto a, auto b){ return a->MenuSort < b->MenuSort; });
	tools.insert(it, tool_);
}

void Menu::ClearTools()
{
	tools.clear();
}
