#ifndef The_Powder_Toy_ContextMenu_h
#define The_Powder_Toy_ContextMenu_h

#include "Window.h"
#include "Appearance.h"
#include "Button.h"

namespace ui
{

class ContextMenuItem
{
public:
	int ID;
	std::string Text;
	bool Enabled;
	ContextMenuItem(std::string text, int id, bool enabled) : ID(id), Text(text), Enabled(enabled) {}
};

class ContextMenu: public ui::Window, public ButtonAction {
	std::vector<Button*> buttons;
	std::vector<ContextMenuItem> items;
	ui::Component * source;
public:
	ui::Appearance Appearance;
	class ItemSelectedAction;
	ContextMenu(Component * source);
	virtual void ActionCallbackItem(ui::Button *sender, int item);
	virtual void AddItem(ContextMenuItem item);
	virtual void RemoveItem(int id);
	virtual void SetItem(int id, std::string text);
	virtual void Show(ui::Point position);
	virtual void OnDraw();	
	virtual void OnMouseDown(int x, int y, unsigned button);
	virtual ~ContextMenu() {}
};
}

#endif
