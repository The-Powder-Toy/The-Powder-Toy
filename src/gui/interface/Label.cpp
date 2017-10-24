#include <string>
#include "Config.h"
#include "Format.h"
#include "Point.h"
#include "Label.h"
#include "Keys.h"
#include "Mouse.h"
#include "PowderToy.h"
#include "ContextMenu.h"
#include "graphics/Graphics.h"

using namespace ui;

Label::Label(Point position, Point size, std::string labelText):
	Component(position, size),
	text(labelText),
	textColour(255, 255, 255),
	selectionIndex0(-1),
	selectionIndex1(-1),
	selectionXL(-1),
	selectionXH(-1),
	multiline(false),
	selecting(false),
	autoHeight(size.Y==-1?true:false)
{
	menu = new ContextMenu(this);
	menu->AddItem(ContextMenuItem("Copy", 0, true));
}

Label::~Label()
{

}

void Label::SetMultiline(bool status)
{
	multiline = status;
	if(status)
	{
		updateMultiline();
		updateSelection();
		TextPosition(textLines);
	}
	else
	{
		TextPosition(text);
	}
}

void Label::SetText(std::string text)
{
	this->text = text;
	if(multiline)
	{
		updateMultiline();
		updateSelection();
		TextPosition(textLines);
	}
	else
	{
		TextPosition(text);
	}
}

void Label::AutoHeight()
{
	bool oldAH = autoHeight;
	autoHeight = true;
	updateMultiline();
	autoHeight = oldAH;
}

void Label::updateMultiline()
{
	int lines = 1;
	if (text.length()>0)
	{
		char * rawText = new char[text.length()+1];
		std::copy(text.begin(), text.end(), rawText);
		rawText[text.length()] = 0;

		char c, pc = 0;
		int charIndex = 0;

		int wordWidth = 0;
		int lineWidth = 0;
		char * wordStart = NULL;
		while ((c = rawText[charIndex++]))
		{
			switch(c)
			{
				case ' ':
					lineWidth += Graphics::CharWidth(c);
					lineWidth += wordWidth;
					wordWidth = 0;
					break;
				case '\n':
					lineWidth = wordWidth = 0;
					lines++;
					break;
				default:
					wordWidth += Graphics::CharWidth(c);
					break;
			}
			if (pc == ' ')
			{
				wordStart = &rawText[charIndex-2];
			}
			if ((c != ' ' || pc == ' ') && lineWidth + wordWidth >= Size.X-(Appearance.Margin.Left+Appearance.Margin.Right))
			{
				if (wordStart && *wordStart)
				{
					*wordStart = '\n';
					if (lineWidth != 0)
						lineWidth = wordWidth;
				}
				else if (!wordStart)
				{
					rawText[charIndex-1] = '\n';
					lineWidth = 0;
				}
				wordWidth = 0;
				wordStart = 0;
				lines++;
			}
			pc = c;
		}
		if (autoHeight)
		{
			Size.Y = lines*12+3;
		}
		textLines = std::string(rawText);
		delete[] rawText;
		/*int currentWidth = 0;
		char * lastSpace = NULL;
		char * currentWord = rawText;
		char * nextSpace;
		while(true)
		{
			nextSpace = strchr(currentWord+1, ' ');
			if(nextSpace)
				nextSpace[0] = 0;
			int width = Graphics::textwidth(currentWord);
			if(width+currentWidth >= Size.X-(Appearance.Margin.Left+Appearance.Margin.Right))
			{
				currentWidth = width;
				if(currentWord!=rawText)
				{
					currentWord[0] = '\n';
					lines++;
				}
			}
			else
				currentWidth += width;
			if(nextSpace)
				nextSpace[0] = ' ';
			if(!currentWord[0] || !currentWord[1] || !(currentWord = strchr(currentWord+1, ' ')))
				break;
		}
		if(autoHeight)
		{
			Size.Y = lines*12;
		}
		textLines = std::string(rawText);
		delete[] rawText;*/
	}
	else
	{
		if (autoHeight)
		{
			Size.Y = 15;
		}
		textLines = std::string("");
	}
}

std::string Label::GetText()
{
	return this->text;
}

void Label::OnContextMenuAction(int item)
{
	switch(item)
	{
	case 0:
		copySelection();
		break;
	}
}

void Label::OnMouseClick(int x, int y, unsigned button)
{
	if(button == SDL_BUTTON_RIGHT)
	{
		if(menu)
			menu->Show(GetScreenPos() + ui::Point(x, y));
	}
	else
	{
		selecting = true;
		if(multiline)
			selectionIndex0 = Graphics::CharIndexAtPosition((char*)textLines.c_str(), x-textPosition.X, y-textPosition.Y);
		else
			selectionIndex0 = Graphics::CharIndexAtPosition((char*)text.c_str(), x-textPosition.X, y-textPosition.Y);
		selectionIndex1 = selectionIndex0;

		updateSelection();
	}
}

void Label::copySelection()
{
	std::string currentText = text;
	std::string copyText;

	if (selectionIndex1 > selectionIndex0)
		copyText = currentText.substr(selectionIndex0, selectionIndex1-selectionIndex0).c_str();
	else if(selectionIndex0 > selectionIndex1)
		copyText = currentText.substr(selectionIndex1, selectionIndex0-selectionIndex1).c_str();
	else if (!currentText.length())
		return;
	else
		copyText = currentText.c_str();
	ClipboardPush(format::CleanString(copyText, false, true, false));
}

void Label::OnMouseUp(int x, int y, unsigned button)
{
	selecting = false;
}

void Label::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(ctrl && key == 'c')
	{
		copySelection();
	}
	if(ctrl && key == 'a')
	{
		selectAll();
		return;
	}
}

void Label::OnMouseMoved(int localx, int localy, int dx, int dy)
{
	if(selecting)
	{
		if(multiline)
			selectionIndex1 = Graphics::CharIndexAtPosition((char*)textLines.c_str(), localx-textPosition.X, localy-textPosition.Y);
		else
			selectionIndex1 = Graphics::CharIndexAtPosition((char*)text.c_str(), localx-textPosition.X, localy-textPosition.Y);
		updateSelection();
	}
}

void Label::Tick(float dt)
{
	if(!this->IsFocused() && (selecting || (selectionIndex0 != -1 && selectionIndex1 != -1)))
	{
		ClearSelection();
	}
}

int Label::getLowerSelectionBound()
{
	return (selectionIndex0 > selectionIndex1) ? selectionIndex1 : selectionIndex0;
}

int Label::getHigherSelectionBound()
{
	return (selectionIndex0 > selectionIndex1) ? selectionIndex0 : selectionIndex1;
}

bool Label::HasSelection()
{
	if(selectionIndex0 != -1 && selectionIndex1 != -1 && selectionIndex0 != selectionIndex1)
		return true;
	return false;
}

void Label::ClearSelection()
{
	selecting = false;
	selectionIndex0 = -1;
	selectionIndex1 = -1;
	updateSelection();
}

void Label::selectAll()
{
	selectionIndex0 = 0;
	selectionIndex1 = text.length();
	updateSelection();
}

void Label::updateSelection()
{
	std::string currentText;

	if (selectionIndex0 < 0) selectionIndex0 = 0;
	if (selectionIndex0 > (int)text.length()) selectionIndex0 = text.length();
	if (selectionIndex1 < 0) selectionIndex1 = 0;
	if (selectionIndex1 > (int)text.length()) selectionIndex1 = text.length();

	if(selectionIndex0 == -1 || selectionIndex1 == -1)
	{
		selectionXH = -1;
		selectionXL = -1;

		textFragments = std::string(currentText);
		return;
	}

	if(multiline)
		currentText = textLines;
	else
		currentText = text;

	if(selectionIndex1 > selectionIndex0) {
		selectionLineH = Graphics::PositionAtCharIndex((char*)currentText.c_str(), selectionIndex1, selectionXH, selectionYH);
		selectionLineL = Graphics::PositionAtCharIndex((char*)currentText.c_str(), selectionIndex0, selectionXL, selectionYL);

		textFragments = std::string(currentText);
		//textFragments.insert(selectionIndex1, "\x0E");
		//textFragments.insert(selectionIndex0, "\x0F\x01\x01\x01");
		textFragments.insert(selectionIndex1, "\x01");
		textFragments.insert(selectionIndex0, "\x01");
	} else if(selectionIndex0 > selectionIndex1) {
		selectionLineH = Graphics::PositionAtCharIndex((char*)currentText.c_str(), selectionIndex0, selectionXH, selectionYH);
		selectionLineL = Graphics::PositionAtCharIndex((char*)currentText.c_str(), selectionIndex1, selectionXL, selectionYL);

		textFragments = std::string(currentText);
		//textFragments.insert(selectionIndex0, "\x0E");
		//textFragments.insert(selectionIndex1, "\x0F\x01\x01\x01");
		textFragments.insert(selectionIndex0, "\x01");
		textFragments.insert(selectionIndex1, "\x01");
	} else {
		selectionXH = -1;
		selectionXL = -1;

		textFragments = std::string(currentText);
	}

	if(displayText.length())
	{
		displayText = tDisplayText;
		if(selectionIndex1 > selectionIndex0) {
			int tSelectionIndex1 = Graphics::CharIndexAtPosition((char*)displayText.c_str(), selectionXH, selectionYH);
			int tSelectionIndex0 = Graphics::CharIndexAtPosition((char*)displayText.c_str(), selectionXL, selectionYL);

			displayText.insert(tSelectionIndex1, "\x01");
			displayText.insert(tSelectionIndex0, "\x01");
		} else if(selectionIndex0 > selectionIndex1) {
			int tSelectionIndex0 = Graphics::CharIndexAtPosition((char*)displayText.c_str(), selectionXH, selectionYH);
			int tSelectionIndex1 = Graphics::CharIndexAtPosition((char*)displayText.c_str(), selectionXL, selectionYL);

			displayText.insert(tSelectionIndex0, "\x01");
			displayText.insert(tSelectionIndex1, "\x01");
		}
	}
}

void Label::SetDisplayText(std::string newText)
{
	ClearSelection();
	displayText = tDisplayText = newText;
}

void Label::Draw(const Point& screenPos)
{
	if(!drawn)
	{
		if(multiline)
		{
			TextPosition(textLines);
			updateMultiline();
			updateSelection();
		}
		else
			TextPosition(text);
		drawn = true;
	}
	Graphics * g = GetGraphics();

	std::string cDisplayText = displayText;

	if(!cDisplayText.length())
	{
		if(selectionXL != -1 && selectionXH != -1)
		{
			cDisplayText = textFragments;
		}
		else
		{
			if(multiline)
				cDisplayText = textLines;
			else
				cDisplayText = text;
		}
	}

	if(multiline)
	{
		if(selectionXL != -1 && selectionXH != -1)
		{
			if(selectionLineH - selectionLineL > 0)
			{
				g->fillrect(screenPos.X+textPosition.X+selectionXL, (screenPos.Y+textPosition.Y-1)+selectionYL, textSize.X-(selectionXL), 10, 255, 255, 255, 255);
				for(int i = 1; i < selectionLineH-selectionLineL; i++)
				{
					g->fillrect(screenPos.X+textPosition.X, (screenPos.Y+textPosition.Y-1)+selectionYL+(i*12), textSize.X, 10, 255, 255, 255, 255);
				}
				g->fillrect(screenPos.X+textPosition.X, (screenPos.Y+textPosition.Y-1)+selectionYH, selectionXH, 10, 255, 255, 255, 255);

			} else {
				g->fillrect(screenPos.X+textPosition.X+selectionXL, screenPos.Y+selectionYL+textPosition.Y-1, selectionXH-(selectionXL), 10, 255, 255, 255, 255);
			}
			g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, cDisplayText, textColour.Red, textColour.Green, textColour.Blue, 255);
		}
		else
		{
			g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, cDisplayText, textColour.Red, textColour.Green, textColour.Blue, 255);
		}
	} else {
		if(selectionXL != -1 && selectionXH != -1)
		{
			g->fillrect(screenPos.X+textPosition.X+selectionXL, screenPos.Y+textPosition.Y-1, selectionXH-(selectionXL), 10, 255, 255, 255, 255);
			g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, cDisplayText, textColour.Red, textColour.Green, textColour.Blue, 255);
		}
		else
		{
			g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, cDisplayText, textColour.Red, textColour.Green, textColour.Blue, 255);
		}
	}
}

