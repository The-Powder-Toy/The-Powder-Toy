#include <string>
#include <iostream>
#include <stdexcept>
#include <time.h>
#include "Config.h"
#include "gui/interface/Point.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Keys.h"
#include "ContextMenu.h"

using namespace ui;

Textbox::Textbox(Point position, Point size, std::string textboxText, std::string textboxPlaceholder):
	Label(position, size, ""),
	actionCallback(NULL),
	masked(false),
	border(true),
	mouseDown(false),
	limit(std::string::npos),
	inputType(All),
	keyDown(0),
	characterDown(0),
	ReadOnly(false)
{
	placeHolder = textboxPlaceholder;

	SetText(textboxText);
	cursor = text.length();

	menu->RemoveItem(0);
	menu->AddItem(ContextMenuItem("Cut", 1, true));
	menu->AddItem(ContextMenuItem("Copy", 0, true));
	menu->AddItem(ContextMenuItem("Paste", 2, true));
}

Textbox::~Textbox()
{
	if(actionCallback)
		delete actionCallback;
}

void Textbox::SetHidden(bool hidden)
{
	menu->RemoveItem(0);
	menu->RemoveItem(1);
	menu->RemoveItem(2);
	menu->AddItem(ContextMenuItem("Cut", 1, !hidden));
	menu->AddItem(ContextMenuItem("Copy", 0, !hidden));
	menu->AddItem(ContextMenuItem("Paste", 2, true));

	masked = hidden;
}

void Textbox::SetPlaceholder(std::string text)
{
	placeHolder = text;
}

void Textbox::SetText(std::string newText)
{
	backingText = newText;

	if(masked)
	{
		std::string maskedText = std::string(newText);
		std::fill(maskedText.begin(), maskedText.end(), '\x8D');
		Label::SetText(maskedText);
	}
	else
		Label::SetText(newText);

	cursor = newText.length();

	if(cursor)
	{
		Graphics::PositionAtCharIndex(multiline?((char*)textLines.c_str()):((char*)text.c_str()), cursor, cursorPositionX, cursorPositionY);
	}
	else
	{
		cursorPositionY = cursorPositionX = 0;
	}
}

Textbox::ValidInput Textbox::GetInputType()
{
	return inputType;
}

void Textbox::SetInputType(ValidInput input)
{
	inputType = input;
}

void Textbox::SetLimit(size_t limit)
{
	this->limit = limit;
}

size_t Textbox::GetLimit()
{
	return limit;
}

std::string Textbox::GetText()
{
	return backingText;
}

void Textbox::OnContextMenuAction(int item)
{
	switch(item)
	{
	case 0:
		copySelection();
		break;
	case 1:
		cutSelection();
		break;
	case 2:
		pasteIntoSelection();
		break;
	}
}

void Textbox::cutSelection()
{
	char * clipboardText;
	clipboardText = ClipboardPull();
	std::string newText = std::string(clipboardText);
	free(clipboardText);
	if(HasSelection())
	{
		if(getLowerSelectionBound() < 0 || getHigherSelectionBound() > backingText.length())
			return;
		ClipboardPush((char*)backingText.substr(getLowerSelectionBound(), getHigherSelectionBound()-getLowerSelectionBound()).c_str());
		backingText.erase(backingText.begin()+getLowerSelectionBound(), backingText.begin()+getHigherSelectionBound());
		cursor = getLowerSelectionBound(); 
	}
	else
	{
		ClipboardPush((char*)backingText.c_str());
		backingText.clear();
	}
	ClearSelection();

	if(masked)
	{
		std::string maskedText = std::string(backingText);
		std::fill(maskedText.begin(), maskedText.end(), '\x8D');
		Label::SetText(maskedText);
	}
	else
	{
		text = backingText;
	}
	if(actionCallback)
		actionCallback->TextChangedCallback(this);

	if(multiline)
		updateMultiline();
	updateSelection();
	TextPosition(text);

	if(cursor)
	{
		Graphics::PositionAtCharIndex(multiline?((char*)textLines.c_str()):((char*)text.c_str()), cursor, cursorPositionX, cursorPositionY);
	}
	else
	{
		cursorPositionY = cursorPositionX = 0;
	}
}

void Textbox::selectAll()
{
	selectionIndex0 = 0;
	selectionIndex1 = text.length();
	updateSelection();
}

void Textbox::pasteIntoSelection()
{
	char * clipboardText;
	clipboardText = ClipboardPull();
	std::string newText = std::string(clipboardText);
	free(clipboardText);
	if(HasSelection())
	{
		if(getLowerSelectionBound() < 0 || getHigherSelectionBound() > backingText.length())
			return;
		backingText.erase(backingText.begin()+getLowerSelectionBound(), backingText.begin()+getHigherSelectionBound());
		cursor = getLowerSelectionBound();
	}
	for(std::string::iterator iter = newText.begin(), end = newText.end(); iter != end; ++iter)
	{
		if(!CharacterValid(*iter))
		{
			if(inputType == All)
			{
				if(*iter == '\n' || *iter == '\r')
					*iter = ' ';
				else
					*iter = '?';
			}
			else
				*iter = '0';
		}
	}

	int regionWidth = Size.X;
	if(Appearance.icon)
		regionWidth -= 13;
	regionWidth -= Appearance.Margin.Left;
	regionWidth -= Appearance.Margin.Right;

	if(limit!=std::string::npos)
	{
		if(limit-backingText.length() >= 0)
			newText = newText.substr(0, limit-backingText.length());
		else
			newText = "";
	}
	else if(!multiline && Graphics::textwidth((char*)std::string(backingText+newText).c_str()) > regionWidth)
	{
		int pLimit = regionWidth - Graphics::textwidth((char*)backingText.c_str());
		int cIndex = Graphics::CharIndexAtPosition((char *)newText.c_str(), pLimit, 0);

		if(cIndex > 0)
			newText = newText.substr(0, cIndex);
		else
			newText = "";
	}

	backingText.insert(cursor, newText);
	cursor = cursor+newText.length();
	ClearSelection();

	if(masked)
	{
		std::string maskedText = std::string(backingText);
		std::fill(maskedText.begin(), maskedText.end(), '\x8D');
		Label::SetText(maskedText);
	}
	else
	{
		text = backingText;
	}
	if(actionCallback)
		actionCallback->TextChangedCallback(this);

	if(multiline)
		updateMultiline();
	updateSelection();
	if(multiline)
		TextPosition(textLines);
	else
		TextPosition(text);

	if(cursor)
	{
		Graphics::PositionAtCharIndex(multiline?((char*)textLines.c_str()):((char*)text.c_str()), cursor, cursorPositionX, cursorPositionY);
	}
	else
	{
		cursorPositionY = cursorPositionX = 0;
	}
}

bool Textbox::CharacterValid(Uint16 character)
{
	switch(inputType)
	{
		case Number:
		case Numeric:
			return (character >= '0' && character <= '9');
		case All:
		default:
			return (character >= ' ' && character < 127);
	}
	return false;
}

void Textbox::Tick(float dt)
{
	Label::Tick(dt);
	if(!IsFocused())
	{
		keyDown = 0;
		characterDown = 0;
	}
	if((keyDown || characterDown) && repeatTime <= clock())
	{
		OnVKeyPress(keyDown, characterDown, false, false, false);
		repeatTime = clock()+(0.03 * CLOCKS_PER_SEC);
	}
}

void Textbox::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	keyDown = 0;
	characterDown = 0;
}

void Textbox::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	characterDown = character;
	keyDown = key;
	repeatTime = clock()+(0.3 * CLOCKS_PER_SEC);
	OnVKeyPress(key, character, shift, ctrl, alt);
}

void Textbox::OnVKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	bool changed = false;
	if(ctrl && key == 'c' && !masked)
	{
		copySelection();
		return;
	}
	if(ctrl && key == 'v' && !ReadOnly)
	{
		pasteIntoSelection();
		return;
	}
	if(ctrl && key == 'x' && !masked && !ReadOnly)
	{
		cutSelection();
		return;
	}
	if(ctrl && key == 'a')
	{
		selectAll();
		return;
	}

	try
	{
		switch(key)
		{
		case KEY_HOME:
			cursor = 0;
			ClearSelection();
			break;
		case KEY_END:
			cursor = backingText.length();
			ClearSelection();
			break;
		case KEY_LEFT:
			if(cursor > 0)
				cursor--;
			ClearSelection();
			break;
		case KEY_RIGHT:
			if(cursor < backingText.length())
				cursor++;
			ClearSelection();
			break;
		case KEY_DELETE:
			if(ReadOnly)
				break;
			if(HasSelection())
			{
				if(getLowerSelectionBound() < 0 || getHigherSelectionBound() > backingText.length())
					return;
				backingText.erase(backingText.begin()+getLowerSelectionBound(), backingText.begin()+getHigherSelectionBound());
				cursor = getLowerSelectionBound();
				changed = true;
			}
			else if(backingText.length() && cursor < backingText.length())
			{
				if(ctrl)
					backingText.erase(cursor, backingText.length()-cursor);
				else
					backingText.erase(cursor, 1);
				changed = true;
			}
			ClearSelection();
			break;
		case KEY_BACKSPACE:
			if(ReadOnly)
				break;
			if(HasSelection())
			{
				if(getLowerSelectionBound() < 0 || getHigherSelectionBound() > backingText.length())
					return;
				backingText.erase(backingText.begin()+getLowerSelectionBound(), backingText.begin()+getHigherSelectionBound());
				cursor = getLowerSelectionBound();
				changed = true;
			}
			else if(backingText.length() && cursor > 0)
			{
				if(ctrl)
				{
					backingText.erase(0, cursor);
					cursor = 0;
				}
				else
				{
					backingText.erase(cursor-1, 1);
					cursor--;
				}
				changed = true;
			}
			ClearSelection();
			break;
		}
		if(CharacterValid(character) && !ReadOnly)
		{
			if(HasSelection())
			{
				if(getLowerSelectionBound() < 0 || getHigherSelectionBound() > backingText.length())
					return;
				backingText.erase(backingText.begin()+getLowerSelectionBound(), backingText.begin()+getHigherSelectionBound());
				cursor = getLowerSelectionBound();
			}

			int regionWidth = Size.X;
			if(Appearance.icon)
				regionWidth -= 13;
			regionWidth -= Appearance.Margin.Left;
			regionWidth -= Appearance.Margin.Right;
			if((limit==std::string::npos || backingText.length() < limit) && (Graphics::textwidth((char*)std::string(backingText+char(character)).c_str()) <= regionWidth || multiline || limit!=std::string::npos))
			{
				if(cursor == backingText.length())
				{
					backingText += character;
				}
				else
				{
					backingText.insert(cursor, 1, (char)character);
				}
				cursor++;
			}
			changed = true;
			ClearSelection();
		}
	}
	catch(std::out_of_range &e)
	{
		cursor = 0;
		backingText = "";
	}
	if(inputType == Number)
	{
		//Remove extra preceding 0's
		while(backingText[0] == '0' && backingText.length()>1)
			backingText.erase(backingText.begin());
	}
	if(cursor > backingText.length())
		cursor = backingText.length();
	if(changed)
	{
		if(masked)
		{
			std::string maskedText = std::string(backingText);
			std::fill(maskedText.begin(), maskedText.end(), '\x8D');
			Label::SetText(maskedText);
		}
		else
		{
			text = backingText;
		}
		if(actionCallback)
			actionCallback->TextChangedCallback(this);
	}

	if(multiline)
		updateMultiline();
	updateSelection();
	if(multiline)
		TextPosition(textLines);
	else
		TextPosition(text);

	if(cursor)
	{
		Graphics::PositionAtCharIndex(multiline?((char*)textLines.c_str()):((char*)text.c_str()), cursor, cursorPositionX, cursorPositionY);
	}
	else
	{
		cursorPositionY = cursorPositionX = 0;
	}
}

void Textbox::OnMouseClick(int x, int y, unsigned button)
{

	if(button != BUTTON_RIGHT)
	{
		mouseDown = true;
		cursor = Graphics::CharIndexAtPosition(multiline?((char*)textLines.c_str()):((char*)text.c_str()), x-textPosition.X, y-textPosition.Y);
		if(cursor)
		{
			Graphics::PositionAtCharIndex(multiline?((char*)textLines.c_str()):((char*)text.c_str()), cursor, cursorPositionX, cursorPositionY);
		}
		else
		{
			cursorPositionY = cursorPositionX = 0;
		}
	}
	Label::OnMouseClick(x, y, button);
}

void Textbox::OnMouseUp(int x, int y, unsigned button)
{
	mouseDown = false;
	Label::OnMouseUp(x, y, button);
}

void Textbox::OnMouseMoved(int localx, int localy, int dx, int dy)
{
	if(mouseDown)
	{
		cursor = Graphics::CharIndexAtPosition(multiline?((char*)textLines.c_str()):((char*)text.c_str()), localx-textPosition.X, localy-textPosition.Y);
		if(cursor)
		{
			Graphics::PositionAtCharIndex(multiline?((char*)textLines.c_str()):((char*)text.c_str()), cursor, cursorPositionX, cursorPositionY);
		}
		else
		{
			cursorPositionY = cursorPositionX = 0;
		}
	}
	Label::OnMouseMoved(localx, localy, dx, dy);
}

void Textbox::Draw(const Point& screenPos)
{
	Label::Draw(screenPos);

	Graphics * g = Engine::Ref().g;
	if(IsFocused())
	{
		if(border) g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);
		g->draw_line(screenPos.X+textPosition.X+cursorPositionX, screenPos.Y-2+textPosition.Y+cursorPositionY, screenPos.X+textPosition.X+cursorPositionX, screenPos.Y+9+textPosition.Y+cursorPositionY, 255, 255, 255, 255);
	}
	else
	{
		if(!text.length())
		{
			g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, placeHolder, textColour.Red, textColour.Green, textColour.Blue, 170);
		}
		if(border) g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 160, 160, 160, 255);
	}
	if(Appearance.icon)
		g->draw_icon(screenPos.X+iconPosition.X, screenPos.Y+iconPosition.Y, Appearance.icon);
}

/*
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
}*/
