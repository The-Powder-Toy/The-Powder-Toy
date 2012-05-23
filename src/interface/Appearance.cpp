//
//  Appearance.cpp
//  The Powder Toy
//
//  Created by Simon Robertshaw on 15/05/2012.
//

#include <iostream>
#include "Appearance.h"

namespace ui
{
	Appearance::Appearance():
		HorizontalAlign(AlignCentre),
		VerticalAlign(AlignMiddle),
	
		BackgroundHover(30, 30, 30),
		BackgroundInactive(0, 0, 0),
		BackgroundActive(255, 255, 255),
		BackgroundDisabled(100, 100, 100),
		
		TextHover(255, 255, 255),
		TextInactive(255, 255, 255),
		TextActive(0, 0, 0),
		
		BorderHover(255, 255, 255),
		BorderInactive(200, 200, 200),
		BorderActive(255, 255, 255),
		Margin(1, 4),
	
		icon(NoIcon)
	{};
}
