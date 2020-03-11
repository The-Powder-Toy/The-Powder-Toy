#include "common/Localization.h"
#include "common/Internationalization.h"

Locale LocaleEN {
	[]{ return String("English"); },
	[]
	{
		using i18n::translation;
	}
};
