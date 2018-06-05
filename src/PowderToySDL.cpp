#ifndef RENDERER

#include <map>
#include "common/String.h"
#include <ctime>
#include <climits>
#ifdef WIN
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

#include "gui/font/FontEditor.h"

#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Keys.h"
#include "gui/Style.h"

#include "client/HTTP.h"

using namespace std;

#define INCLUDE_SYSWM
#include "SDLCompat.h"

int desktopWidth = 1280, desktopHeight = 1024;

SDL_Window * sdl_window;
SDL_Renderer * sdl_renderer;
SDL_Texture * sdl_texture;
int scale = 1;
bool fullscreen = false;
bool altFullscreen = false;
bool resizable = false;


void ClipboardPush(ByteString text)
{
	SDL_SetClipboardText(text.c_str());
}

ByteString ClipboardPull()
{
	return ByteString(SDL_GetClipboardText());
}

int GetModifiers()
{
	return SDL_GetModState();
}

void LoadWindowPosition()
{
	int savedWindowX = Client::Ref().GetPrefInteger("WindowX", INT_MAX);
	int savedWindowY = Client::Ref().GetPrefInteger("WindowY", INT_MAX);

	int borderTop, borderLeft;
	SDL_GetWindowBordersSize(sdl_window, &borderTop, &borderLeft, nullptr, nullptr);

	if (savedWindowX + borderLeft > 0 && savedWindowX + borderLeft < desktopWidth
	        && savedWindowY + borderTop > 0 && savedWindowY + borderTop < desktopHeight)
		SDL_SetWindowPosition(sdl_window, savedWindowX + borderLeft, savedWindowY + borderTop);
}

void SaveWindowPosition()
{
	int x, y;
	SDL_GetWindowPosition(sdl_window, &x, &y);

	int borderTop, borderLeft;
	SDL_GetWindowBordersSize(sdl_window, &borderTop, &borderLeft, nullptr, nullptr);

	Client::Ref().SetPref("WindowX", x - borderLeft);
	Client::Ref().SetPref("WindowY", y - borderTop);
}

void CalculateMousePosition(int *x, int *y)
{
	int globalMx, globalMy;
	SDL_GetGlobalMouseState(&globalMx, &globalMy);
	int windowX, windowY;
	SDL_GetWindowPosition(sdl_window, &windowX, &windowY);

	if (x)
		*x = (globalMx - windowX) / scale;
	if (y)
		*y = (globalMy - windowY) / scale;
}

#ifdef OGLI
void blit()
{
	SDL_GL_SwapBuffers();
}
#else
void blit(pixel * vid)
{
	SDL_UpdateTexture(sdl_texture, NULL, vid, WINDOWW * sizeof (Uint32));
	// need to clear the renderer if there are black edges (fullscreen, or resizable window)
	if (fullscreen || resizable)
		SDL_RenderClear(sdl_renderer);
	SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
	SDL_RenderPresent(sdl_renderer);
}
#endif

int SDLOpen()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	unsigned int flags = 0;
	if (fullscreen)
		flags = altFullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;
	if (resizable)
		flags |= SDL_WINDOW_RESIZABLE;
	sdl_window = SDL_CreateWindow("The Powder Toy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOWW * scale, WINDOWH * scale,
	                              flags);
	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
	SDL_RenderSetLogicalSize(sdl_renderer, WINDOWW, WINDOWH);
	//Uncomment this to force fullscreen to an integer resolution
	//SDL_RenderSetIntegerScale(sdl_renderer, SDL_TRUE);
	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOWW, WINDOWH);
	if (fullscreen)
		SDL_RaiseWindow(sdl_window);
	//Uncomment this to enable resizing
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	//SDL_SetWindowResizable(sdl_window, SDL_TRUE);

	int displayIndex = SDL_GetWindowDisplayIndex(sdl_window);
	if (displayIndex >= 0)
	{
		SDL_Rect rect;
		if (!SDL_GetDisplayUsableBounds(displayIndex, &rect))
		{
			desktopWidth = rect.w;
			desktopHeight = rect.h;
		}
	}

#ifdef WIN
	SDL_SysWMinfo SysInfo;
	SDL_VERSION(&SysInfo.version);
	if(SDL_GetWindowWMInfo(sdl_window, &SysInfo) <= 0)
	{
	    printf("%s : %p\n", SDL_GetError(), SysInfo.info.win.window);
	    exit(-1);
	}
	HWND WindowHandle = SysInfo.info.win.window;

	// Use GetModuleHandle to get the Exe HMODULE/HINSTANCE
	HMODULE hModExe = GetModuleHandle(NULL);
	HICON hIconSmall = (HICON)LoadImage(hModExe, MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_SHARED);
	HICON hIconBig = (HICON)LoadImage(hModExe, MAKEINTRESOURCE(101), IMAGE_ICON, 32, 32, LR_SHARED);
	SendMessage(WindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	SendMessage(WindowHandle, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
#endif
#ifdef LIN
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom((void*)app_icon, 48, 48, 32, 192, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_SetWindowIcon(sdl_window, icon);
	SDL_FreeSurface(icon);
#endif
	atexit(SDL_Quit);

	return 0;
}

void SDLSetScreen(int scale_, bool resizable_, bool fullscreen_, bool altFullscreen_)
{
	scale = scale_;
	fullscreen = fullscreen_;
	altFullscreen = altFullscreen_;
	resizable = resizable_;
	SDL_SetWindowSize(sdl_window, WINDOWW * scale, WINDOWH * scale);
	unsigned int flags = 0;
	if (fullscreen)
		flags = altFullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDL_SetWindowFullscreen(sdl_window, flags);
	if (fullscreen)
		SDL_RaiseWindow(sdl_window);
	SDL_SetWindowResizable(sdl_window, resizable ? SDL_TRUE : SDL_FALSE);
}

unsigned int GetTicks()
{
	return SDL_GetTicks();
}

std::map<ByteString, ByteString> readArguments(int argc, char * argv[])
{
	std::map<ByteString, ByteString> arguments;

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
			arguments["scale"] = argv[i]+6;
		}
		else if (!strncmp(argv[i], "proxy:", 6))
		{
			if(argv[i]+6)
				arguments["proxy"] = argv[i]+6;
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
			arguments["open"] = argv[i+1];
			i++;
		}
		else if (!strncmp(argv[i], "ddir", 5) && i+1<argc)
		{
			arguments["ddir"] = argv[i+1];
			i++;
		}
		else if (!strncmp(argv[i], "ptsave", 7) && i+1<argc)
		{
			arguments["ptsave"] = argv[i+1];
			i++;
			break;
		}
	}
	return arguments;
}

int elapsedTime = 0, currentTime = 0, lastTime = 0, currentFrame = 0;
unsigned int lastTick = 0;
unsigned int lastFpsUpdate = 0;
float fps = 0;
ui::Engine * engine = NULL;
bool showDoubleScreenDialog = false;
float currentWidth, currentHeight;

int mousex = 0, mousey = 0;
int mouseButton = 0;
bool mouseDown = false;

bool calculatedInitialMouse = false, delay = false;
bool hasMouseMoved = false;

void EventProcess(SDL_Event event)
{
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
		engine->onTextInput(ByteString(event.text.text).FromUtf8());
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

		engine->onMouseWheel(mousex, mousey, y); // TODO: pass x?
		break;
	}
	case SDL_MOUSEMOTION:
		mousex = event.motion.x;
		mousey = event.motion.y;
		engine->onMouseMove(mousex, mousey);

		hasMouseMoved = true;
		break;
	case SDL_MOUSEBUTTONDOWN:
		// if mouse hasn't moved yet, sdl will send 0,0. We don't want that
		if (hasMouseMoved)
		{
			mousex = event.motion.x;
			mousey = event.motion.y;
		}
		mouseButton = event.button.button;
		engine->onMouseClick(event.motion.x, event.motion.y, mouseButton);

		mouseDown = true;
		SDL_CaptureMouse(SDL_TRUE);
		break;
	case SDL_MOUSEBUTTONUP:
		// if mouse hasn't moved yet, sdl will send 0,0. We don't want that
		if (hasMouseMoved)
		{
			mousex = event.motion.x;
			mousey = event.motion.y;
		}
		mouseButton = event.button.button;
		engine->onMouseUnclick(mousex, mousey, mouseButton);

		mouseDown = false;
		SDL_CaptureMouse(SDL_FALSE);
		break;
	case SDL_WINDOWEVENT:
	{
		switch (event.window.event)
		{
		case SDL_WINDOWEVENT_SHOWN:
			if (!calculatedInitialMouse)
			{
				//initial mouse coords, sdl won't tell us this if mouse hasn't moved
				CalculateMousePosition(&mousex, &mousey);
				engine->onMouseMove(mousex, mousey);
				calculatedInitialMouse = true;
			}
			break;
		// This event would be needed in certain glitchy cases of window resizing
		// But for all currently tested cases, it isn't needed
		/*case SDL_WINDOWEVENT_RESIZED:
		{
			float width = event.window.data1;
			float height = event.window.data2;

			currentWidth = width;
			currentHeight = height;
			// this "* scale" thing doesn't really work properly
			// currently there is a bug where input doesn't scale properly after resizing, only when double scale mode is active
			inputScaleH = (float)WINDOWW * scale / currentWidth;
			inputScaleV = (float)WINDOWH * scale / currentHeight;
			std::cout << "Changing input scale to " << inputScaleH << "x" << inputScaleV << std::endl;
			break;
		}*/
		// This would send a mouse up event when focus is lost
		// Not even sdl itself will know when the mouse was released if it happens in another window
		// So it will ignore the next mouse down (after tpt is re-focused) and not send any events at all
		// This is more unintuitive than pretending the mouse is still down when it's not, so this code is commented out
		/*case SDL_WINDOWEVENT_FOCUS_LOST:
			if (mouseDown)
			{
				mouseDown = false;
				engine->onMouseUnclick(mousex, mousey, mouseButton);
			}
			break;*/
		}
		break;
	}
	}
}

void DoubleScreenDialog()
{
	StringBuilder message;
	message << "Switching to double size mode since your screen was determined to be large enough: ";
	message << desktopWidth << "x" << desktopHeight << " detected, " << WINDOWW*2 << "x" << WINDOWH*2 << " required";
	message << "\nTo undo this, hit Cancel. You can toggle double size mode in settings at any time.";
	if (!ConfirmPrompt::Blocking("Large screen detected", message.Build()))
	{
		Client::Ref().SetPref("Scale", 1);
		engine->SetScale(1);
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

		if (scale != engine->Scale || fullscreen != engine->Fullscreen
				|| altFullscreen != engine->GetAltFullscreen() || resizable != engine->GetResizable())
		{
			SDLSetScreen(engine->Scale, engine->GetResizable(), engine->Fullscreen, engine->GetAltFullscreen());
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
		if (frameStart - lastFpsUpdate > 200)
		{
			engine->SetFps(1000.0 / correctedFrameTimeAvg);
			lastFpsUpdate = frameStart;
		}
		if (frameStart - lastTick > 1000)
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

void BlueScreen(String detailMessage)
{
	ui::Engine * engine = &ui::Engine::Ref();
	engine->g->fillrect(0, 0, engine->GetWidth(), engine->GetHeight(), 17, 114, 169, 210);

	String errorTitle = "ERROR";
	String errorDetails = "Details: " + detailMessage;
	String errorHelp = "An unrecoverable fault has occurred, please report the error by visiting the website below\n"
		"http://" SERVER;
	int currentY = 0, width, height;
	int errorWidth = 0;
	Graphics::textsize(errorHelp, errorWidth, height);

	engine->g->drawtext((engine->GetWidth()/2)-(errorWidth/2), ((engine->GetHeight()/2)-100) + currentY, errorTitle.c_str(), 255, 255, 255, 255);
	Graphics::textsize(errorTitle, width, height);
	currentY += height + 4;

	engine->g->drawtext((engine->GetWidth()/2)-(errorWidth/2), ((engine->GetHeight()/2)-100) + currentY, errorDetails.c_str(), 255, 255, 255, 255);
	Graphics::textsize(errorTitle, width, height);
	currentY += height + 4;

	engine->g->drawtext((engine->GetWidth()/2)-(errorWidth/2), ((engine->GetHeight()/2)-100) + currentY, errorHelp.c_str(), 255, 255, 255, 255);
	Graphics::textsize(errorTitle, width, height);
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


	std::map<ByteString, ByteString> arguments = readArguments(argc, argv);

	if(arguments["ddir"].length())
#ifdef WIN
		_chdir(arguments["ddir"].c_str());
#else
		chdir(arguments["ddir"].c_str());
#endif

	scale = Client::Ref().GetPrefInteger("Scale", 1);
	resizable = Client::Ref().GetPrefBool("Resizable", false);
	fullscreen = Client::Ref().GetPrefBool("Fullscreen", false);
	altFullscreen = Client::Ref().GetPrefBool("AltFullscreen", false);


	if(arguments["kiosk"] == "true")
	{
		fullscreen = true;
		Client::Ref().SetPref("Fullscreen", fullscreen);
	}

	if(arguments["scale"].length())
	{
		scale = arguments["scale"].ToNumber<int>();
		Client::Ref().SetPref("Scale", scale);
	}

	ByteString proxyString = "";
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
		proxyString = (Client::Ref().GetPrefByteString("Proxy", ""));
	}

	Client::Ref().Initialise(proxyString);

	// TODO: maybe bind the maximum allowed scale to screen size somehow
	if(scale < 1 || scale > 10)
		scale = 1;

	SDLOpen();
	// TODO: mabe make a nice loop that automagically finds the optimal scale
	if (Client::Ref().IsFirstRun() && desktopWidth > WINDOWW*2+30 && desktopHeight > WINDOWH*2+30)
	{
		scale = 2;
		Client::Ref().SetPref("Scale", 2);
		SDL_SetWindowSize(sdl_window, WINDOWW * 2, WINDOWH * 2);
		showDoubleScreenDialog = true;
	}
	if (!Client::Ref().IsFirstRun())
		LoadWindowPosition();

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
	ui::Engine::Ref().SetResizable(resizable);
	ui::Engine::Ref().Fullscreen = fullscreen;
	ui::Engine::Ref().SetAltFullscreen(altFullscreen);

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

#ifndef FONTEDITOR
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
					new ErrorMessage("Error", "Could not open save file:\n" + ByteString(e.what()).FromUtf8()) ;
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
			ByteString ptsaveArg = arguments["ptsave"];
			try
			{
				ByteString saveIdPart;
				if (ByteString::Split split = arguments["ptsave"].SplitBy(':'))
				{
					if (split.Before() != "ptsave")
						throw std::runtime_error("Not a ptsave link");
					saveIdPart = split.After().SplitBy('#').Before();
				}
				else
					throw std::runtime_error("Invalid save link");

				if (!saveIdPart.size())
					throw std::runtime_error("No Save ID");
#ifdef DEBUG
				std::cout << "Got Ptsave: id: " << saveIdPart << std::endl;
#endif
				int saveId = saveIdPart.ToNumber<int>();

				SaveInfo * newSave = Client::Ref().GetSave(saveId, 0);
				if (!newSave)
					throw std::runtime_error("Could not load save info");
				std::vector<unsigned char> saveData = Client::Ref().GetSaveData(saveId, 0);
				if (!saveData.size())
					throw std::runtime_error(("Could not load save\n" + Client::Ref().GetLastError()).ToUtf8());
				GameSave * newGameSave = new GameSave(saveData);
				newSave->SetGameSave(newGameSave);

				gameController->LoadSave(newSave);
				delete newSave;
			}
			catch (std::exception & e)
			{
				new ErrorMessage("Error", ByteString(e.what()).FromUtf8());
			}
		}

#else // FONTEDITOR
		if(argc <= 1)
			throw std::runtime_error("Not enough arguments");
		engine->ShowWindow(new FontEditor(argv[1]));
#endif

		EngineProcess();

		SaveWindowPosition();

#if !defined(DEBUG) && !defined(_DEBUG)
	}
	catch(exception& e)
	{
		BlueScreen(ByteString(e.what()).FromUtf8());
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
