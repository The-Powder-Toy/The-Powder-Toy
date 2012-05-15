#ifndef LABEL_H
#define LABEL_H

#include <string>

#include "Component.h"
#include "Misc.h"
#include "Colour.h"

namespace ui
{
	class Label : public Component
	{
	protected:
		std::string text;
		Colour textColour;
	public:
		//Label(Window* parent_state, std::string labelText);
		Label(Point position, Point size, std::string labelText);
		//Label(std::string labelText);
		virtual ~Label();

		virtual void SetText(std::string text);
		virtual std::string GetText();

		void SetTextColour(Colour textColour) { this->textColour = textColour; }


		virtual void Draw(const Point& screenPos);
	};
}

#endif // LABEL_H
