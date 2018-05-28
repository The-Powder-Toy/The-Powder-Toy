#ifndef LABEL_H
#define LABEL_H

#include "common/String.h"

#include "Component.h"
#include "Colour.h"

namespace ui
{
	class Label : public Component
	{
	protected:
		String textFragments;
		String textLines;
		String displayText;
		String tDisplayText;

		String text;
		Colour textColour;
		int selectionIndex0;
		int selectionIndex1;

		int selectionXL;
		int selectionXH;
		int selectionYL;
		int selectionYH;
		int selectionLineL;
		int selectionLineH;

		bool multiline;
		bool selecting;
		bool autoHeight;

		void updateMultiline();
		void updateSelection();

		int getLowerSelectionBound();
		int getHigherSelectionBound();

		virtual void copySelection();
	public:
		//Label(Window* parent_state, String labelText);
		Label(Point position, Point size, String labelText);
		//Label(String labelText);
		virtual ~Label();

		virtual void SetMultiline(bool status);

		virtual void SetText(String text);
		virtual void SetDisplayText(String newText);
		virtual String GetText();

		virtual bool HasSelection();
		virtual void ClearSelection();
		virtual void selectAll();
		virtual void AutoHeight();

		void SetTextColour(Colour textColour) { this->textColour = textColour; }

		virtual void OnContextMenuAction(int item);
		virtual void OnMouseClick(int x, int y, unsigned button);
		virtual void OnMouseUp(int x, int y, unsigned button);
		virtual void OnMouseMoved(int localx, int localy, int dx, int dy);
		virtual void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
		virtual void Draw(const Point& screenPos);
		virtual void Tick(float dt);
	};
}

#endif // LABEL_H
