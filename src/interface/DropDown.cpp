/*
 * DropDown.cpp
 *
 *  Created on: Apr 16, 2012
 *      Author: Simon
 */

#include <iostream>
#include "Button.h"
#include "DropDown.h"

namespace ui {

class ItemSelectedAction;
class DropDownWindow: public ui::Window {
	friend class ItemSelectedAction;
	Colour background, activeBackground;
	Colour border, activeBorder;
	Colour text, activeText;
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
		Window(ui::Point(dropDown->Position.X+dropDown->GetParentWindow()->Position.X-5, dropDown->Position.Y+dropDown->GetParentWindow()->Position.Y-3), ui::Point(dropDown->Size.X+10, dropDown->options.size()*13)),
		dropDown(dropDown),
		background(background),
		activeBackground(dropDown->activeBackground),
		border(dropDown->border),
		activeBorder(dropDown->activeBorder),
		text(dropDown->text),
		activeText(dropDown->activeText)
	{
		int currentY = 0;
		for(int i = 0; i < dropDown->options.size(); i++)
		{
			Button * tempButton = new Button(Point(0, currentY), Point(Size.X, 14), dropDown->options[i].first);
			tempButton->SetActionCallback(new ItemSelectedAction(this, dropDown->options[i].first));
			AddComponent(tempButton);
			currentY += 13;
		}
	}
	virtual void OnDraw()
	{
		Graphics * g = ui::Engine::Ref().g;
		g->fillrect(Position.X, Position.Y, Size.X, Size.Y, background.Red, background.Green, background.Blue, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, border.Red, border.Green, border.Blue, 255);
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
	background = activeBackground = Colour(0, 0, 0);
	activeText = text = activeBackground = border = activeBorder = Colour(255, 255, 255);
}

void DropDown::OnMouseClick(int x, int y, unsigned int button)
{
	DropDownWindow * newWindow = new DropDownWindow(this);
	ui::Engine().Ref().ShowWindow(newWindow);
}

void DropDown::Draw(const Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	Point Position = screenPos;
	if(isMouseInside)
	{
		g->fillrect(Position.X-1, Position.Y-1, Size.X+2, Size.Y+2, activeBackground.Red, activeBackground.Green, activeBackground.Blue, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, activeBorder.Red, activeBorder.Green, activeBorder.Blue, 255);
		if(optionIndex!=-1)
			g->drawtext(Position.X, Position.Y+1, options[optionIndex].first, activeText.Red, activeText.Green, activeText.Blue, 255);
		//g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y+1, displayText, activeText.Red, activeText.Green, activeText.Blue, 255);
	}
	else
	{
		g->fillrect(Position.X, Position.Y, Size.X, Size.Y, background.Red, background.Green, background.Blue, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, border.Red, border.Green, border.Blue, 255);
		if(optionIndex!=-1)
			g->drawtext(Position.X, Position.Y+1, options[optionIndex].first, text.Red, text.Green, text.Blue, 255);
		//g->drawtext(Position.X+textPosition.X, Position.Y+textPosition.Y+1, displayText, text.Red, text.Green, text.Blue, 255);
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
