
#include <time.h>
#include <SDL/SDL.h>
#include "Config.h"
#include "Simulation.h"
#include "Renderer.h"
#include "Graphics.h"
#include "Air.h"

#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Sandbox.h"

SDL_Surface * SDLOpen()
{
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 0;
	}
	atexit(SDL_Quit);
	return SDL_SetVideoMode(XRES + BARSIZE, YRES + MENUSIZE, 32, SDL_SWSURFACE);
}

int SDLPoll(SDL_Event * event)
{
	while (SDL_PollEvent(event))
	{
		switch (event->type)
		{
			case SDL_QUIT:
				return 1;
		}
	}
	return 0;
}

int main(int argc, char * argv[])
{
	int mouseX, mouseY, mouseButton, lastMouseButton;

	//Renderer * ren;
	Graphics * g = new Graphics();
	g->AttachSDLSurface(SDLOpen());
	//Simulation * sim = new Simulation();
	//ren = new Renderer(g, sim);

	ui::Window * window = new ui::Window();
	ui::Sandbox * sandbox = new ui::Sandbox();
	ui::Button * button = new ui::Button(100, 100, 100, 100, "poP");
	window->Add(sandbox);
	window->Add(button);

	SDL_Event event;
	while(!SDLPoll(&event))
	{
		mouseButton = SDL_GetMouseState(&mouseX, &mouseY);
		switch(event.type)
		{
		case SDL_KEYDOWN:
			break;
		case SDL_KEYUP:
			break;
		case SDL_MOUSEMOTION:
			window->OnMouseMove(event.motion.x, event.motion.y);
			break;
		case SDL_MOUSEBUTTONDOWN:
			window->OnMouseDown(event.motion.x, event.motion.y, event.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			window->OnMouseUp(event.motion.x, event.motion.y, event.button.button);
			break;
		}
		window->Tick(1.0f);
		window->Draw(g);
		/*sim->update_particles();
		sim->air->update_air();
		mouseButton = SDL_GetMouseState(&mouseX, &mouseY);
		if(mouseButton)
		{
			sim->create_parts(mouseX, mouseY, 4, 4, (rand()%4)+1, 0);
		}
		if(mouseButton==4 && !lastMouseButton)
		{
			sim->sys_pause = !sim->sys_pause;
		}
		//ren->render_parts();
		//ren->render_fire();


		ren->g->clearrect(0, 0, XRES+BARSIZE, YRES+MENUSIZE);*/
		g->Blit();
		g->Clear();
	}
}
