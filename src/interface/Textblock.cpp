/*
 * Textblock.cpp
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#include <iostream>
#include "Textblock.h"

using namespace ui;

Textblock::Textblock(Point position, Point size, std::string textboxText):
	Label(position, size, textboxText)
{
	if(size.Y==-1)
		autoHeight = true;
	else
		autoHeight = false;
	updateMultiline();
}

void Textblock::SetText(std::string text)
{
	this->text = text;
	updateMultiline();
}

void Textblock::updateMultiline()
{
	char * rawText = (char*)malloc(text.length()+1);
	memcpy(rawText, text.c_str(), text.length());
	rawText[text.length()] = 0;

	int lines = 1;
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
			lines++;
		}
		else
			currentWidth += width;
		if(nextSpace)
			nextSpace[0] = ' ';
		if(!(currentWord = strchr(currentWord+1, ' ')))
			break;
	}
	if(autoHeight)
	{
		Size.Y = lines*12;
	}
	textLines = rawText;
}

void Textblock::Draw(const Point &screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	//g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, textColour.Red, textColour.Green, textColour.Blue, 255);
	g->drawtext(screenPos.X+3, screenPos.Y+3, textLines, textColour.Red, textColour.Green, textColour.Blue, 255);
}

Textblock::~Textblock() {
	// TODO Auto-generated destructor stub
}

