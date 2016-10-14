#ifdef USE_SDL

#ifdef SDL_INC
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

#if defined(WIN) || defined(LIN)
#ifdef SDL_INC
#include <SDL/SDL_syswm.h>
#else
#include <SDL_syswm.h>
#endif
#endif

#endif //USE_SDL