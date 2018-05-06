#ifdef SDL_INC
#include "SDL2/SDL.h"
#else
#include "SDL.h"
#endif

#ifdef INCLUDE_SYSWM
#if defined(WIN)
#ifdef SDL_INC
#include <SDL2/SDL_syswm.h>
#else
#include <SDL_syswm.h>
#endif
#endif // WIN
#endif // INCLUDE_SYSWM
