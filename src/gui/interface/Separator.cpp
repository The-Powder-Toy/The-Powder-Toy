#include "Separator.h"
#include "graphics/Graphics.h"

namespace ui
{

void Separator::Draw(const ui::Point& screenPos)
{
	GetGraphics()->BlendRect(RectSized(screenPos, Size), 0xFFFFFF_rgb .WithAlpha(180));
}

}