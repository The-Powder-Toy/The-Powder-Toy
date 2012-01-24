
#include <time.h>
#include <SDL/SDL.h>
#include <iostream>
#include <sstream>
#include <string>
#include "Config.h"
#include "Global.h"
#include "Graphics.h"

#include "interface/Engine.h"
#include "interface/Button.h"
#include "interface/Panel.h"
#include "interface/ControlFactory.h"
#include "interface/Point.h"
#include "interface/Label.h"

#include "game/GameController.h"
#include "game/GameView.h"

#include "client/HTTP.h"

using namespace std;

SDL_Surface * SDLOpen()
{
#if defined(WIN32) && defined(WINCONSOLE)
	FILE * console = fopen("CON", "w" );
#endif
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 0;
	}
#if defined(WIN32) && defined(WINCONSOLE)
	//On Windows, SDL redirects stdout to stdout.txt, which can be annoying when debugging, here we redirect back to the console
	if (console)
	{
		freopen("CON", "w", stdout);
		freopen("con", "w", stderr);
		fclose(console);
	}
#endif
	atexit(SDL_Quit);
	return SDL_SetVideoMode(XRES + BARSIZE, YRES + MENUSIZE, 32, SDL_SWSURFACE);
}

/*int SDLPoll(SDL_Event * event)
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
}*/

int main(int argc, char * argv[])
{
	int elapsedTime = 0, currentTime = 0, lastTime = 0, currentFrame = 0;
	float fps = 0, delta = 1.0f;

	ui::Engine::Ref().g = new Graphics();
	ui::Engine::Ref().g->AttachSDLSurface(SDLOpen());

	ui::Engine * engine = &ui::Engine::Ref();
	engine->Begin(XRES+BARSIZE, YRES+MENUSIZE);

	GameController * gameController = new GameController();
	engine->ShowWindow(gameController->GetView());

	SDL_Event event;
	while(engine->Running())
	{
		event.type = 0;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				engine->Exit();
				break;
			case SDL_KEYDOWN:
				engine->onKeyPress(event.key.keysym.sym, false, false, false);
				break;
			case SDL_KEYUP:
				break;
			case SDL_MOUSEMOTION:
				engine->onMouseMove(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button == SDL_BUTTON_WHEELUP)
				{
					engine->onMouseWheel(event.motion.x, event.motion.y, 1);
				}
				else if (event.button.button == SDL_BUTTON_WHEELDOWN)
				{
					engine->onMouseWheel(event.motion.x, event.motion.y, -1);
				}
				else
				{
					engine->onMouseClick(event.motion.x, event.motion.y, event.button.button);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if(event.button.button != SDL_BUTTON_WHEELUP && event.button.button != SDL_BUTTON_WHEELDOWN)
					engine->onMouseUnclick(event.motion.x, event.motion.y, event.button.button);
				break;
			}
			event.type = 0; //Clear last event
		}

		engine->Tick(delta);
		engine->Draw();

		currentFrame++;
		currentTime = SDL_GetTicks();
		elapsedTime = currentTime - lastTime;
		if(ui::Engine::Ref().FpsLimit > 2.0f && (currentFrame>2 || elapsedTime > 1000*2/ui::Engine::Ref().FpsLimit) && elapsedTime && currentFrame*1000/elapsedTime > ui::Engine::Ref().FpsLimit)
		{
			while (currentFrame*1000/elapsedTime > ui::Engine::Ref().FpsLimit)
			{
				SDL_Delay(1);
				currentTime = SDL_GetTicks();
				elapsedTime = currentTime-lastTime;
			}
		}
		if(elapsedTime>=1000)
		{
			fps = (((float)currentFrame)/((float)elapsedTime))*1000.0f;
			currentFrame = 0;
			lastTime = currentTime;
			delta = 60.0f/fps;
		}
	}
	ui::Engine::Ref().CloseWindow();
	delete gameController;
	delete ui::Engine::Ref().g;
}
