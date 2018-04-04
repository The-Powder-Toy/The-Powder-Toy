#ifndef LABEL_H
#define LABEL_H

#include <string>

#include "Component.h"
#include "Colour.h"

namespace ui
{
	class Label : public Component
	{
	protected:
		std::string textFragments;
		std::string textLines;
		std::string displayText;
		std::string tDisplayText;

		std::string text;
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
		//Label(std::string labelText);
		~Label() override;

		virtual void SetMultiline(bool status);

		virtual void SetText(std::string text);
		virtual void SetDisplayText(std::string newText);
		virtual std::string GetText();

		virtual bool HasSelection();
		virtual void ClearSelection();
		virtual void selectAll();
		virtual void AutoHeight();

		void SetTextColour(Colour textColour) { this->textColour = textColour; }

		void OnContextMenuAction(int item) override;
		void OnMouseClick(int x, int y, unsigned button) override;
		void OnMouseUp(int x, int y, unsigned button) override;
		void OnMouseMoved(int localx, int localy, int dx, int dy) override;
		void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt) override;
		void Draw(const Point& screenPos) override;
		void Tick(float dt) override;
	};
}

#endif // LABEL_H
