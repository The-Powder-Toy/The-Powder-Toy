#include <iostream>
#include "graphics/Graphics.h"
#include "gui/Style.h"
#include "Button.h"
#include "DropDown.h"
#include "gui/interface/Window.h"

namespace ui {

class ItemSelectedAction;
class DropDownWindow: public ui::Window {
	friend class ItemSelectedAction;
	DropDown * dropDown;
	Appearance appearance;
	std::vector<Button> buttons;
	bool isMouseInside;
public:
	class ItemSelectedAction: public ButtonAction
	{
		DropDownWindow * window;
		std::string option;
	public:
		ItemSelectedAction(DropDownWindow * window, std::string option): window(window), option(option) { }
		virtual void ActionCallback(ui::Button *sender)
		{
			window->CloseActiveWindow();
			window->setOption(option);
			window->SelfDestruct();
		}
	};
	DropDownWindow(DropDown * dropDown):
		Window(ui::Point(dropDown->Position.X+dropDown->GetParentWindow()->Position.X-5, dropDown->Position.Y+dropDown->GetParentWindow()->Position.Y-3), ui::Point(dropDown->Size.X+10, 1+dropDown->options.size()*16)),
		dropDown(dropDown),
		appearance(dropDown->Appearance)
	{
		int currentY = 1;
		for (size_t i = 0; i < dropDown->options.size(); i++)
		{
			Button * tempButton = new Button(Point(1, currentY), Point(Size.X-2, 16), dropDown->options[i].first);
			tempButton->Appearance = appearance;
			if (i)
				tempButton->Appearance.Border = ui::Border(0, 1, 1, 1);
			tempButton->SetActionCallback(new ItemSelectedAction(this, dropDown->options[i].first));
			AddComponent(tempButton);
			currentY += 16;
		}
	}
	virtual void OnDraw()
	{
		Graphics * g = GetGraphics();
		g->clearrect(Position.X, Position.Y, Size.X, Size.Y);
	}
	void setOption(std::string option)
	{
		dropDown->SetOption(option);
		if (dropDown->callback)
		{
			size_t optionIndex = 0;
			for (optionIndex = 0; optionIndex < dropDown->options.size(); optionIndex++)
			{
				if(option == dropDown->options[optionIndex].first)
					break;
			}
			dropDown->callback->OptionChanged(dropDown, dropDown->options[optionIndex]);
		}
	}
	virtual void OnTryExit(ExitMethod method)
	{
		CloseActiveWindow();
		SelfDestruct();
	}
	virtual ~DropDownWindow() {}
};

DropDown::DropDown(Point position, Point size):
	Component(position, size),
	isMouseInside(false),
	optionIndex(-1),
	callback(NULL)
{
}

void DropDown::OnMouseClick(int x, int y, unsigned int button)
{
	DropDownWindow * newWindow = new DropDownWindow(this);
	newWindow->MakeActiveWindow();
}

void DropDown::Draw(const Point& screenPos)
{
	if(!drawn)
	{
		if(optionIndex!=-1)
			TextPosition(options[optionIndex].first);
		drawn = true;
	}
	Graphics * g = GetGraphics();
	Point Position = screenPos;

	ui::Colour textColour = Appearance.TextInactive;
	ui::Colour borderColour = Appearance.BorderInactive;
	ui::Colour backgroundColour = Appearance.BackgroundInactive;

	if (isMouseInside)
	{
		textColour = Appearance.TextHover;
		borderColour = Appearance.BorderHover;
		backgroundColour = Appearance.BackgroundHover;
	}
	else
	{
		textColour = Appearance.TextInactive;
		borderColour = Appearance.BorderInactive;
		backgroundColour = Appearance.BackgroundInactive;
	}

	g->fillrect(Position.X-1, Position.Y-1, Size.X+2, Size.Y+2, backgroundColour.Red, backgroundColour.Green, backgroundColour.Blue, backgroundColour.Alpha);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, borderColour.Red, borderColour.Green, borderColour.Blue, borderColour.Alpha);
	if(optionIndex!=-1)
		g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, options[optionIndex].first, textColour.Red, textColour.Green, textColour.Blue, textColour.Alpha);
}
	
void DropDown::OnMouseEnter(int x, int y)
{
	isMouseInside = true;
}

void DropDown::OnMouseLeave(int x, int y)
{
	isMouseInside = false;
}
	std::pair<std::string, int> DropDown::GetOption()
	{
		if(optionIndex!=-1)
		{
			return options[optionIndex];
		}
		return std::pair<std::string, int>("", -1);
	}
	
	void DropDown::SetOption(std::string option)
	{
		for (size_t i = 0; i < options.size(); i++)
		{
			if (options[i].first == option)
			{
				optionIndex = i;
				TextPosition(options[optionIndex].first);
				return;
			}
		}
	}
	void DropDown::SetOption(int option)
	{
		for (size_t i = 0; i < options.size(); i++)
		{
			if (options[i].second == option)
			{
				optionIndex = i;
				TextPosition(options[optionIndex].first);
				return;
			}
		}
	}
	void DropDown::AddOption(std::pair<std::string, int> option)
	{
		for (size_t i = 0; i < options.size(); i++)
		{
			if (options[i] == option)
				return;
		}
		options.push_back(option);
	}
	void DropDown::RemoveOption(std::string option)
	{
	start:
		for (size_t i = 0; i < options.size(); i++)
		{
			if (options[i].first == option)
			{
				if ((int)i == optionIndex)
					optionIndex = -1;
				options.erase(options.begin()+i);
				goto start;
			}
		}
	}
	void DropDown::SetOptions(std::vector<std::pair<std::string, int> > options)
	{
		this->options = options;
	}


DropDown::~DropDown() {
	delete callback;
}

} /* namespace ui */
