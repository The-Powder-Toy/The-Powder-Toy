#pragma once

#include <functional>

#include "String.h"

struct Locale
{
	// The name of the language this locale is for, readable in both the native
	// language and in English;
	virtual String GetName() const = 0;

	// Populate the translations map.
	virtual void Set() const = 0;

	virtual String GetIntroText() const = 0;
	virtual String GetSavePublishingInfo() const = 0;
	virtual String GetRules() const = 0;
};
