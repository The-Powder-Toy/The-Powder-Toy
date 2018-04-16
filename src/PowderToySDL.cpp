#ifdef USE_SDL

#include <map>
#include <string>
#include <ctime>
#include <climits>
#ifdef WIN
#define _WIN32_WINNT 0x0501	//Necessary for some macros and functions, tells windows.h to include functions only available in Windows XP or later
#include <direct.h>
#endif
#include "SDLCompat.h"

#ifdef X86_SSE
#include <xmmintrin.h>
#endif
#ifdef X86_SSE3
#include <pmmintrin.h>
#endif

#include <iostream>
#include <sstream>
#include <string>
#include "Config.h"
#include "graphics/Graphics.h"
#if defined(LIN)
#include "icon.h"
#endif
#include <signal.h>
#include <stdexcept>

#ifndef WIN
#include <unistd.h>
#endif

#include "Format.h"

#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "simulation/SaveRenderer.h"
#include "client/Client.h"
#include "Misc.h"

#include "gui/game/GameController.h"
#include "gui/game/GameView.h"

#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Keys.h"
#include "gui/Style.h"

#include "client/HTTP.h"

using namespace std;

#define INCLUDE_SYSWM
#include "SDLCompat.h"
#if defined(USE_SDL) && defined(LIN) && defined(SDL_VIDEO_DRIVER_X11)
SDL_SysWMinfo sdl_wminfo;
Atom XA_CLIPBOARD, XA_TARGETS, XA_UTF8_STRING;
#endif

int desktopWidth = 1280, desktopHeight = 1024;

SDL_Window * sdl_window;
SDL_Renderer * sdl_renderer;
SDL_Texture * sdl_texture;
int scale = 1;
bool fullscreen = false;

void ClipboardPush(std::string text)
{
	SDL_SetClipboardText(text.c_str());
}

std::string ClipboardPull()
{
	char *clipboardTemp = SDL_GetClipboardText();
	return std::string(clipboardTemp);
}

int mousex = 0, mousey = 0;
#ifdef OGLI
void blit()
{
	SDL_GL_SwapBuffers();
}
#else
void blit(pixel * vid)
{
	SDL_UpdateTexture(sdl_texture, NULL, vid, WINDOWW * sizeof (Uint32));
	SDL_RenderClear(sdl_renderer);
	SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
	SDL_RenderPresent(sdl_renderer);
}
#endif

int SDLOpen()
{
#if defined(WIN) && defined(WINCONSOLE)
	FILE * console = fopen("CON", "w" );
#endif
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_DisplayMode SDLDisplayMode;
	SDL_GetCurrentDisplayMode(0, &SDLDisplayMode);
	desktopWidth = SDLDisplayMode.w;
	desktopHeight = SDLDisplayMode.h;

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
	    printf("%s : %p\n", SDL_GetError(), SysInfo.window);
	    exit(-1);
	}
	HWND WindowHandle = SysInfo.window;

	// Use GetModuleHandle to get the Exe HMODULE/HINSTANCE
	HMODULE hModExe = GetModuleHandle(NULL);
	HICON hIconSmall = (HICON)LoadImage(hModExe, MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_SHARED);
	HICON hIconBig = (HICON)LoadImage(hModExe, MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, LR_SHARED);
	SendMessage(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	SendMessage(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
#elif defined(LIN)
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom((void*)app_icon, 48, 48, 24, 144, 0x00FF0000, 0x0000FF00, 0x000000FF, 0);
	//SDL_WM_SetIcon(icon, (Uint8*)app_icon_bitmap);
	SDL_SetWindowIcon(sdl_window, icon);
	SDL_FreeSurface(icon);
#endif

	atexit(SDL_Quit);

	return 0;
}

void SDLSetScreen(int newScale, bool newFullscreen)
{
	SDL_DestroyTexture(sdl_texture);
	SDL_DestroyRenderer(sdl_renderer);
	SDL_DestroyWindow(sdl_window);
	scale = newScale;
	fullscreen = newFullscreen;
	sdl_window = SDL_CreateWindow("The Powder Toy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOWW * newScale, WINDOWH * newScale,
								  newFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
	if (newScale > 1)
		SDL_RenderSetLogicalSize(sdl_renderer, WINDOWW, WINDOWH);
	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOWW, WINDOWH);
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	//SDL_SetWindowResizable(sdl_window, SDL_TRUE);
}

unsigned int GetTicks()
{
	return SDL_GetTicks();
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

/*SDLKey MapNumpad(SDLKey key)
{
	switch(key)
	{
	case SDLK_KP8:
		return SDLK_UP;
	case SDLK_KP2:
		return SDLK_DOWN;
	case SDLK_KP6:
		return SDLK_RIGHT;
	case SDLK_KP4:
		return SDLK_LEFT;
	case SDLK_KP7:
		return SDLK_HOME;
	case SDLK_KP1:
		return SDLK_END;
	case SDLK_KP_PERIOD:
		return SDLK_DELETE;
	case SDLK_KP0:
	case SDLK_KP9:
	case SDLK_KP3:
		return SDLK_UNKNOWN;
	default:
		return key;
	}
}*/

int elapsedTime = 0, currentTime = 0, lastTime = 0, currentFrame = 0;
unsigned int lastTick = 0;
float fps = 0, delta = 1.0f;
float inputScaleH = 1.0f, inputScaleV = 1.0f;
ui::Engine * engine = NULL;
bool showDoubleScreenDialog = false;
float currentWidth, currentHeight;

void EventProcess(SDL_Event event)
{
	//inputScale= 1.0f;
	/*if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
	{
		if (event.key.keysym.unicode==0)
		{
			// If unicode is zero, this could be a numpad key with numlock off, or numlock on and shift on (unicode is set to 0 by SDL or the OS in these circumstances. If numlock is on, unicode is the relevant digit character).
			// For some unknown reason, event.key.keysym.mod seems to be unreliable on some computers (keysum.mod&KEY_MOD_NUM is opposite to the actual value), so check keysym.unicode instead.
			// Note: unicode is always zero for SDL_KEYUP events, so this translation won't always work properly for keyup events.
			SDLKey newKey = MapNumpad(event.key.keysym.sym);
			if (newKey != event.key.keysym.sym)
			{
				event.key.keysym.sym = newKey;
				event.key.keysym.unicode = 0;
			}
		}
	}*/
	switch (event.type)
	{
	case SDL_QUIT:
		if (engine->GetFastQuit() || engine->CloseWindow())
			engine->Exit();
		break;
	case SDL_KEYDOWN:
		if (!event.key.repeat && event.key.keysym.sym == 'q' && (event.key.keysym.mod&KMOD_CTRL))
			engine->ConfirmExit();
		else
			engine->onKeyPress(event.key.keysym.sym, event.key.keysym.scancode, event.key.repeat, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_KEYUP:
		engine->onKeyRelease(event.key.keysym.sym, event.key.keysym.scancode, event.key.repeat, event.key.keysym.mod&KMOD_SHIFT, event.key.keysym.mod&KMOD_CTRL, event.key.keysym.mod&KMOD_ALT);
		break;
	case SDL_TEXTINPUT:
		engine->onTextInput(std::string(event.text.text));
		break;
	case SDL_MOUSEWHEEL:
	{
		int x = event.wheel.x;
		int y = event.wheel.y;
		if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
		{
			x *= -1;
			y *= -1;
		}
		bool positiveDir = y == 0 ? x > 0 : y > 0;
		engine->onMouseWheel(event.motion.x * inputScaleH, event.motion.y * inputScaleV, positiveDir ? 1 : -1);
		break;
	}
	case SDL_MOUSEMOTION:
		engine->onMouseMove(event.motion.x * inputScaleH, event.motion.y * inputScaleV);
		mousex = event.motion.x * inputScaleH;
		mousey = event.motion.y * inputScaleV;
		break;
	case SDL_MOUSEBUTTONDOWN:
		engine->onMouseClick(event.motion.x * inputScaleH, event.motion.y * inputScaleV, event.button.button);
		mousex = event.motion.x * inputScaleH;
		mousey = event.motion.y * inputScaleV;
		break;
	case SDL_MOUSEBUTTONUP:
		engine->onMouseUnclick(event.motion.x * inputScaleH, event.motion.y * inputScaleV, event.button.button);
		mousex = event.motion.x * inputScaleH;
		mousey = event.motion.y * inputScaleV;
		break;
	case SDL_WINDOWEVENT:
	{
		if (event.window.event != SDL_WINDOWEVENT_RESIZED)
			break;
		float width = event.window.data1;
		float height = event.window.data2;

		currentWidth = width;
		currentHeight = height;
		// this "* scale" thing doesn't really work properly
		// currently there is a bug where input doesn't scale properly after resizing, only when double scale mode is active
		inputScaleH = (float)WINDOWW * scale / currentWidth;
		inputScaleV = (float)WINDOWH * scale / currentHeight;

		break;
	}
	}
}

void DoubleScreenDialog()
{
	std::stringstream message;
	message << "Switching to double size mode since your screen was determined to be large enough: ";
	message << desktopWidth << "x" << desktopHeight << " detected, " << WINDOWW*2 << "x" << WINDOWH*2 << " required";
	message << "\nTo undo this, hit Cancel. You can toggle double size mode in settings at any time.";
	if (!ConfirmPrompt::Blocking("Large screen detected", message.str()))
	{
		Client::Ref().SetPref("Scale", 1);
		engine->SetScale(1);
#ifdef WIN
		LoadWindowPosition(1);
#endif
	}
}

void EngineProcess()
{
	double frameTimeAvg = 0.0f, correctedFrameTimeAvg = 0.0f;
	SDL_Event event;
	while(engine->Running())
	{
		int frameStart = SDL_GetTicks();
		if(engine->Broken()) { engine->UnBreak(); break; }
		event.type = 0;
		while (SDL_PollEvent(&event))
		{
			EventProcess(event);
			event.type = 0; //Clear last event
		}
		if(engine->Broken()) { engine->UnBreak(); break; }

		engine->Tick();
		engine->Draw();

		if(scale != engine->Scale || fullscreen != engine->Fullscreen)
		{
			SDLSetScreen(engine->Scale, engine->Fullscreen);
		}

#ifdef OGLI
		blit();
#else
		blit(engine->g->vid);
#endif

		int frameTime = SDL_GetTicks() - frameStart;
		frameTimeAvg = frameTimeAvg * 0.8 + frameTime * 0.2;
		int fpsLimit = ui::Engine::Ref().FpsLimit;
		if(fpsLimit > 2)
		{
			double offset = 1000.0 / fpsLimit - frameTimeAvg;
			if(offset > 0)
				SDL_Delay(offset + 0.5);
		}
		int correctedFrameTime = SDL_GetTicks() - frameStart;
		correctedFrameTimeAvg = correctedFrameTimeAvg * 0.95 + correctedFrameTime * 0.05;
		engine->SetFps(1000.0 / correctedFrameTimeAvg);
		if(frameStart - lastTick > 1000)
		{
			//Run client tick every second
			lastTick = frameStart;
			Client::Ref().Tick();
		}
		if (showDoubleScreenDialog)
		{
			showDoubleScreenDialog = false;
			DoubleScreenDialog();
		}
	}
#ifdef DEBUG
	std::cout << "Breaking out of EngineProcess" << std::endl;
#endif
}

int GetModifiers()
{
	return SDL_GetModState();
}

#ifdef WIN

// Returns true if the loaded position was set
// Returns false if something went wrong: SDL_GetWMInfo failed or the loaded position was invalid
bool LoadWindowPosition(int scale)
{
	SDL_SysWMinfo sysInfo;
	SDL_VERSION(&sysInfo.version);
	if (SDL_GetWMInfo(&sysInfo) > 0)
	{
		int windowW = WINDOWW * scale;
		int windowH = WINDOWH * scale;

		int savedWindowX = Client::Ref().GetPrefInteger("WindowX", INT_MAX);
		int savedWindowY = Client::Ref().GetPrefInteger("WindowY", INT_MAX);

		// Center the window on the primary desktop by default
		int newWindowX = (desktopWidth - windowW) / 2;
		int newWindowY = (desktopHeight - windowH) / 2;

		bool success = false;

		if (savedWindowX != INT_MAX && savedWindowY != INT_MAX)
		{
			POINT windowPoints[] = {
				{savedWindowX, savedWindowY},                       // Top-left
				{savedWindowX + windowW, savedWindowY + windowH}    // Bottom-right
			};

			MONITORINFO monitor;
			monitor.cbSize = sizeof(monitor);
			if (GetMonitorInfo(MonitorFromPoint(windowPoints[0], MONITOR_DEFAULTTONEAREST), &monitor) != 0)
			{
				// Only use the saved window position if it lies inside the visible screen
				if (PtInRect(&monitor.rcMonitor, windowPoints[0]) && PtInRect(&monitor.rcMonitor, windowPoints[1]))
				{
					newWindowX = savedWindowX;
					newWindowY = savedWindowY;

					success = true;
				}
				else
				{
					// Center the window on the nearest monitor
					newWindowX = monitor.rcMonitor.left + (monitor.rcMonitor.right - monitor.rcMonitor.left - windowW) / 2;
					newWindowY = monitor.rcMonitor.top + (monitor.rcMonitor.bottom - monitor.rcMonitor.top - windowH) / 2;
				}
			}
		}

		SetWindowPos(sysInfo.window, 0, newWindowX, newWindowY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

		// True if we didn't use the default, i.e. the position was valid
		return success;
	}

	return false;
}

// Returns true if the window position was saved
bool SaveWindowPosition()
{
	SDL_SysWMinfo sysInfo;
	SDL_VERSION(&sysInfo.version);
	if (SDL_GetWMInfo(&sysInfo) > 0)
	{
		WINDOWPLACEMENT placement;
		placement.length = sizeof(placement);
		GetWindowPlacement(sysInfo.window, &placement);

		Client::Ref().SetPref("WindowX", (int)placement.rcNormalPosition.left);
		Client::Ref().SetPref("WindowY", (int)placement.rcNormalPosition.top);

		return true;
	}

	return false;
}

#endif

void BlueScreen(const char * detailMessage)
{
	ui::Engine * engine = &ui::Engine::Ref();
	engine->g->fillrect(0, 0, engine->GetWidth(), engine->GetHeight(), 17, 114, 169, 210);

	std::string errorTitle = "ERROR";
	std::string errorDetails = "Details: " + std::string(detailMessage);
	std::string errorHelp = "An unrecoverable fault has occurred, please report the error by visiting the website below\n"
		"http://" SERVER;
	int currentY = 0, width, height;
	int errorWidth = 0;
	Graphics::textsize(errorHelp.c_str(), errorWidth, height);

	engine->g->drawtext((engine->GetWidth()/2)-(errorWidth/2), ((engine->GetHeight()/2)-100) + currentY, errorTitle.c_str(), 255, 255, 255, 255);
	Graphics::textsize(errorTitle.c_str(), width, height);
	currentY += height + 4;

	engine->g->drawtext((engine->GetWidth()/2)-(errorWidth/2), ((engine->GetHeight()/2)-100) + currentY, errorDetails.c_str(), 255, 255, 255, 255);
	Graphics::textsize(errorTitle.c_str(), width, height);
	currentY += height + 4;

	engine->g->drawtext((engine->GetWidth()/2)-(errorWidth/2), ((engine->GetHeight()/2)-100) + currentY, errorHelp.c_str(), 255, 255, 255, 255);
	Graphics::textsize(errorTitle.c_str(), width, height);
	currentY += height + 4;

	//Death loop
	SDL_Event event;
	while(true)
	{
		while (SDL_PollEvent(&event))
			if(event.type == SDL_QUIT)
				exit(-1);
#ifdef OGLI
		blit();
#else
		blit(engine->g->vid);
#endif
	}
}

void SigHandler(int signal)
{
	switch(signal){
	case SIGSEGV:
		BlueScreen("Memory read/write error");
		break;
	case SIGFPE:
		BlueScreen("Floating point exception");
		break;
	case SIGILL:
		BlueScreen("Program execution exception");
		break;
	case SIGABRT:
		BlueScreen("Unexpected program abort");
		break;
	}
}

int main(int argc, char * argv[])
{
#if defined(_DEBUG) && defined(_MSC_VER)
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif
	currentWidth = WINDOWW;
	currentHeight = WINDOWH;


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

	// TODO: maybe bind the maximum allowed scale to screen size somehow
	if(tempScale < 1 || tempScale > 10)
		tempScale = 1;

	SDLOpen();
	// TODO: mabe make a nice loop that automagically finds the optimal scale
	if (Client::Ref().IsFirstRun() && desktopWidth > WINDOWW*2+50 && desktopHeight > WINDOWH*2+50)
	{
		tempScale = 2;
		Client::Ref().SetPref("Scale", 2);
		showDoubleScreenDialog = true;
	}
#ifdef WIN
	LoadWindowPosition(tempScale);
#endif
	SDLSetScreen(tempScale, tempFullscreen);

#ifdef OGLI
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	//glScaled(2.0f, 2.0f, 1.0f);
#endif
#if defined(OGLI) && !defined(MACOSX)
	int status = glewInit();
	if(status != GLEW_OK)
	{
		fprintf(stderr, "Initializing Glew: %d\n", status);
		exit(-1);
	}
#endif
	ui::Engine::Ref().g = new Graphics();
	ui::Engine::Ref().Scale = scale;
	ui::Engine::Ref().Fullscreen = fullscreen;

	engine = &ui::Engine::Ref();
	engine->SetMaxSize(desktopWidth, desktopHeight);
	engine->Begin(WINDOWW, WINDOWH);
	engine->SetFastQuit(Client::Ref().GetPrefBool("FastQuit", true));

#if !defined(DEBUG) && !defined(_DEBUG)
	//Get ready to catch any dodgy errors
	signal(SIGSEGV, SigHandler);
	signal(SIGFPE, SigHandler);
	signal(SIGILL, SigHandler);
	signal(SIGABRT, SigHandler);
#endif

#ifdef X86_SSE
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif
#ifdef X86_SSE3
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif

	GameController * gameController = NULL;
#if !defined(DEBUG) && !defined(_DEBUG)
	try {
#endif

		gameController = new GameController();
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
			blit(engine->g->vid);
#endif
			std::string ptsaveArg = arguments["ptsave"];
			try
			{
				if (ptsaveArg.find("ptsave:"))
					throw std::runtime_error("Invalid save link");

				std::string saveIdPart = "";
				int saveId;
				size_t hashPos = ptsaveArg.find('#');
				if (hashPos != std::string::npos)
				{
					saveIdPart = ptsaveArg.substr(7, hashPos-7);
				}
				else
				{
					saveIdPart = ptsaveArg.substr(7);
				}
				if (!saveIdPart.length())
					throw std::runtime_error("No Save ID");
#ifdef DEBUG
				std::cout << "Got Ptsave: id: " <<  saveIdPart << std::endl;
#endif
				saveId = format::StringToNumber<int>(saveIdPart);
				if (!saveId)
					throw std::runtime_error("Invalid Save ID");

				SaveInfo * newSave = Client::Ref().GetSave(saveId, 0);
				if (!newSave)
					throw std::runtime_error("Could not load save info");
				std::vector<unsigned char> saveData = Client::Ref().GetSaveData(saveId, 0);
				if (!saveData.size())
					throw std::runtime_error("Could not load save\n" + Client::Ref().GetLastError());
				GameSave * newGameSave = new GameSave(saveData);
				newSave->SetGameSave(newGameSave);

				gameController->LoadSave(newSave);
				delete newSave;
			}
			catch (std::exception & e)
			{
				new ErrorMessage("Error", e.what());
			}
		}

		//initial mouse coords
		int sdl_x, sdl_y;
		SDL_GetMouseState(&sdl_x, &sdl_y);
		engine->onMouseMove(sdl_x * inputScaleH, sdl_y * inputScaleV);
		EngineProcess();

#ifdef WIN
		SaveWindowPosition();
#endif

#if !defined(DEBUG) && !defined(_DEBUG)
	}
	catch(exception& e)
	{
		BlueScreen(e.what());
	}
#endif

	Client::Ref().SetPref("Scale", ui::Engine::Ref().GetScale());
	ui::Engine::Ref().CloseWindow();
	delete gameController;
	delete ui::Engine::Ref().g;
	Client::Ref().Shutdown();
	return 0;
}

#endif
