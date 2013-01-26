#ifdef USE_SDL

#include <map>
#include <string>
#include <time.h>
#include "SDL.h"
#ifdef WIN
#include "SDL_syswm.h"
#include <direct.h>
#endif
#include <iostream>
#include <sstream>
#include <string>
#include "Config.h"
#include "graphics/Graphics.h"
#if defined(LIN)
#include "icon.h"
#endif

#ifndef WIN
#include <unistd.h>
#endif

#include "Format.h"
#include "Style.h"
#include "interface/Engine.h"
#include "interface/Button.h"
#include "interface/Panel.h"
#include "interface/Point.h"
#include "interface/Label.h"
#include "interface/Keys.h"

#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "simulation/SaveRenderer.h"
#include "client/Client.h"
#include "Misc.h"

#include "game/GameController.h"
#include "game/GameView.h"

#include "dialogues/ErrorMessage.h"

#include "client/HTTP.h"

using namespace std;

#if defined(USE_SDL) && defined(LIN)
#include <SDL_syswm.h>
#endif
#if defined(USE_SDL) && defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
SDL_SysWMinfo sdl_wminfo;
Atom XA_CLIPBOARD, XA_TARGETS;
#endif

char *clipboardText = NULL;

#ifdef WIN
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

int desktopWidth = 1280, desktopHeight = 1024;

SDL_Surface * sdl_scrn;
int scale = 1;
bool fullscreen = false;

void ClipboardPush(char * text)
{
	if (clipboardText != NULL) {
		free(clipboardText);
		clipboardText = NULL;
	}
	clipboardText = mystrdup(text);
#ifdef MACOSX
	PasteboardRef newclipboard;

	if (PasteboardCreate(kPasteboardClipboard, &newclipboard)!=noErr) return;
	if (PasteboardClear(newclipboard)!=noErr) return;
	PasteboardSynchronize(newclipboard);

	CFDataRef data = CFDataCreate(kCFAllocatorDefault, (const UInt8*)text, strlen(text));
	PasteboardPutItemFlavor(newclipboard, (PasteboardItemID)1, CFSTR("com.apple.traditional-mac-plain-text"), data, 0);
#elif defined(WIN)
	if (OpenClipboard(NULL))
	{
		HGLOBAL cbuffer;
		char * glbuffer;

		EmptyClipboard();

		cbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(text)+1);
		glbuffer = (char*)GlobalLock(cbuffer);

		strcpy(glbuffer, text);

		GlobalUnlock(cbuffer);
		SetClipboardData(CF_TEXT, cbuffer);
		CloseClipboard();
	}
#elif defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	sdl_wminfo.info.x11.lock_func();
	XSetSelectionOwner(sdl_wminfo.info.x11.display, XA_CLIPBOARD, sdl_wminfo.info.x11.window, CurrentTime);
	XFlush(sdl_wminfo.info.x11.display);
	sdl_wminfo.info.x11.unlock_func();
#else
	printf("Not implemented: put text on clipboard \"%s\"\n", text);
#endif
}

char * ClipboardPull()
{
#ifdef MACOSX
	printf("Not implemented: get text from clipboard\n");
#elif defined(WIN)
	if (OpenClipboard(NULL))
	{
		HANDLE cbuffer;
		char * glbuffer;

		cbuffer = GetClipboardData(CF_TEXT);
		glbuffer = (char*)GlobalLock(cbuffer);
		GlobalUnlock(cbuffer);
		CloseClipboard();
		if(glbuffer!=NULL){
			return mystrdup(glbuffer);
		}// else {
		//	return mystrdup("");
		//}
	}
#elif defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	printf("Not implemented: get text from clipboard\n");
#else
	printf("Not implemented: get text from clipboard\n");
#endif
	if (clipboardText)
		return mystrdup(clipboardText);
	return mystrdup("");
}

#ifdef OGLI
void blit()
{
	SDL_GL_SwapBuffers();
}
#else
void blit(pixel * vid)
{
	if(sdl_scrn)
	{
		pixel * src = vid;
		int j, x = 0, y = 0, w = XRES+BARSIZE, h = YRES+MENUSIZE, pitch = XRES+BARSIZE;
		pixel *dst;
		if (SDL_MUSTLOCK(sdl_scrn))
			if (SDL_LockSurface(sdl_scrn)<0)
				return;
		dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
		if (SDL_MapRGB(sdl_scrn->format,0x33,0x55,0x77)!=PIXPACK(0x335577))
		{
			//pixel format conversion
			int i;
			pixel px;
			SDL_PixelFormat *fmt = sdl_scrn->format;
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
				{
					px = src[i];
					dst[i] = ((PIXR(px)>>fmt->Rloss)<<fmt->Rshift)|
							((PIXG(px)>>fmt->Gloss)<<fmt->Gshift)|
							((PIXB(px)>>fmt->Bloss)<<fmt->Bshift);
				}
				dst+=sdl_scrn->pitch/PIXELSIZE;
				src+=pitch;
			}
		}
		else
		{
			for (j=0; j<h; j++)
			{
				memcpy(dst, src, w*PIXELSIZE);
				dst+=sdl_scrn->pitch/PIXELSIZE;
				src+=pitch;
			}
		}
		if (SDL_MUSTLOCK(sdl_scrn))
			SDL_UnlockSurface(sdl_scrn);
		SDL_UpdateRect(sdl_scrn,0,0,0,0);
	}
}
void blit2(pixel * vid, int currentScale)
{
	if(sdl_scrn)
	{
		pixel * src = vid;
		int j, x = 0, y = 0, w = XRES+BARSIZE, h = YRES+MENUSIZE, pitch = XRES+BARSIZE;
		pixel *dst;
		int i,k;
		if (SDL_MUSTLOCK(sdl_scrn))
			if (SDL_LockSurface(sdl_scrn)<0)
				return;
		dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
		if (SDL_MapRGB(sdl_scrn->format,0x33,0x55,0x77)!=PIXPACK(0x335577))
		{
			//pixel format conversion
			pixel px;
			SDL_PixelFormat *fmt = sdl_scrn->format;
			for (j=0; j<h; j++)
			{
				for (k=0; k<currentScale; k++)
				{
					for (i=0; i<w; i++)
					{
						px = src[i];
						px = ((PIXR(px)>>fmt->Rloss)<<fmt->Rshift)|
							((PIXG(px)>>fmt->Gloss)<<fmt->Gshift)|
							((PIXB(px)>>fmt->Bloss)<<fmt->Bshift);
						dst[i*2]=px;
						dst[i*2+1]=px;
					}
					dst+=sdl_scrn->pitch/PIXELSIZE;
				}
				src+=pitch;
			}
		}
		else
		{
			for (j=0; j<h; j++)
			{
				for (k=0; k<currentScale; k++)
				{
					for (i=0; i<w; i++)
					{
						dst[i*2]=src[i];
						dst[i*2+1]=src[i];
					}
					dst+=sdl_scrn->pitch/PIXELSIZE;
				}
				src+=pitch;
			}
		}
		if (SDL_MUSTLOCK(sdl_scrn))
			SDL_UnlockSurface(sdl_scrn);
		SDL_UpdateRect(sdl_scrn,0,0,0,0);
	}
}
#endif

int SDLOpen()
{
	SDL_Surface * surface;
#if defined(WIN) && defined(WINCONSOLE)
	FILE * console = fopen("CON", "w" );
#endif
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 1;
	}
	const SDL_VideoInfo * vidInfo = SDL_GetVideoInfo();
	desktopWidth = vidInfo->current_w;
	desktopHeight = vidInfo->current_h;
	SDL_EnableUNICODE(1);
#if defined(WIN) && defined(WINCONSOLE)
	//On Windows, SDL redirects stdout to stdout.txt, which can be annoying when debugging, here we redirect back to the console
	if (console)
	{
		freopen("CON", "w", stdout);
		freopen("CON", "w", stderr);
		//fclose(console);
	}
#endif
#ifdef WIN
	SDL_SysWMinfo SysInfo;
	SDL_VERSION(&SysInfo.version);
	if(SDL_GetWMInfo(&SysInfo) <= 0) {
	    printf("%s : %d\n", SDL_GetError(), SysInfo.window);
	    exit(-1);
	}
	HWND WindowHandle = SysInfo.window;
	HICON hIconSmall = (HICON)LoadImage(reinterpret_cast<HMODULE>(&__ImageBase), MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_SHARED);
	HICON hIconBig = (HICON)LoadImage(reinterpret_cast<HMODULE>(&__ImageBase), MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, LR_SHARED);
	SendMessage(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	SendMessage(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
#elif defined(LIN)
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(app_icon, 16, 16, 32, 64, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_WM_SetIcon(icon, NULL);
#endif

	SDL_WM_SetCaption("The Powder Toy", "Powder Toy");
	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	atexit(SDL_Quit);

	return 0;
}

SDL_Surface * SDLSetScreen(int newScale, bool newFullscreen)
{
	scale = newScale;
	fullscreen = newFullscreen;
	SDL_Surface * surface;
#ifndef OGLI
	surface = SDL_SetVideoMode((XRES + BARSIZE) * newScale, (YRES + MENUSIZE) * newScale, 32, SDL_SWSURFACE | (newFullscreen?SDL_FULLSCREEN:0));
#else
	surface = SDL_SetVideoMode((XRES + BARSIZE) * newScale, (YRES + MENUSIZE) * newScale, 32, SDL_OPENGL | SDL_RESIZABLE | (newFullscreen?SDL_FULLSCREEN:0));
#endif
	return surface;
}

std::map<std::string, std::string> readArguments(int argc, char * argv[])
{
	std::map<std::string, std::string> arguments;

	//Defaults
	arguments["scale"] = "";
	arguments["proxy"] = "";
	arguments["nohud"] = "false"; //the nohud, sound, and scripts commands currently do nothing.
	arguments["sound"] = "false";
	arguments["kiosk"] = "false";
	arguments["scripts"] = "false";
	arguments["open"] = "";
	arguments["ddir"] = "";
	arguments["ptsave"] = "";

	for (int i=1; i<argc; i++)
	{
		if (!strncmp(argv[i], "scale:", 6) && argv[i]+6)
		{
			arguments["scale"] = std::string(argv[i]+6);
		}
		else if (!strncmp(argv[i], "proxy:", 6))
		{
			if(argv[i]+6)
				arguments["proxy"] =  std::string(argv[i]+6);
			else
				arguments["proxy"] = "false";
		}
		else if (!strncmp(argv[i], "nohud", 5))
		{
			arguments["nohud"] = "true";
		}
		else if (!strncmp(argv[i], "kiosk", 5))
		{
			arguments["kiosk"] = "true";
		}
		else if (!strncmp(argv[i], "sound", 5))
		{
			arguments["sound"] = "true";
		}
		else if (!strncmp(argv[i], "scripts", 8))
		{
			arguments["scripts"] = "true";
		}
		else if (!strncmp(argv[i], "open", 5) && i+1<argc)
		{
			arguments["open"] = std::string(argv[i+1]);;
			i++;
		}
		else if (!strncmp(argv[i], "ddir", 5) && i+1<argc)
		{
			arguments["ddir"] = std::string(argv[i+1]);
			i++;
		}
		else if (!strncmp(argv[i], "ptsave", 7) && i+1<argc)
		{
			arguments["ptsave"] = std::string(argv[i+1]);
			i++;
			break;
		}
	}
	return arguments;
}

int elapsedTime = 0, currentTime = 0, lastTime = 0, currentFrame = 0;
unsigned int lastTick = 0;
float fps = 0, delta = 1.0f, inputScale = 1.0f;
ui::Engine * engine = NULL;
float currentWidth, currentHeight;
void EngineProcess()
{
	int frameStart;
	float frameTime;
	float frameTimeAvg = 0.0f, correctedFrameTimeAvg = 0.0f;
	SDL_Event event;
	while(engine->Running())
	{
		if(engine->Broken()) { engine->UnBreak(); break; }
		event.type = 0;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				if (engine->GetFastQuit() || engine->CloseWindow())
					engine->Exit();
				break;
			case SDL_KEYDOWN:
				engine->onKeyPress(event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod&KEY_MOD_LSHIFT, event.key.keysym.mod&KEY_MOD_LCONTROL, event.key.keysym.mod&KEY_MOD_LALT);
				break;
			case SDL_KEYUP:
				engine->onKeyRelease(event.key.keysym.sym, event.key.keysym.unicode, event.key.keysym.mod&KEY_MOD_LSHIFT, event.key.keysym.mod&KEY_MOD_LCONTROL, event.key.keysym.mod&KEY_MOD_LALT);
				break;
			case SDL_MOUSEMOTION:
				engine->onMouseMove(event.motion.x*inputScale, event.motion.y*inputScale);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button == SDL_BUTTON_WHEELUP)
				{
					engine->onMouseWheel(event.motion.x*inputScale, event.motion.y*inputScale, 1);
				}
				else if (event.button.button == SDL_BUTTON_WHEELDOWN)
				{
					engine->onMouseWheel(event.motion.x*inputScale, event.motion.y*inputScale, -1);
				}
				else
				{
					engine->onMouseClick(event.motion.x*inputScale, event.motion.y*inputScale, event.button.button);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if(event.button.button != SDL_BUTTON_WHEELUP && event.button.button != SDL_BUTTON_WHEELDOWN)
					engine->onMouseUnclick(event.motion.x*inputScale, event.motion.y*inputScale, event.button.button);
				break;
#ifdef OGLI
			case SDL_VIDEORESIZE:
				float ratio = float(XRES+BARSIZE) / float(YRES+MENUSIZE);
				float width = event.resize.w;
				float height = width/ratio;

				sdl_scrn = SDL_SetVideoMode(event.resize.w, height, 32, SDL_OPENGL | SDL_RESIZABLE);

				glViewport(0, 0, width, height);
				engine->g->Reset();
				//glScaled(width/currentWidth, height/currentHeight, 1.0f);

				currentWidth = width;
				currentHeight = height;
				inputScale = float(XRES+BARSIZE)/currentWidth;

				glLineWidth(currentWidth/float(XRES+BARSIZE));
				if(sdl_scrn == NULL)
				{
					std::cerr << "Oh bugger" << std::endl;
				}
				break;
#endif
#if defined (USE_SDL) && defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
			case SDL_SYSWMEVENT:
				if (event.syswm.msg->subsystem != SDL_SYSWM_X11)
					break;
				sdl_wminfo.info.x11.lock_func();
				XEvent xe = event.syswm.msg->event.xevent;
				if (xe.type==SelectionClear)
				{
					if (clipboardText != NULL) {
						free(clipboardText);
						clipboardText = NULL;
					}
				}
				else if (xe.type==SelectionRequest)
				{
					XEvent xr;
					xr.xselection.type = SelectionNotify;
					xr.xselection.requestor = xe.xselectionrequest.requestor;
					xr.xselection.selection = xe.xselectionrequest.selection;
					xr.xselection.target = xe.xselectionrequest.target;
					xr.xselection.property = xe.xselectionrequest.property;
					xr.xselection.time = xe.xselectionrequest.time;
					if (xe.xselectionrequest.target==XA_TARGETS)
					{
						// send list of supported formats
						Atom targets[] = {XA_TARGETS, XA_STRING};
						xr.xselection.property = xe.xselectionrequest.property;
						XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, XA_ATOM, 32, PropModeReplace, (unsigned char*)targets, (int)(sizeof(targets)/sizeof(Atom)));
					}
					// TODO: Supporting more targets would be nice
					else if (xe.xselectionrequest.target==XA_STRING && clipboardText)
					{
						XChangeProperty(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, xe.xselectionrequest.property, xe.xselectionrequest.target, 8, PropModeReplace, (unsigned char*)clipboardText, strlen(clipboardText)+1);
					}
					else
					{
						// refuse clipboard request
						xr.xselection.property = None;
					}
					XSendEvent(sdl_wminfo.info.x11.display, xe.xselectionrequest.requestor, 0, 0, &xr);
				}
				sdl_wminfo.info.x11.unlock_func();
				continue;
#endif
			}
			event.type = 0; //Clear last event
		}
		if(engine->Broken()) { engine->UnBreak(); break; }

		frameStart = SDL_GetTicks();
		engine->Tick();
		engine->Draw();
		frameTime = SDL_GetTicks() - frameStart;
		
		frameTimeAvg = (frameTimeAvg*(1.0f-0.2f)) + (0.2f*frameTime);
		if(ui::Engine::Ref().FpsLimit > 2.0f)
		{
			float targetFrameTime = 1000.0f/((float)ui::Engine::Ref().FpsLimit);
			if(targetFrameTime - frameTimeAvg > 0)
			{
				SDL_Delay((targetFrameTime - frameTimeAvg) + 0.5f);
				frameTime = SDL_GetTicks() - frameStart;//+= (int)(targetFrameTime - frameTimeAvg);
			}
		}

		correctedFrameTimeAvg = (correctedFrameTimeAvg*(1.0f-0.05f)) + (0.05f*frameTime);
		fps = 1000.0f/correctedFrameTimeAvg;
		engine->SetFps(fps);

		if(frameStart-lastTick>250)
		{
			//Run client tick every second
			lastTick = frameStart;
			Client::Ref().Tick();
		}

		if(scale != engine->Scale || fullscreen != engine->Fullscreen)
		{
			sdl_scrn = SDLSetScreen(engine->Scale, engine->Fullscreen);
			inputScale = 1.0f/float(scale);
		}

#ifdef OGLI
		blit();
#else
		if(engine->Scale==2)
			blit2(engine->g->vid, engine->Scale);
		else
			blit(engine->g->vid);
#endif
	}
#ifdef DEBUG
	std::cout << "Breaking out of EngineProcess" << std::endl;
#endif
}

int main(int argc, char * argv[])
{
	currentWidth = XRES+BARSIZE; 
	currentHeight = YRES+MENUSIZE;


	std::map<std::string, std::string> arguments = readArguments(argc, argv);

	if(arguments["ddir"].length())
#ifdef WIN
		_chdir(arguments["ddir"].c_str());
#else
		chdir(arguments["ddir"].c_str());
#endif

	int tempScale = 1;
	bool tempFullscreen = false;

	tempScale = Client::Ref().GetPrefInteger("Scale", 1);
	tempFullscreen = Client::Ref().GetPrefBool("Fullscreen", false);


	if(arguments["kiosk"] == "true")
	{
		tempFullscreen = true;
		Client::Ref().SetPref("Fullscreen", tempFullscreen);
	}

	if(arguments["scale"].length())
	{
		tempScale = format::StringToNumber<int>(arguments["scale"]);
		Client::Ref().SetPref("Scale", tempScale);
	}

	std::string proxyString = "";
	if(arguments["proxy"].length())
	{
		if(arguments["proxy"] == "false")
		{
			proxyString = "";
			Client::Ref().SetPref("Proxy", "");	
		}
		else
		{
			proxyString = (arguments["proxy"]);
			Client::Ref().SetPref("Proxy", arguments["proxy"]);
		}
	}
	else if(Client::Ref().GetPrefString("Proxy", "").length())
	{
		proxyString = (Client::Ref().GetPrefString("Proxy", ""));
	}

	Client::Ref().Initialise(proxyString);

	if(tempScale != 1 && tempScale != 2)
		tempScale = 1;

	int sdlStatus = SDLOpen();
	sdl_scrn = SDLSetScreen(tempScale, tempFullscreen);
#ifdef OGLI
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	//glScaled(2.0f, 2.0f, 1.0f);
#endif
#if defined(OGLI)
	int status = glewInit();
	if(status != GLEW_OK)
	{
		fprintf(stderr, "Initializing Glew: %d\n", status);
		exit(-1);
	}
#endif
#if defined (USE_SDL) && defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	SDL_VERSION(&sdl_wminfo.version);
	SDL_GetWMInfo(&sdl_wminfo);
	sdl_wminfo.info.x11.lock_func();
	XA_CLIPBOARD = XInternAtom(sdl_wminfo.info.x11.display, "CLIPBOARD", 1);
	XA_TARGETS = XInternAtom(sdl_wminfo.info.x11.display, "TARGETS", 1);
	sdl_wminfo.info.x11.unlock_func();
#endif
	ui::Engine::Ref().g = new Graphics();
	ui::Engine::Ref().Scale = scale;
	inputScale = 1.0f/float(scale);
	ui::Engine::Ref().Fullscreen = fullscreen;

	engine = &ui::Engine::Ref();
	engine->SetMaxSize(desktopWidth, desktopHeight);
	engine->Begin(XRES+BARSIZE, YRES+MENUSIZE);
	engine->SetFastQuit(Client::Ref().GetPrefBool("FastQuit", true));

	GameController * gameController = new GameController();
	engine->ShowWindow(gameController->GetView());

	if(arguments["open"].length())
	{
#ifdef DEBUG
		std::cout << "Loading " << arguments["open"] << std::endl;
#endif
		if(Client::Ref().FileExists(arguments["open"]))
		{
			try
			{
				std::vector<unsigned char> gameSaveData = Client::Ref().ReadFile(arguments["open"]);
				if(!gameSaveData.size())
				{
					new ErrorMessage("Error", "Could not read file");
				}
				else
				{
					SaveFile * newFile = new SaveFile(arguments["open"]);
					GameSave * newSave = new GameSave(gameSaveData);
					newFile->SetGameSave(newSave);
					gameController->LoadSaveFile(newFile);
					delete newFile;
				}

			}
			catch(std::exception & e)
			{
				new ErrorMessage("Error", "Could not open save file:\n"+std::string(e.what())) ;
			}
		}
		else
		{
			new ErrorMessage("Error", "Could not open file");
		}
	}

	if(arguments["ptsave"].length())
	{
		engine->g->fillrect((engine->GetWidth()/2)-101, (engine->GetHeight()/2)-26, 202, 52, 0, 0, 0, 210);
		engine->g->drawrect((engine->GetWidth()/2)-100, (engine->GetHeight()/2)-25, 200, 50, 255, 255, 255, 180);
		engine->g->drawtext((engine->GetWidth()/2)-(Graphics::textwidth("Loading save...")/2), (engine->GetHeight()/2)-5, "Loading save...", style::Colour::InformationTitle.Red, style::Colour::InformationTitle.Green, style::Colour::InformationTitle.Blue, 255);

#ifdef OGLI
		blit();
#else
		if(engine->Scale==2)
			blit2(engine->g->vid, engine->Scale);
		else
			blit(engine->g->vid);
#endif
		std::string ptsaveArg = arguments["ptsave"];
		try
		{
		if(!ptsaveArg.find("ptsave:"))
		{
			std::string saveIdPart = "";
			int saveId;
			int hashPos = ptsaveArg.find('#');
			if(hashPos != std::string::npos)
			{
				saveIdPart = ptsaveArg.substr(7, hashPos-7);
			}
			else
			{
				saveIdPart = ptsaveArg.substr(7);
			}
			if(saveIdPart.length())
			{
#ifdef DEBUG
				std::cout << "Got Ptsave: id: " <<  saveIdPart << std::endl;
#endif
				saveId = format::StringToNumber<int>(saveIdPart);
				if(!saveId)
					throw std::runtime_error("Invalid Save ID");

				SaveInfo * newSave = Client::Ref().GetSave(saveId, 0);
				GameSave * newGameSave = new GameSave(Client::Ref().GetSaveData(saveId, 0));
				newSave->SetGameSave(newGameSave);
				if(!newSave)
					throw std::runtime_error("Could not load save");

				gameController->LoadSave(newSave);
				delete newSave;
			}
			else
			{
				throw std::runtime_error("No Save ID");
			}
		}
		}
		catch (std::exception & e)
		{
			new ErrorMessage("Error", "Invalid save link");
		}
	}

	EngineProcess();
	
	ui::Engine::Ref().CloseWindow();
	delete gameController;
	delete ui::Engine::Ref().g;
	Client::Ref().Shutdown();
	return 0;
}

#endif
