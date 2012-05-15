/*
 * DropDown.cpp
 *
 *  Created on: Apr 16, 2012
 *      Author: Simon
 */

#include <iostream>
#include "Style.h"
#include "Button.h"
#include "DropDown.h"

namespace ui {

class ItemSelectedAction;
class DropDownWindow: public ui::Window {
	friend class ItemSelectedAction;
	Appearance appearance;
	DropDown * dropDown;
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
			ui::Engine::Ref().CloseWindow();
			window->setOption(option);
			window->SelfDestruct();
		}
	};
	DropDownWindow(DropDown * dropDown):
		Window(ui::Point(dropDown->Position.X+dropDown->GetParentWindow()->Position.X-5, dropDown->Position.Y+dropDown->GetParentWindow()->Position.Y-3), ui::Point(dropDown->Size.X+10, 1+dropDown->options.size()*15)),
		dropDown(dropDown),
		appearance(dropDown->Appearance)
	{
		int currentY = 1;
		for(int i = 0; i < dropDown->options.size(); i++)
		{
			Button * tempButton = new Button(Point(1, currentY), Point(Size.X-2, 14), dropDown->options[i].first);
			tempButton->Appearance = appearance;
			tempButton->SetActionCallback(new ItemSelectedAction(this, dropDown->options[i].first));
			AddComponent(tempButton);
			currentY += 15;
		}
	}
	virtual void OnDraw()
	{
		Graphics * g = ui::Engine::Ref().g;
		g->fillrect(Position.X, Position.Y, Size.X, Size.Y, 100, 100, 100, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, appearance.BackgroundInactive.Red, appearance.BackgroundInactive.Green, appearance.BackgroundInactive.Blue, appearance.BackgroundInactive.Alpha);
	}
	void setOption(std::string option)
	{
		dropDown->SetOption(option);
		if(dropDown->callback)
		{
			int optionIndex = 0;
			for(optionIndex = 0; optionIndex < dropDown->options.size(); optionIndex++)
			{
				if(option == dropDown->options[optionIndex].first)
					break;
			}
			dropDown->callback->OptionChanged(dropDown, dropDown->options[optionIndex]);
		}
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
	ui::Engine().Ref().ShowWindow(newWindow);
}

void DropDown::Draw(const Point& screenPos)
{
	if(!drawn)
	{
		if(optionIndex!=-1)
			TextPosition(options[optionIndex].first);
		drawn = true;
	}
	Graphics * g = ui::Engine::Ref().g;
	Point Position = screenPos;
	if(isMouseInside)
	{
		g->fillrect(Position.X-1, Position.Y-1, Size.X+2, Size.Y+2, Appearance.BackgroundActive.Red, Appearance.BackgroundActive.Green, Appearance.BackgroundActive.Blue, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, Appearance.BorderActive.Red, Appearance.BorderActive.Green, Appearance.BorderActive.Blue, 255);
		if(optionIndex!=-1)
			g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, options[optionIndex].first, Appearance.TextActive.Red, Appearance.TextActive.Green, Appearance.TextActive.Blue, 255);
	}
	else
	{
		g->fillrect(Position.X, Position.Y, Size.X, Size.Y, Appearance.BackgroundInactive.Red, Appearance.BackgroundInactive.Green, Appearance.BackgroundInactive.Blue, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, Appearance.BorderInactive.Red, Appearance.BorderInactive.Green, Appearance.BorderInactive.Blue, 255);
		if(optionIndex!=-1)
			g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y, options[optionIndex].first, Appearance.TextInactive.Red, Appearance.TextInactive.Green, Appearance.TextInactive.Blue, 255);
	}

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
		for(int i = 0; i < options.size(); i++)
		{
			if(options[i].first == option)
			{
				optionIndex = i;
				TextPosition(options[optionIndex].first);
				return;
			}
		}
	}
	void DropDown::SetOption(int option)
	{
		for(int i = 0; i < options.size(); i++)
		{
			if(options[i].second == option)
			{
				optionIndex = i;
				TextPosition(options[optionIndex].first);
				return;
			}
		}
	}
	void DropDown::AddOption(std::pair<std::string, int> option)
	{
		for(int i = 0; i < options.size(); i++)
		{
			if(options[i] == option)
				return;
		}
		options.push_back(option);
	}
	void DropDown::RemoveOption(std::string option)
	{
	start:
		for(int i = 0; i < options.size(); i++)
		{
			if(options[i].first == option)
			{
				if(i == optionIndex)
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
	if(callback)
		delete callback;
}

} /* namespace ui */
