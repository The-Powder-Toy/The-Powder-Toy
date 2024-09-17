#include "DropDown.h"

#include <utility>

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
		Window(dropDown->GetScreenPos() + ui::Point(-1, -1 - (dropDown->optionIndex*16 < dropDown->GetScreenPos().Y ? dropDown->optionIndex*16 : 0)),
						  ui::Point(dropDown->Size.X+2, 2+dropDown->options.size()*16)),
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
		g->DrawFilledRect(RectSized(Position, Size), 0x000000_rgb);
	}
	void setOption(const String& option)
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

	g->BlendFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), backgroundColour);
	g->BlendRect(RectSized(Position, Size), borderColour);
	if(optionIndex!=-1)
		g->BlendText(Position + textPosition, options[optionIndex].first, textColour);
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

void DropDown::SetOption(const String& option)
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

void DropDown::AddOption(const std::pair<String, int>& option)
{
	for (size_t i = 0; i < options.size(); i++)
	{
		if (options[i] == option)
			return;
	}
	options.push_back(option);
}

void DropDown::RemoveOption(const String& option)
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
	this->options = std::move(options);
}

} /* namespace ui */
