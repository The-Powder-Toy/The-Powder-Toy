//
//  Appearance.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 15/05/2012.
//

#ifndef The_Powder_Toy_Appearance_h
#define The_Powder_Toy_Appearance_h

#include "Border.h"
#include "Colour.h"
#include "graphics/Graphics.h"

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
