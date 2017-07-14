#ifndef APPEARANCE_H_
#define APPEARANCE_H_

#include "Border.h"
#include "Colour.h"
#include "graphics/Icons.h"

class VideoBuffer;
namespace ui
{
	class Appearance
	{
	private:
			VideoBuffer * texture;
	public:
		enum HorizontalAlignment
		{
			AlignLeft, AlignCentre, AlignRight
		};
		
		enum VerticalAlignment
		{
			AlignTop, AlignMiddle, AlignBottom
		};
		
		VerticalAlignment VerticalAlign;
		HorizontalAlignment HorizontalAlign;
		
		ui::Colour BackgroundHover;
		ui::Colour BackgroundInactive;
		ui::Colour BackgroundActive;
		ui::Colour BackgroundDisabled;
		
		ui::Colour TextHover;
		ui::Colour TextInactive;
		ui::Colour TextActive;
		ui::Colour TextDisabled;
		
		ui::Colour BorderHover;
		ui::Colour BorderInactive;
		ui::Colour BorderActive;
		ui::Colour BorderFavorite;
		ui::Colour BorderDisabled;
		
		ui::Border Margin;

		ui::Border Border;
		
		Icon icon;

		VideoBuffer * GetTexture();
		void SetTexture(VideoBuffer * texture);
		
		Appearance();
		~Appearance();
	};
}

#endif
