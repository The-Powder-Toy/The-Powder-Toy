#include <iostream>
#include "Appearance.h"

namespace ui
{
	Appearance::Appearance():
		HorizontalAlign(AlignCentre),
		VerticalAlign(AlignMiddle),
	
		BackgroundHover(20, 20, 20),
		BackgroundInactive(0, 0, 0),
		BackgroundActive(255, 255, 255),
		BackgroundDisabled(10, 10, 10),
		
		TextHover(255, 255, 255),
		TextInactive(255, 255, 255),
		TextActive(0, 0, 0),
		TextDisabled(100, 100, 100),
		
		BorderHover(255, 255, 255),
		BorderInactive(200, 200, 200),
		BorderActive(235, 235, 235),
		BorderDisabled(100, 100, 100),

		Margin(1, 4),
		Border(1),
	
		icon(NoIcon),

		texture(NULL)
	{};

	VideoBuffer * Appearance::GetTexture()
	{
		return texture;
	}

	void Appearance::SetTexture(VideoBuffer * texture)
	{
		if(this->texture)
			delete this->texture;
		if(texture)
			this->texture = new VideoBuffer(texture);
		else
			this->texture = NULL;
	}

	Appearance::~Appearance()
	{
		if(texture)
			delete texture;
	}

}
