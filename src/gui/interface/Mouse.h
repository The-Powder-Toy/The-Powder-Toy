
#ifdef USE_SDL

#ifdef SDL_INC
#include "SDL/SDL_mouse.h"
#else
#include "SDL_mouse.h"
#endif

#else // USE_SDL

/* These are used for the renderer, which doesn't include the rest of SDL and only this file
 * It still needs fake SDL_BUTTON_* constants to compile
 */
 
#define SDL_BUTTON(X)		(1 << ((X)-1))
#define SDL_BUTTON_LEFT		1
#define SDL_BUTTON_MIDDLE	2
#define SDL_BUTTON_RIGHT	3
#define SDL_BUTTON_WHEELUP	4
#define SDL_BUTTON_WHEELDOWN	5

#endif // USE_SDL

