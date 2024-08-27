#include "Format.h"
#include "graphics/VideoBuffer.h"
#include "WindowIcon.h"

#include "icon_exe.png.h"

void WindowIcon(SDL_Window *window)
{
	if (auto image = format::PixelsFromPNG(std::vector<char>(icon_exe_png, icon_exe_png + icon_exe_png_size)))
	{
		SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(image->data(), image->Size().X, image->Size().Y, 32, image->Size().Y * sizeof(pixel), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		SDL_SetWindowIcon(window, icon);
		SDL_FreeSurface(icon);
	}
}
