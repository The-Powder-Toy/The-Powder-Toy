//
//  Style.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 14/05/2012.
//

#ifndef The_Powder_Toy_Style_h
#define The_Powder_Toy_Style_h

namespace ui { class Colour; }

namespace style
{
	class Colour
	{
	public:
		static ui::Colour InformationTitle;
		static ui::Colour WarningTitle;
		static ui::Colour ErrorTitle;
		
		static ui::Colour ConfirmButton;
		
		static ui::Colour ActiveBorder;
		static ui::Colour InactiveBorder;
		
		static ui::Colour ActiveBackground;
		static ui::Colour InactiveBackground;
	};
	class Metrics
	{ 
	};
}

#endif
