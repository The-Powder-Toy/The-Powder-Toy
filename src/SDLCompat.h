#ifdef USE_SDL

#ifdef SDL_INC
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

#ifdef INCLUDE_SYSWM
#if defined(WIN) || defined(LIN)
#ifdef SDL_INC
#include <SDL/SDL_syswm.h>
#else
#include <SDL_syswm.h>
#endif
#endif // WIN || LIN
#endif // INCLUDE_SYSWM

#endif // USE_SDL
