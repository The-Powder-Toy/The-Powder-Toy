#pragma once
#include "Component.h"

namespace ui
{

class Separator : public Component
{
public:
	Separator(ui::Point position, ui::Point size) : Component(position, size)
	{ }

	void Draw(const ui::Point& screenPos) override;
};

}