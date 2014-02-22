#include "ContextMenu.h"

using namespace ui;

class ContextMenu::ItemSelectedAction: public ButtonAction
{
	ContextMenu * window;
	int item;
public:
	ItemSelectedAction(ContextMenu * window, int itemID): window(window), item(itemID) { }
	virtual void ActionCallback(ui::Button *sender)
	{
		window->ActionCallback(sender, item);
	}
};

ContextMenu::ContextMenu(Component * source):
		Window(ui::Point(0, 0), ui::Point(0, 0)),
		Appearance(source->Appearance),
		source(source)
{
}

void ContextMenu::Show(ui::Point position)
{
	for(int i = 0; i < buttons.size(); i++)
	{
		RemoveComponent(buttons[i]);
		delete buttons[i];
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
	for(int i = 0; i < items.size(); i++)
	{
		Button * tempButton = new Button(Point(1, currentY), Point(Size.X-2, 16), items[i].Text);
		tempButton->Appearance = Appearance;
		tempButton->Enabled = items[i].Enabled;
		tempButton->SetActionCallback(new ItemSelectedAction(this, items[i].ID));
		buttons.push_back(tempButton);
		AddComponent(tempButton);
		currentY += 15;
	}

	ui::Engine::Ref().ShowWindow(this);
}

void ContextMenu::ActionCallback(ui::Button *sender, int item)
{
	ui::Engine::Ref().CloseWindow();
	Halt();
	source->OnContextMenuAction(item);
}

void ContextMenu::OnMouseDown(int x, int y, unsigned button)
{
	if(!(x > Position.X && y > Position.Y && y < Position.Y+Size.Y && x < Position.X+Size.X)) //Clicked outside window
		ui::Engine::Ref().CloseWindow();
}

void ContextMenu::SetItem(int id, std::string text)
{
	for(int i = 0; i < items.size(); i++)
	{
		if(items[i].ID == id)
		{
			items[i].Text = text;
			break;
		}
	}
}

void ContextMenu::RemoveItem(int id)
{
	for(int i = 0; i < items.size(); i++)
	{
		if(items[i].ID == id)
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
	Graphics * g = ui::Engine::Ref().g;
	g->fillrect(Position.X, Position.Y, Size.X, Size.Y, 100, 100, 100, 255);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, Appearance.BackgroundInactive.Red, Appearance.BackgroundInactive.Green, Appearance.BackgroundInactive.Blue, Appearance.BackgroundInactive.Alpha);
}
