#ifndef The_Powder_Toy_ContextMenu_h
#define The_Powder_Toy_ContextMenu_h

#include "Window.h"
#include "Appearance.h"
#include "Button.h"

namespace ui
{
	class Button;
	
class ContextMenuItem
{
public:
	int ID;
	String Text;
	bool Enabled;
	ContextMenuItem(String text, int id, bool enabled) : ID(id), Text(text), Enabled(enabled) {}
};

class ContextMenu: public ui::Window {
	std::vector<Button*> buttons;
	std::vector<ContextMenuItem> items;
	ui::Component * source;
public:
	ui::Appearance Appearance;
	ContextMenu(Component * source);
	virtual ~ContextMenu() = default;
	
	void ActionCallbackItem(ui::Button *sender, int item);
	void AddItem(ContextMenuItem item);
	void RemoveItem(int id);
	void SetItem(int id, String text);
	void Show(ui::Point position);
	void OnDraw() override;
	void OnMouseDown(int x, int y, unsigned button) override;
};
}

#endif
