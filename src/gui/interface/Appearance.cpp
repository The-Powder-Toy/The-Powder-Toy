#include "Appearance.h"
#include "graphics/Graphics.h"
#include "graphics/VideoBuffer.h"
#include <iostream>

namespace ui
{
	Appearance::Appearance():
		texture(nullptr),

		VerticalAlign(AlignMiddle),
		HorizontalAlign(AlignCentre),

		BackgroundHover(0x141414_rgb .WithAlpha(0xFF)),
		BackgroundInactive(0x000000_rgb .WithAlpha(0xFF)),
		BackgroundActive(0xFFFFFF_rgb .WithAlpha(0xFF)),
		BackgroundDisabled(0x0A0A0A_rgb .WithAlpha(0xFF)),

		TextHover(0xFFFFFF_rgb .WithAlpha(0xFF)),
		TextInactive(0xFFFFFF_rgb .WithAlpha(0xFF)),
		TextActive(0x000000_rgb .WithAlpha(0xFF)),
		TextDisabled(0x646464_rgb .WithAlpha(0xFF)),

		BorderHover(0xFFFFFF_rgb .WithAlpha(0xFF)),
		BorderInactive(0xC8C8C8_rgb .WithAlpha(0xFF)),
		BorderActive(0xEBEBEB_rgb .WithAlpha(0xFF)),
		BorderFavorite(0xFFFF00_rgb .WithAlpha(0xFF)),
		BorderDisabled(0x646464_rgb .WithAlpha(0xFF)),

		Margin(1, 4),
		Border(1),

		icon(NoIcon)
	{}

	VideoBuffer const *Appearance::GetTexture()
	{
		return texture.get();
	}

	void Appearance::SetTexture(std::unique_ptr<VideoBuffer> texture)
	{
		this->texture = std::move(texture);
	}
}
