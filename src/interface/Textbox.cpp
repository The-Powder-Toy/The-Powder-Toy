#include <string>
#include <iostream>
#include <stdexcept>
#include "Config.h"
#include "Global.h"
#include "interface/Point.h"
#include "interface/Textbox.h"
#include "interface/Keys.h"

using namespace ui;

Textbox::Textbox(Point position, Point size, std::string textboxText):
	Component(position, size),
	text(textboxText),
	textPosition(ui::Point(0, 0)),
	textVAlign(AlignMiddle),
	textHAlign(AlignCentre),
	actionCallback(NULL),
	masked(false)
{
	SetText(textboxText);
	TextPosition();
	cursor = text.length();
}

Textbox::~Textbox()
{
	if(actionCallback)
		delete actionCallback;
}

void Textbox::TextPosition()
{
	std::string tempText = displayText;
	if(tempText.length() && cursor)
	{
		tempText.erase(cursor, tempText.length()-cursor);
		cursorPosition = Graphics::textwidth((char *)tempText.c_str());
	}
	else
	{
		cursorPosition = 0;
	}
	//Position.X+(Size.X-Graphics::textwidth((char *)ButtonText.c_str()))/2, Position.Y+(Size.Y-10)/2
	switch(textVAlign)
	{
	case AlignTop:
		textPosition.Y = 3;
		break;
	case AlignMiddle:
		textPosition.Y = (Size.Y-10)/2;
		break;
	case AlignBottom:
		textPosition.Y = Size.Y-11;
		break;
	}

	switch(textHAlign)
	{
	case AlignLeft:
		textPosition.X = 3;
		break;
	case AlignCentre:
		textPosition.X = (Size.X-Graphics::textwidth((char *)displayText.c_str()))/2;
		break;
	case AlignRight:
		textPosition.X = (Size.X-Graphics::textwidth((char *)displayText.c_str()))-2;
		break;
	}
}

void Textbox::SetText(std::string text)
{
	if(masked)
	{
		char tempText[text.length()];
		memset(tempText, 0x8d, text.length());
		tempText[text.length()] = 0;
		displayText = tempText;
	}
	else
	{
		displayText = text;
	}
	this->text = text;
	TextPosition();
}

std::string Textbox::GetText()
{
	return text;
}

void Textbox::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	bool changed = false;
	try
	{
		switch(key)
		{
		case KEY_HOME:
			cursor = 0;
			break;
		case KEY_END:
			cursor = text.length();
			break;
		case KEY_LEFT:
			if(cursor > 0)
				cursor--;
			break;
		case KEY_RIGHT:
			if(cursor < text.length())
				cursor++;
			break;
		case KEY_DELETE:
			if(text.length() && cursor < text.length())
			{
				if(ctrl)
					text.erase(cursor, text.length()-cursor);
				else
					text.erase(cursor, 1);
				changed = true;
			}
			break;
		case KEY_BACKSPACE:
			if(text.length() && cursor > 0)
			{
				if(ctrl)
				{
					text.erase(0, cursor);
					cursor = 0;
				}
				else
				{
					text.erase(cursor-1, 1);
					cursor--;
				}
				changed = true;
			}
			break;
		}
		if(character >= ' ' && character < 127)
		{
			if(cursor == text.length())
			{
				text += character;
				//std::cout << key << std::endl;
			}
			else
			{
				text.insert(cursor, 1, (char)character);
			}
			cursor++;
			changed = true;
		}
		if(changed && actionCallback)
		{
			actionCallback->TextChangedCallback(this);
		}
	}
	catch(std::out_of_range &e)
	{
		cursor = 0;
		text = "";
	}
	SetText(text);
	TextPosition();
}

void Textbox::Draw(const Point& screenPos)
{
	Graphics * g = Engine::Ref().g;
	if(IsFocused())
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);
		g->draw_line(screenPos.X+textPosition.X+cursorPosition, screenPos.Y+3, screenPos.X+textPosition.X+cursorPosition, screenPos.Y+12, 255, 255, 255, XRES+BARSIZE);
	}
	else
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 160, 160, 160, 255);
	}
	g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, displayText, 255, 255, 255, 255);
}
