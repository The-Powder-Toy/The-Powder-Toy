#include "common/String.h"
#include <iostream>
#include <stdexcept>
#include "Config.h"
#include "Platform.h"
#include "Format.h"
#include "graphics/Graphics.h"
#include "gui/interface/Point.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Keys.h"
#include "gui/interface/Mouse.h"
#include "ContextMenu.h"

using namespace ui;

Textbox::Textbox(Point position, Point size, String textboxText, String textboxPlaceholder):
	Label(position, size, ""),
	ReadOnly(false),
	inputType(All),
	limit(String::npos),
	keyDown(0),
	characterDown(0),
	mouseDown(false),
	masked(false),
	border(true),
	actionCallback(NULL)
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

void Textbox::SetPlaceholder(String text)
{
	placeHolder = text;
}

void Textbox::SetText(String newText)
{
	backingText = newText;

	if(masked)
	{
		String maskedText = newText;
		std::fill(maskedText.begin(), maskedText.end(), 0xE00D);
		Label::SetText(maskedText);
	}
	else
		Label::SetText(newText);

	cursor = newText.length();

	if(cursor)
	{
		Graphics::PositionAtCharIndex(multiline?textLines:text, cursor, cursorPositionX, cursorPositionY);
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

String Textbox::GetText()
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

void Textbox::resetCursorPosition()
{
	Graphics::PositionAtCharIndex(multiline?textLines:text, cursor, cursorPositionX, cursorPositionY);
}

void Textbox::TabFocus()
{
	GetParentWindow()->FocusComponent(this);
	selectAll();
}

void Textbox::cutSelection()
{
	if (HasSelection())
	{
		if (getLowerSelectionBound() < 0 || getHigherSelectionBound() > (int)backingText.length())
			return;
		String toCopy = backingText.Between(getLowerSelectionBound(), getHigherSelectionBound());
		ClipboardPush(format::CleanString(toCopy, false, true, false).ToUtf8());
		backingText.erase(backingText.begin()+getLowerSelectionBound(), backingText.begin()+getHigherSelectionBound());
		cursor = getLowerSelectionBound();
	}
	else
	{
		if (!backingText.length())
			return;
		ClipboardPush(format::CleanString(backingText, false, true, false).ToUtf8());
		backingText.clear();
		cursor = 0;
	}
	ClearSelection();

	if(masked)
	{
		String maskedText = backingText;
		std::fill(maskedText.begin(), maskedText.end(), 0xE00D);
		Label::SetText(maskedText);
	}
	else
	{
		text = backingText;
	}

	if(multiline)
		updateMultiline();
	updateSelection();
	TextPosition(text);

	if(cursor)
	{
		Graphics::PositionAtCharIndex(multiline?textLines:text, cursor, cursorPositionX, cursorPositionY);
	}
	else
	{
		cursorPositionY = cursorPositionX = 0;
	}
	if(actionCallback)
		actionCallback->TextChangedCallback(this);
}

void Textbox::pasteIntoSelection()
{
	String newText = format::CleanString(ClipboardPull().FromUtf8(), true, true, inputType != Multiline, inputType == Number || inputType == Numeric);
	if (HasSelection())
	{
		if (getLowerSelectionBound() < 0 || getHigherSelectionBound() > (int)backingText.length())
			return;
		backingText.EraseBetween(getLowerSelectionBound(), getHigherSelectionBound());
		cursor = getLowerSelectionBound();
	}

	int regionWidth = Size.X;
	if (Appearance.icon)
		regionWidth -= 13;
	regionWidth -= Appearance.Margin.Left;
	regionWidth -= Appearance.Margin.Right;

	if (limit != String::npos)
	{
		newText = newText.Substr(0, limit-backingText.length());
	}
	if (!multiline && Graphics::textwidth(backingText + newText) > regionWidth)
	{
		int pLimit = regionWidth - Graphics::textwidth(backingText);
		int cIndex = Graphics::CharIndexAtPosition(newText, pLimit, 0);

		if (cIndex > 0)
			newText = newText.Substr(0, cIndex);
		else
			newText = "";
	}

	backingText.Insert(cursor, newText);
	cursor = cursor+newText.length();
	ClearSelection();

	if(masked)
	{
		String maskedText = backingText;
		std::fill(maskedText.begin(), maskedText.end(), 0xE00D);
		Label::SetText(maskedText);
	}
	else
	{
		text = backingText;
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
		Graphics::PositionAtCharIndex(multiline?textLines:text, cursor, cursorPositionX, cursorPositionY);
	}
	else
	{
		cursorPositionY = cursorPositionX = 0;
	}
	if(actionCallback)
		actionCallback->TextChangedCallback(this);
}

bool Textbox::CharacterValid(Uint16 character)
{
	switch(inputType)
	{
		case Numeric:
			if (character == '-' && cursor == 0 && backingText[0] != '-')
				return true;
		case Number:
			return (character >= '0' && character <= '9');
		case Multiline:
			if (character == '\n')
				return true;
		case All:
		default:
			return (character >= ' ' && character < 127);
	}
	return false;
}

void Textbox::Tick(float dt)
{
	Label::Tick(dt);
	if (!IsFocused())
	{
		keyDown = 0;
		characterDown = 0;
	}
	unsigned long time_pls = Platform::GetTime();
	if ((keyDown || characterDown) && repeatTime <= time_pls)
	{
		OnVKeyPress(keyDown, characterDown, false, false, false);
		repeatTime = Platform::GetTime()+30;
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
	repeatTime = Platform::GetTime()+300;
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
		case SDLK_HOME:
			cursor = 0;
			ClearSelection();
			break;
		case SDLK_END:
			cursor = backingText.length();
			ClearSelection();
			break;
		case SDLK_LEFT:
			if(cursor > 0)
				cursor--;
			ClearSelection();
			break;
		case SDLK_RIGHT:
			if (cursor < (int)backingText.length())
				cursor++;
			ClearSelection();
			break;
		case SDLK_DELETE:
			if(ReadOnly)
				break;
			if (HasSelection())
			{
				if (getLowerSelectionBound() < 0 || getHigherSelectionBound() > (int)backingText.length())
					return;
				backingText.Erase(getLowerSelectionBound(), getHigherSelectionBound());
				cursor = getLowerSelectionBound();
				changed = true;
			}
			else if (backingText.length() && cursor < (int)backingText.length())
			{
				if (ctrl)
				{
					size_t stopChar;
					stopChar = backingText.SplitByNot(" .,!?\n", cursor).PositionBefore();
					stopChar = backingText.SplitByAny(" .,!?\n", stopChar).PositionBefore();
					backingText.EraseBetween(cursor, stopChar);
				}
				else
					backingText.erase(cursor, 1);
				changed = true;
			}
			ClearSelection();
			break;
		case SDLK_BACKSPACE:
			if (ReadOnly)
				break;
			if (HasSelection())
			{
				if (getLowerSelectionBound() < 0 || getHigherSelectionBound() > (int)backingText.length())
					return;
				backingText.erase(backingText.begin()+getLowerSelectionBound(), backingText.begin()+getHigherSelectionBound());
				cursor = getLowerSelectionBound();
				changed = true;
			}
			else if (backingText.length() && cursor > 0)
			{
				if (ctrl)
				{
					size_t stopChar;
					stopChar = backingText.SplitFromEndByNot(" .,!?\n", cursor).PositionBefore();
					if (stopChar == backingText.npos)
						stopChar = -1;
					else
						stopChar = backingText.SplitFromEndByAny(" .,!?\n", stopChar).PositionBefore();
					backingText.EraseBetween(stopChar+1, cursor);
					cursor = stopChar+1;
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
		case SDLK_RETURN:
			character = '\n';
		default:
			if (CharacterValid(character) && !ReadOnly)
			{
				if (HasSelection())
				{
					if (getLowerSelectionBound() < 0 || getHigherSelectionBound() > (int)backingText.length())
						return;
					backingText.erase(backingText.begin()+getLowerSelectionBound(), backingText.begin()+getHigherSelectionBound());
					cursor = getLowerSelectionBound();
				}

				int regionWidth = Size.X;
				if (Appearance.icon)
					regionWidth -= 13;
				regionWidth -= Appearance.Margin.Left;
				regionWidth -= Appearance.Margin.Right;
				if ((limit==String::npos || backingText.length() < limit) && (Graphics::textwidth(backingText + character) <= regionWidth || multiline))
				{
					if (cursor == (int)backingText.length())
					{
						backingText += character;
					}
					else
					{
						backingText.Insert(cursor, character);
					}
					cursor++;
				}
				changed = true;
				ClearSelection();
			}
			break;
		}
	}
	catch (std::out_of_range &e)
	{
		cursor = 0;
		backingText = "";
	}
	if (inputType == Number)
	{
		//Remove extra preceding 0's
		while(backingText[0] == '0' && backingText.length()>1)
			backingText.erase(backingText.begin());
	}
	if (cursor > (int)backingText.length())
		cursor = backingText.length();
	if (changed)
	{
		if (masked)
		{
			String maskedText = backingText;
			std::fill(maskedText.begin(), maskedText.end(), 0xE00D);
			Label::SetText(maskedText);
		}
		else
		{
			text = backingText;
		}
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
		Graphics::PositionAtCharIndex(multiline?textLines:text, cursor, cursorPositionX, cursorPositionY);
	}
	else
	{
		cursorPositionY = cursorPositionX = 0;
	}
	if (changed && actionCallback)
		actionCallback->TextChangedCallback(this);
}

void Textbox::OnMouseClick(int x, int y, unsigned button)
{

	if (button != SDL_BUTTON_RIGHT)
	{
		mouseDown = true;
		cursor = Graphics::CharIndexAtPosition(multiline?textLines:text, x-textPosition.X, y-textPosition.Y);
		if(cursor)
		{
			Graphics::PositionAtCharIndex(multiline?textLines:text, cursor, cursorPositionX, cursorPositionY);
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
		cursor = Graphics::CharIndexAtPosition(multiline?textLines:text, localx-textPosition.X, localy-textPosition.Y);
		if(cursor)
		{
			Graphics::PositionAtCharIndex(multiline?textLines:text, cursor, cursorPositionX, cursorPositionY);
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

	Graphics * g = GetGraphics();
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
	Graphics * g = GetGraphics();
	if(IsFocused())
	{
		if(border) g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);
		g->draw_line(screenPos.X+textPosition.X+cursorPosition, screenPos.Y+3, screenPos.X+textPosition.X+cursorPosition, screenPos.Y+12, 255, 255, 255, WINDOWW);
	}
	else
	{
		if(border) g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 160, 160, 160, 255);
	}
	g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, displayText, 255, 255, 255, 255);
	if(Appearance.icon)
		g->draw_icon(screenPos.X+iconPosition.X, screenPos.Y+iconPosition.Y, Appearance.icon);
}*/
