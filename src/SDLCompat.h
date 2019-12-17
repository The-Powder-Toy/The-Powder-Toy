#include "Config.h"
#include "SDL2/SDL.h"
#ifdef INCLUDE_SYSWM
# if defined(WIN)
#  include <SDL2/SDL_syswm.h>
# endif // WIN
#endif // INCLUDE_SYSWM
