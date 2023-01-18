#pragma once
#include "Prefs.h"
#include "common/ExplicitSingleton.h"

class GlobalPrefs : public Prefs, public ExplicitSingleton<GlobalPrefs>
{
public:
	GlobalPrefs() : Prefs("powder.pref")
	{
	}
};
