#pragma once
#include "bzip2/bz2wrap.h"
#include <array>
#include <cassert>
#include <span>
#include <string>

enum class ResourceCompression
{
	none,
	bzip2,
};
