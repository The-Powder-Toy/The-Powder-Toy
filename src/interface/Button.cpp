/*
 * Button.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#include <iostream>

#include "interface/Button.h"
#include "Graphics.h"
#include "Global.h"
#include "Engine.h"

namespace ui {

Button::Button(Window* parent_state, std::string buttonText):
	Component(parent_state),
	ButtonText(buttonText),
	isMouseInside(false),
	isButtonDown(false),
	isTogglable(false),
	actionCallback(NULL)
{

}

Button::Button(Point position, Point size, std::string buttonText):
	Component(position, size),
	ButtonText(buttonText),
	isMouseInside(false),
	isButtonDown(false),
	isTogglable(false),
	actionCallback(NULL)
{

}

Button::Button(std::string buttonText):
	Component(),
	ButtonText(buttonText),
	isMouseInside(false),
	isButtonDown(false),
	isTogglable(false),
	actionCallback(NULL)
{

}

void Button::SetTogglable(bool togglable)
{
	toggle = false;
	isTogglable = togglable;
}

bool Button::GetTogglable()
{
	return isTogglable;
}

inline bool Button::GetToggleState()
{
	return toggle;
}

inline void Button::SetToggleState(bool state)
{
	toggle = state;
}



void Button::Draw(const Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	Point Position = screenPos;
	// = reinterpret_cast<Graphics*>(userdata);
	//TODO: Cache text location, that way we don't have the text alignment code here
	if(isButtonDown || (isTogglable && toggle))
	{
		g->fillrect(Position.X-1, Position.Y-1, Size.X+2, Size.Y+2, 255, 255, 255, 255);
		g->drawtext(Position.X+(Size.X-Graphics::textwidth((char *)ButtonText.c_str()))/2, Position.Y+(Size.Y-10)/2, ButtonText, 0, 0, 0, 255);
	}
	else
	{
		if(isMouseInside)
			g->fillrect(Position.X, Position.Y, Size.X, Size.Y, 20, 20, 20, 255);
		g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
		g->drawtext(Position.X+(Size.X-Graphics::textwidth((char *)ButtonText.c_str()))/2, Position.Y+(Size.Y-10)/2, ButtonText, 255, 255, 255, 255);
	}
    /*sf::RenderWindow* rw = reinterpret_cast<sf::RenderWindow*>(userdata); //it better be a RenderWindow or so help your god

	//Draw component here
	sf::Text textGraphic(ButtonText);
	textGraphic.SetCharacterSize(11);
	if(isButtonDown)
		textGraphic.SetColor(sf::Color::Black);
	else
		textGraphic.SetColor(sf::Color::White);
	sf::FloatRect tempRect = textGraphic.GetRect();
    textGraphic.SetPosition(ceil(X + Width/2 - tempRect.Width/2), ceil(Y + Height/2 - tempRect.Height/2));

	if(isMouseInside)
	{
	    if(isButtonDown)
            rw->Draw(sf::Shape::Rectangle(X+2, Y+2, Width-4, Width-4, sf::Color::White, 2.f, sf::Color::Black));
        else
            rw->Draw(sf::Shape::Rectangle(X+2, Y+2, Width-4, Width-4, sf::Color::Black, 2.f, sf::Color::White));
	}
	else
	{
	    if(isButtonDown)
            rw->Draw(sf::Shape::Rectangle(X+2, Y+2, Width-4, Width-4, sf::Color::White, 2.f, sf::Color::Black));
        else
            rw->Draw(sf::Shape::Rectangle(X+1, Y+1, Width-2, Width-2, sf::Color::Black, 1.f, sf::Color::White));
	}

	rw->Draw(textGraphic);*/
}

void Button::OnMouseUnclick(int x, int y, unsigned int button)
{
    if(button != 1)
    {
        return; //left click only!
    }

    if(isButtonDown)
    {
		DoAction();
    }

    isButtonDown = false;
}

//void Button::OnMouseUp(int x, int y, unsigned int button) //mouse unclick is called before this
//{
 //   if(button != 1) return; //left click only!

//    isButtonDown = false;
//}

void Button::OnMouseClick(int x, int y, unsigned int button)
{
    if(button != 1) return; //left click only!
	if(isTogglable)
	{
		toggle = !toggle;
	}
    isButtonDown = true;
}

void Button::OnMouseEnter(int x, int y)
{
    isMouseInside = true;
}

void Button::OnMouseLeave(int x, int y)
{
    isMouseInside = false;
}

void Button::DoAction()
{
    std::cout << "Do action!"<<std::endl;
	//if(actionCallback)
	//	(*(actionCallback))();
	if(actionCallback)
		actionCallback->ActionCallback(this);
}

void Button::SetActionCallback(ButtonAction * action)
{
	actionCallback = action;
}

Button::~Button()
{
}

} /* namespace ui */
