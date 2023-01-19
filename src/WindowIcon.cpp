#include "WindowIcon.h"
#include "graphics/Graphics.h"
#include "icon_exe.png.h"

void WindowIcon(SDL_Window *window)
{
	std::vector<pixel> imageData;
	int imgw, imgh;
	if (PngDataToPixels(imageData, imgw, imgh, reinterpret_cast<const char *>(icon_exe_png), icon_exe_png_size, false))
	{
		SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(&imageData[0], imgw, imgh, 32, imgw * sizeof(pixel), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		SDL_SetWindowIcon(window, icon);
		SDL_FreeSurface(icon);
	}
}
