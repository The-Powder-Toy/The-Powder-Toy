#pragma once

#include <functional>

#include "String.h"

class Locale
{
public:
	// The name of the language this locale is for, readable in both the native
	// language and in English;
	std::function<String()> GetName;

	// Populate the translations map.
	std::function<void()> Set;
};
