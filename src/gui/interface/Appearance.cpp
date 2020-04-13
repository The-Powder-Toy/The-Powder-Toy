#include <iostream>
#include "Appearance.h"
#include "graphics/Graphics.h"

namespace ui
{
	Appearance::Appearance() :
		texture(NULL),

		VerticalAlign(AlignMiddle),
		HorizontalAlign(AlignCentre),

		BackgroundHover(0, 0, 100),
		BackgroundInactive(0, 0, 40),
		BackgroundActive(100, 100, 250),
		BackgroundDisabled(0, 0, 10),

		TextHover(100, 100, 250),
		TextInactive(100, 100, 250),
		TextActive(0, 0, 50),
		TextDisabled(0, 0, 240),


		BorderHover(255, 255, 255),
		BorderInactive(70, 70, 70),
		BorderActive(235, 235, 235),
		BorderDisabled(100, 100, 100),

		Margin(1, 4),
		Border(1),

		icon(NoIcon)
	{}

	VideoBuffer * Appearance::GetTexture()
	{
		return texture;
	}

	void Appearance::SetTexture(VideoBuffer * texture)
	{
		delete this->texture;
		if (texture)
			this->texture = new VideoBuffer(texture);
		else
			this->texture = NULL;
	}

	Appearance::~Appearance()
	{
		delete texture;
	}

}
