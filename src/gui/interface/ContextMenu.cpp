#include "ContextMenu.h"
#include "common/tpt-minmax.h"
#include "graphics/Graphics.h"

using namespace ui;

class ContextMenu::ItemSelectedAction: public ButtonAction
{
	ContextMenu * window;
	int item;
public:
	ItemSelectedAction(ContextMenu * window, int itemID): window(window), item(itemID) { }
	void ActionCallback(ui::Button *sender) override
	{
		window->ActionCallbackItem(sender, item);
	}
};

ContextMenu::ContextMenu(Component * source):
		Window(ui::Point(0, 0), ui::Point(0, 0)),
		source(source),
		Appearance(source->Appearance)
{
}

void ContextMenu::Show(ui::Point position)
{
	for (auto & button : buttons)
	{
		RemoveComponent(button);
		delete button;
	}
	buttons.clear();

	Size.X = 100;
	Size.Y = items.size()*16-1;

	if(position.X+Size.X > WINDOWW)
		position.X -= std::min(position.X, Size.X);
	if(position.Y+Size.Y > YRES+MENUSIZE)
		position.Y -= std::min(position.Y, Size.Y);
	Position = position;

	int currentY = 1;
	for (auto & item : items)
	{
		Button * tempButton = new Button(Point(1, currentY), Point(Size.X-2, 16), item.Text);
		tempButton->Appearance = Appearance;
		tempButton->Enabled = item.Enabled;
		tempButton->SetActionCallback(new ItemSelectedAction(this, item.ID));
		buttons.push_back(tempButton);
		AddComponent(tempButton);
		currentY += 15;
	}

	MakeActiveWindow();
}

void ContextMenu::ActionCallbackItem(ui::Button *sender, int item)
{
	CloseActiveWindow();
	Halt();
	source->OnContextMenuAction(item);
}

void ContextMenu::OnMouseDown(int x, int y, unsigned button)
{
	// Clicked outside window
	if (!(x > Position.X && y > Position.Y && y < Position.Y+Size.Y && x < Position.X+Size.X))
		CloseActiveWindow();
}

void ContextMenu::SetItem(int id, std::string text)
{
	for (auto & item : items)
	{
		if (item.ID == id)
		{
			item.Text = text;
			break;
		}
	}
}

void ContextMenu::RemoveItem(int id)
{
	for (size_t i = 0; i < items.size(); i++)
	{
		if (items[i].ID == id)
		{
			items.erase(items.begin()+i);
			break;
		}
	}
}

void ContextMenu::AddItem(ContextMenuItem item)
{
	items.push_back(item);
}

void ContextMenu::OnDraw()
{
	Graphics * g = GetGraphics();
	g->fillrect(Position.X, Position.Y, Size.X, Size.Y, 100, 100, 100, 255);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, Appearance.BackgroundInactive.Red, Appearance.BackgroundInactive.Green, Appearance.BackgroundInactive.Blue, Appearance.BackgroundInactive.Alpha);
}
