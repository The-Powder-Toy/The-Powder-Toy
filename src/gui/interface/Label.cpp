#include "Label.h"

#include "Format.h"
#include "Point.h"
#include "Keys.h"
#include "Mouse.h"
#include "PowderToy.h"
#include "ContextMenu.h"

#include "graphics/Graphics.h"

using namespace ui;

Label::Label(Point position, Point size, String labelText):
	Component(position, size),
	textColour(255, 255, 255),
	selectionIndexL(textWrapper.IndexBegin()),
	selectionIndexH(textWrapper.IndexBegin()),
	multiline(false),
	selecting(false),
	autoHeight(size.Y==-1?true:false)
{
	SetText(labelText);

	menu = new ContextMenu(this);
	menu->AddItem(ContextMenuItem("Copy", 0, true));
}

Label::~Label()
{
}

void Label::SetMultiline(bool status)
{
	multiline = status;
	updateTextWrapper();
	updateSelection();
	TextPosition(displayTextWrapper.WrappedText());
}

void Label::SetText(String newText)
{
	this->text = newText;
	updateTextWrapper();
	updateSelection();
	TextPosition(displayTextWrapper.WrappedText());
}

void Label::AutoHeight()
{
	bool oldAH = autoHeight;
	autoHeight = true;
	updateTextWrapper();
	autoHeight = oldAH;
}

void Label::updateTextWrapper()
{
	int lines = textWrapper.Update(
		text,
		multiline,
		Size.X - Appearance.Margin.Left - Appearance.Margin.Right
	);
	displayTextWrapper.Update(
		displayText.size() ? displayText : text,
		multiline,
		Size.X - Appearance.Margin.Left - Appearance.Margin.Right
	);
	if (autoHeight)
	{
		Size.Y = lines * FONT_H + 3;
	}
}

String Label::GetText()
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
		if (menu)
		{
			menu->Show(GetScreenPos() + ui::Point(x, y));
		}
	}
	else
	{
		selecting = true;
		selectionIndex0 = textWrapper.Point2Index(x - textPosition.X, y - textPosition.Y);
		selectionIndexL = selectionIndex0;
		selectionIndexH = selectionIndex0;

		updateSelection();
	}
}

void Label::copySelection()
{
	if (HasSelection())
	{
		ClipboardPush(format::CleanString(text.Between(selectionIndexL.raw_index, selectionIndexH.raw_index), false, true, false).ToUtf8());
	}
	else
	{
		ClipboardPush(format::CleanString(text, false, true, false).ToUtf8());
	}
}

void Label::OnMouseUp(int x, int y, unsigned button)
{
	selecting = false;
}

void Label::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
	{
		return;
	}
	if (ctrl && scan == SDL_SCANCODE_C)
	{
		copySelection();
	}
	if (ctrl && scan == SDL_SCANCODE_A)
	{
		selectAll();
		return;
	}
}

void Label::OnMouseMoved(int localx, int localy, int dx, int dy)
{
	if (selecting)
	{
		selectionIndex1 = textWrapper.Point2Index(localx - textPosition.X, localy - textPosition.Y);
		if (selectionIndex1.raw_index < selectionIndex0.raw_index)
		{
			selectionIndexL = selectionIndex1;
			selectionIndexH = selectionIndex0;
		}
		else
		{
			selectionIndexL = selectionIndex0;
			selectionIndexH = selectionIndex1;
		}
		updateSelection();
	}
}

void Label::Tick(float dt)
{
	if (!this->IsFocused() && (HasSelection() || selecting))
	{
		ClearSelection();
	}
}

int Label::getLowerSelectionBound()
{
	return selectionIndexL.raw_index;
}

int Label::getHigherSelectionBound()
{
	return selectionIndexH.raw_index;
}

bool Label::HasSelection()
{
	return selectionIndexH.raw_index > selectionIndexL.raw_index;
}

void Label::ClearSelection()
{
	selecting = false;
	selectionIndexL = textWrapper.IndexBegin();
	selectionIndexH = textWrapper.IndexBegin();
	updateSelection();
}

void Label::selectAll()
{
	selectionIndexL = textWrapper.IndexBegin();
	selectionIndexH = textWrapper.IndexEnd();
	updateSelection();
}

void Label::updateSelection()
{
	if (selectionIndexL.raw_index <                  0) selectionIndexL = textWrapper.IndexBegin();
	if (selectionIndexL.raw_index > (int)text.length()) selectionIndexL = textWrapper.IndexEnd();
	if (selectionIndexH.raw_index <                  0) selectionIndexH = textWrapper.IndexBegin();
	if (selectionIndexH.raw_index > (int)text.length()) selectionIndexH = textWrapper.IndexEnd();

	displayTextWithSelection = displayTextWrapper.WrappedText();
	if (HasSelection())
	{
		auto indexL = displayTextWrapper.Clear2Index(selectionIndexL.clear_index);
		auto indexH = displayTextWrapper.Clear2Index(selectionIndexH.clear_index);
		displayTextWithSelection.Insert(indexL.wrapped_index    , "\x01");
		displayTextWithSelection.Insert(indexH.wrapped_index + 1, "\x01");
	}
}

void Label::SetDisplayText(String newText)
{
	displayText = newText;
	ClearSelection();
	updateTextWrapper();
	updateSelection();
	TextPosition(displayTextWrapper.WrappedText());
}

void Label::Draw(const Point& screenPos)
{
	if (!drawn)
	{
		TextPosition(displayTextWrapper.WrappedText());
		updateTextWrapper();
		updateSelection();
		drawn = true;
	}
	Graphics *g = GetGraphics();

	auto indexL = displayTextWrapper.Clear2Index(selectionIndexL.clear_index);
	auto indexH = displayTextWrapper.Clear2Index(selectionIndexH.clear_index);
		
	int selectionXL;
	int selectionYL;
	int selectionLineL = displayTextWrapper.Index2Point(indexL, selectionXL, selectionYL);

	int selectionXH;
	int selectionYH;
	int selectionLineH = displayTextWrapper.Index2Point(indexH, selectionXH, selectionYH);

	if (HasSelection())
	{
		if (selectionLineH == selectionLineL)
		{
			g->fillrect(
				screenPos.X + textPosition.X + selectionXL - 1,
				screenPos.Y + textPosition.Y + selectionYL - 2,
				selectionXH - selectionXL + 1,
				FONT_H,
				255, 255, 255, 255
			);
		}
		else
		{
			g->fillrect(
				screenPos.X + textPosition.X + selectionXL - 1,
				screenPos.Y + textPosition.Y + selectionYL - 2,
				textSize.X - selectionXL + 1,
				FONT_H,
				255, 255, 255, 255
			);
			for (int i = 1; i < selectionLineH - selectionLineL; ++i)
			{
				g->fillrect(
					screenPos.X + textPosition.X - 1,
					screenPos.Y + textPosition.Y + selectionYL - 2 + i * FONT_H,
					textSize.X + 1,
					FONT_H,
					255, 255, 255, 255
				);
			}
			g->fillrect(
				screenPos.X + textPosition.X - 1,
				screenPos.Y + textPosition.Y + selectionYH - 2,
				selectionXH + 1,
				FONT_H,
				255, 255, 255, 255
			);
		}
	}

	g->drawtext(
		screenPos.X + textPosition.X,
		screenPos.Y + textPosition.Y,
		displayTextWithSelection,
		textColour.Red, textColour.Green, textColour.Blue, 255
	);
}

