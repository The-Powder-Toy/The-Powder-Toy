#include <string>
#include <iostream>
#include <stdexcept>
#include "Config.h"
#include "interface/Point.h"
#include "interface/Textbox.h"
#include "interface/Keys.h"

using namespace ui;

Textbox::Textbox(Point position, Point size, std::string textboxText):
	Component(position, size),
	text(textboxText),
	actionCallback(NULL),
	masked(false),
	border(true)
{
	SetText(textboxText);
	cursor = text.length();
}

Textbox::~Textbox()
{
	if(actionCallback)
		delete actionCallback;
}

void Textbox::TextPosition()
{
	if(cursor)
	{
		cursorPosition = Graphics::textnwidth((char *)displayText.c_str(), cursor);
	}
	else
	{
		cursorPosition = 0;
	}
	Component::TextPosition(displayText);
}

void Textbox::SetText(std::string text)
{
	cursor = text.length();
	this->text = text;
	this->displayText = text;
	TextPosition();
}


void Textbox::SetDisplayText(std::string text)
{
	displayText = text;
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
			}
			else
			{
				text.insert(cursor, 1, (char)character);
			}
			cursor++;
			changed = true;
		}
	}
	catch(std::out_of_range &e)
	{
		cursor = 0;
		text = "";
	}
	if(changed)
	{
		if(masked)
		{
			char * tempText = new char[text.length()+1];
			std::fill(tempText, tempText+text.length(), 0x8d);
			tempText[text.length()] = 0;
			displayText = std::string(tempText);
			delete tempText;
		}
		else
		{
			displayText = text;
		}
		if(actionCallback)
			actionCallback->TextChangedCallback(this);
	}
	TextPosition();
}

void Textbox::Draw(const Point& screenPos)
{
	if(!drawn)
	{
		TextPosition();
		drawn = true;
	}
	Graphics * g = Engine::Ref().g;
	if(IsFocused())
	{
		if(border) g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);
		g->draw_line(screenPos.X+textPosition.X+cursorPosition, screenPos.Y+3, screenPos.X+textPosition.X+cursorPosition, screenPos.Y+12, 255, 255, 255, XRES+BARSIZE);
	}
	else
	{
		if(border) g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 160, 160, 160, 255);
	}
	g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, displayText, 255, 255, 255, 255);
	if(Appearance.icon)
		g->draw_icon(screenPos.X+iconPosition.X, screenPos.Y+iconPosition.Y, Appearance.icon);
}
