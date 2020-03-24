#pragma once

#include <vector>

#include "common/Localization.h"

extern Locale const &Locale_EN;

const std::vector<Locale const *> locales =
{
	&Locale_EN,
};
