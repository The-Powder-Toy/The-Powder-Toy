#ifndef LABEL_H
#define LABEL_H

#include "common/String.h"

#include "Component.h"
#include "Colour.h"
#include "TextWrapper.h"

namespace ui
{
	class Label : public Component
	{
	protected:
		String textFragments;
		String displayTextWithSelection;

		String text;
		TextWrapper textWrapper;

		String displayText;
		TextWrapper displayTextWrapper;

		Colour textColour;
		TextWrapper::Index selectionIndex0;
		TextWrapper::Index selectionIndex1;
		TextWrapper::Index selectionIndexL;
		TextWrapper::Index selectionIndexH;

		bool multiline;
		bool selecting;
		bool autoHeight;

		void updateTextWrapper();
		void updateSelection();

		int getLowerSelectionBound();
		int getHigherSelectionBound();


		void copySelection();
	public:
		//Label(Window* parent_state, String labelText);
		Label(Point position, Point size, String labelText);
		//Label(String labelText);
		virtual ~Label();

		void SetMultiline(bool status);

		virtual void SetText(String text);
		virtual void SetDisplayText(String text);
		virtual String GetText();

		bool HasSelection();
		void ClearSelection();
		void selectAll();
		void AutoHeight();

		void SetTextColour(Colour textColour) { this->textColour = textColour; }

		void OnContextMenuAction(int item) override;
		void OnMouseClick(int x, int y, unsigned button) override;
		void OnMouseUp(int x, int y, unsigned button) override;
		void OnMouseMoved(int localx, int localy, int dx, int dy) override;
		void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
		void Draw(const Point& screenPos) override;
		void Tick(float dt) override;
	};
}

#endif // LABEL_H
