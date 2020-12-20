#ifndef STYLE_H_
#define STYLE_H_
#include "Config.h"

#include "gui/interface/Colour.h"

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
