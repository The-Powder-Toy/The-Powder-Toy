/*
 * Button.cpp
 *
 *  Created on: Jan 8, 2012
 *      Author: Simon
 */

#include <iostream>

#include "interface/Button.h"
#include "Graphics.h"

namespace ui {

Button::Button(int x, int y, int width, int height, const std::string& buttonText):
		Component(x, y, width, height),
		Toggleable(false),
		ButtonText(buttonText),
		isMouseInside(false),
		isButtonDown(false),
		state(false)
{

}

void Button::Draw(void* userdata)
{
	Graphics * g = reinterpret_cast<Graphics*>(userdata);
	//TODO: Cache text location, that way we don't have the text alignment code here
	if(isButtonDown)
	{
		g->fillrect(X, Y, Width, Height, 255, 255, 255, 255);
		g->drawtext(X+(Width-Graphics::textwidth((char *)ButtonText.c_str()))/2, Y+(Height-10)/2, ButtonText, 0, 0, 0, 255);
	}
	else
	{
		if(isMouseInside)
			g->fillrect(X, Y, Width, Height, 20, 20, 20, 255);
		g->drawrect(X, Y, Width, Height, 255, 255, 255, 255);
		g->drawtext(X+(Width-Graphics::textwidth((char *)ButtonText.c_str()))/2, Y+(Height-10)/2, ButtonText, 255, 255, 255, 255);
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
        if(state)
        {
            state = false;
        }
        else
        {
            if(Toggleable)
            {
                state = true;
            }
            DoAction();
        }
    }

    isButtonDown = false;
}

void Button::OnMouseUp(int x, int y, unsigned int button) //mouse unclick is called before this
{
    if(button != 1) return; //left click only!

    isButtonDown = false;
}

void Button::OnMouseClick(int x, int y, unsigned int button)
{
    if(button != 1) return; //left click only!

    isButtonDown = true;
}

void Button::OnMouseEnter(int x, int y, int dx, int dy)
{
    isMouseInside = true;
}

void Button::OnMouseLeave(int x, int y, int dx, int dy)
{
    isMouseInside = false;
}

void Button::DoAction()
{
    std::cout << "Do action!"<<std::endl;
}

} /* namespace ui */
