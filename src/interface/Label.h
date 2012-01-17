#ifndef LABEL_H
#define LABEL_H

#include <string>

#include "Component.h"

namespace ui
{
	class Label : public Component
	{
	public:
		Label(Window* parent_state, std::string labelText);

		Label(Point position, Point size, std::string labelText);

		Label(std::string labelText);
		virtual ~Label();

		std::string LabelText;

		virtual void Draw(const Point& screenPos);
	};
}

#endif // LABEL_H
