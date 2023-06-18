#include "Platform.h"
#include "common/String.h"
#include "Config.h"
#include <SDL.h>
#include <memory>

namespace Platform
{
ByteString DefaultDdir()
{
	auto ddir = std::unique_ptr<char, decltype(&SDL_free)>(SDL_GetPrefPath(NULL, APPDATA), SDL_free);
	return ddir.get();
}
}
