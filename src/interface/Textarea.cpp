/*
 * Textarea.cpp
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#include <iostream>
#include "Textarea.h"

using namespace ui;

Textarea::Textarea(Point position, Point size, std::string textboxText):
	Textbox(position, size, textboxText)
{
	updateMultiline();
}

void Textarea::SetText(std::string text)
{
	this->text = text;
	updateMultiline();
}

void Textarea::updateMultiline()
{
	char * rawText = (char*)malloc(text.length()+1);
	memcpy(rawText, text.c_str(), text.length());
	rawText[text.length()] = 0;

	int currentWidth = 0;
	char * lastSpace = NULL;
	char * currentWord = rawText;
	char * nextSpace;
	while(true)
	{
		nextSpace = strchr(currentWord+1, ' ');
		if(nextSpace)
			nextSpace[0] = 0;
		int width = Graphics::textwidth(currentWord);
		if(width+currentWidth > Size.X-6)
		{
			currentWidth = width;
			currentWord[0] = '\n';
		}
		else
			currentWidth += width;
		if(nextSpace)
			nextSpace[0] = ' ';
		if(!(currentWord = strchr(currentWord+1, ' ')))
			break;
	}
	textLines = rawText;
}

void Textarea::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	Textbox::OnKeyPress(key, character, shift, ctrl, alt);
	updateMultiline();
}

void Textarea::Draw(const Point &screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	if(IsFocused())
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);
		g->drawtext(screenPos.X+3, screenPos.Y+3, textLines, 255, 255, 255, 255);
	}
	else
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 160, 160, 160, 255);
		g->drawtext(screenPos.X+3, screenPos.Y+3, textLines, 160, 160, 160, 255);
	}
}

Textarea::~Textarea() {
	// TODO Auto-generated destructor stub
}

