#include "DropDown.h"

#include "graphics/Graphics.h"
#include "gui/Style.h"
#include "Button.h"
#include "gui/interface/Window.h"

namespace ui {

class DropDownWindow : public ui::Window
{
	DropDown * dropDown;
	Appearance appearance;
	std::vector<Button> buttons;

public:
	DropDownWindow(DropDown * dropDown):
		Window(dropDown->GetScreenPos() + ui::Point(-1, -1 - dropDown->optionIndex * 16), ui::Point(dropDown->Size.X+2, 1+dropDown->options.size()*16)),
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
			auto option = dropDown->options[i].first;
			tempButton->SetActionCallback({ [this, option] {
				CloseActiveWindow();
				setOption(option);
				SelfDestruct();
			} });
			AddComponent(tempButton);
			currentY += 16;
		}
	}
	void OnDraw() override
	{
		Graphics * g = GetGraphics();
		g->clearrect(Position.X, Position.Y, Size.X, Size.Y);
	}
	void setOption(String option)
	{
		dropDown->SetOption(option);
		if (dropDown->actionCallback.change)
		{
			dropDown->actionCallback.change();
		}
	}
	void OnTryExit(ExitMethod method) override
	{
		CloseActiveWindow();
		SelfDestruct();
	}
	virtual ~DropDownWindow() {}
};

DropDown::DropDown(Point position, Point size):
	Component(position, size),
	isMouseInside(false),
	optionIndex(-1)
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

std::pair<String, int> DropDown::GetOption()
{
	if(optionIndex!=-1)
	{
		return options[optionIndex];
	}
	return std::pair<String, int>("", -1);
}

void DropDown::SetOption(String option)
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

void DropDown::AddOption(std::pair<String, int> option)
{
	for (size_t i = 0; i < options.size(); i++)
	{
		if (options[i] == option)
			return;
	}
	options.push_back(option);
}

void DropDown::RemoveOption(String option)
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

void DropDown::SetOptions(std::vector<std::pair<String, int> > options)
{
	this->options = options;
}

} /* namespace ui */
