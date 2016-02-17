#ifndef LABEL_H
#define LABEL_H

#include <string>

#include "Component.h"
#include "PowderToy.h"
#include "Colour.h"

namespace ui
{
	class Label : public Component
	{
	protected:
		std::wstring textFragments;
		std::wstring textLines;
		std::wstring displayText;
		std::wstring tDisplayText;

		std::wstring text;
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
		//Label(Window* parent_state, std::string labelText);
		Label(Point position, Point size, std::string labelText);
		Label(Point position, Point size, std::wstring labelText);
		//Label(std::string labelText);
		virtual ~Label();

		virtual void SetMultiline(bool status);

		virtual void SetText(std::string text);
		virtual void SetText(std::wstring text);
		virtual void SetDisplayText(std::string newText);
		virtual void SetDisplayText(std::wstring newText);
		virtual std::string GetText();
		virtual std::wstring GetWText();

		virtual bool HasSelection();
		virtual void ClearSelection();
		virtual void selectAll();
		virtual void AutoHeight();

		void SetTextColour(Colour textColour) { this->textColour = textColour; }

		virtual void OnContextMenuAction(int item);
		virtual void OnMouseClick(int x, int y, unsigned button);
		virtual void OnMouseUp(int x, int y, unsigned button);
		virtual void OnMouseMoved(int localx, int localy, int dx, int dy);
		virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
		virtual void Draw(const Point& screenPos);
		virtual void Tick(float dt);
	};
}

#endif // LABEL_H
