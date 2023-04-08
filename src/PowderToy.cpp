#include "PowderToySDL.h"
#include "Format.h"
#include "X86KillDenormals.h"
#include "prefs/GlobalPrefs.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "client/SaveInfo.h"
#include "client/http/requestmanager/RequestManager.h"
#include "common/platform/Platform.h"
#include "graphics/Graphics.h"
#include "simulation/SaveRenderer.h"
#include "common/tpt-rand.h"
#include "gui/game/Favorite.h"
#include "gui/Style.h"
#include "gui/game/GameController.h"
#include "gui/game/GameView.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/interface/Engine.h"
#include "Config.h"
#include "SimulationConfig.h"
#include <optional>
#include <climits>
#include <iostream>
#include <csignal>
#include <SDL.h>

void LoadWindowPosition()
{
	if (Client::Ref().IsFirstRun())
	{
		return;
	}

	auto &prefs = GlobalPrefs::Ref();
	int savedWindowX = prefs.Get("WindowX", INT_MAX);
	int savedWindowY = prefs.Get("WindowY", INT_MAX);

	int borderTop, borderLeft;
	SDL_GetWindowBordersSize(sdl_window, &borderTop, &borderLeft, nullptr, nullptr);
	// Sometimes (Windows), the border size may not be reported for 200+ frames
	// So just have a default of 5 to ensure the window doesn't get stuck where it can't be moved
	if (borderTop == 0)
		borderTop = 5;

	int numDisplays = SDL_GetNumVideoDisplays();
	SDL_Rect displayBounds;
	bool ok = false;
	for (int i = 0; i < numDisplays; i++)
	{
		SDL_GetDisplayBounds(i, &displayBounds);
		if (savedWindowX + borderTop > displayBounds.x && savedWindowY + borderLeft > displayBounds.y &&
				savedWindowX + borderTop < displayBounds.x + displayBounds.w &&
				savedWindowY + borderLeft < displayBounds.y + displayBounds.h)
		{
			ok = true;
			break;
		}
	}
	if (ok)
		SDL_SetWindowPosition(sdl_window, savedWindowX + borderLeft, savedWindowY + borderTop);
}

void SaveWindowPosition()
{
	int x, y;
	SDL_GetWindowPosition(sdl_window, &x, &y);

	int borderTop, borderLeft;
	SDL_GetWindowBordersSize(sdl_window, &borderTop, &borderLeft, nullptr, nullptr);

	auto &prefs = GlobalPrefs::Ref();
	prefs.Set("WindowX", x - borderLeft);
	prefs.Set("WindowY", y - borderTop);
}

void LargeScreenDialog()
{
	StringBuilder message;
	message << "Switching to " << scale << "x size mode since your screen was determined to be large enough: ";
	message << desktopWidth << "x" << desktopHeight << " detected, " << WINDOWW*scale << "x" << WINDOWH*scale << " required";
	message << "\nTo undo this, hit Cancel. You can change this in settings at any time.";
	if (!ConfirmPrompt::Blocking("Large screen detected", message.Build()))
	{
		GlobalPrefs::Ref().Set("Scale", 1);
		ui::Engine::Ref().SetScale(1);
	}
}

void TickClient()
{
	Client::Ref().Tick();
}

void BlueScreen(String detailMessage)
{
	auto &engine = ui::Engine::Ref();
	engine.g->BlendFilledRect(engine.g->Size().OriginRect(), 0x1172A9_rgb .WithAlpha(0xD2));

	String errorTitle = "ERROR";
	String errorDetails = "Details: " + detailMessage;
	String errorHelp = String("An unrecoverable fault has occurred, please report the error by visiting the website below\n") + SCHEME + SERVER;

	// We use the width of errorHelp to center, but heights of the individual texts for vertical spacing
	auto pos = engine.g->Size() / 2 - Vec2(Graphics::TextSize(errorHelp).X / 2, 100);
	engine.g->BlendText(pos, errorTitle, 0xFFFFFF_rgb .WithAlpha(0xFF));
	pos.Y += 4 + Graphics::TextSize(errorTitle).Y;
	engine.g->BlendText(pos, errorDetails, 0xFFFFFF_rgb .WithAlpha(0xFF));
	pos.Y += 4 + Graphics::TextSize(errorDetails).Y;
	engine.g->BlendText(pos, errorHelp, 0xFFFFFF_rgb .WithAlpha(0xFF));

	//Death loop
	SDL_Event event;
	while(true)
	{
		while (SDL_PollEvent(&event))
			if(event.type == SDL_QUIT)
				exit(-1); // Don't use Platform::Exit, we're practically zombies at this point anyway.
		blit(engine.g->Data());
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

constexpr int SCALE_MAXIMUM = 10;
constexpr int SCALE_MARGIN = 30;

int GuessBestScale()
{
	const int widthNoMargin = desktopWidth - SCALE_MARGIN;
	const int widthGuess = widthNoMargin / WINDOWW;

	const int heightNoMargin = desktopHeight - SCALE_MARGIN;
	const int heightGuess = heightNoMargin / WINDOWH;

	int guess = std::min(widthGuess, heightGuess);
	if(guess < 1 || guess > SCALE_MAXIMUM)
		guess = 1;

	return guess;
}

struct ExplicitSingletons
{
	// These need to be listed in the order they are populated in main.
	std::unique_ptr<GlobalPrefs> globalPrefs;
	http::RequestManagerPtr requestManager;
	std::unique_ptr<Client> client;
	std::unique_ptr<SaveRenderer> saveRenderer;
	std::unique_ptr<Favorite> favorite;
	std::unique_ptr<ui::Engine> engine;
	std::unique_ptr<GameController> gameController;
};
static std::unique_ptr<ExplicitSingletons> explicitSingletons;

int main(int argc, char * argv[])
{
	Platform::SetupCrt();
	Platform::Atexit([]() {
		SDLClose();
		explicitSingletons.reset();
	});
	explicitSingletons = std::make_unique<ExplicitSingletons>();


	// https://bugzilla.libsdl.org/show_bug.cgi?id=3796
	if (SDL_Init(0) < 0)
	{
		fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	Platform::originalCwd = Platform::GetCwd();

	using Argument = std::optional<ByteString>;
	std::map<ByteString, Argument> arguments;

	for (auto i = 1; i < argc; ++i)
	{
		auto str = ByteString(argv[i]);
		if (str.BeginsWith("file://"))
		{
			arguments.insert({ "open", format::URLDecode(str.substr(7 /* length of the "file://" prefix */)) });
		}
		else if (str.BeginsWith("ptsave:"))
		{
			arguments.insert({ "ptsave", str });
		}
		else if (auto split = str.SplitBy(':'))
		{
			arguments.insert({ split.Before(), split.After() });
		}
		else if (auto split = str.SplitBy('='))
		{
			arguments.insert({ split.Before(), split.After() });
		}
		else if (str == "open" || str == "ptsave" || str == "ddir")
		{
			if (i + 1 < argc)
			{
				arguments.insert({ str, argv[i + 1] });
				i += 1;
			}
			else
			{
				std::cerr << "no value provided for command line parameter " << str << std::endl;
			}
		}
		else
		{
			arguments.insert({ str, "" }); // so .has_value() is true
		}
	}

	auto ddirArg = arguments["ddir"];
	if (ddirArg.has_value())
	{
		if (Platform::ChangeDir(ddirArg.value()))
			Platform::sharedCwd = Platform::GetCwd();
		else
			perror("failed to chdir to requested ddir");
	}
	else
	{
		char *ddir = SDL_GetPrefPath(NULL, APPDATA);
		if (!Platform::FileExists("powder.pref"))
		{
			if (ddir)
			{
				if (!Platform::ChangeDir(ddir))
				{
					perror("failed to chdir to default ddir");
					SDL_free(ddir);
					ddir = nullptr;
				}
			}
		}

		if (ddir)
		{
			Platform::sharedCwd = ddir;
			SDL_free(ddir);
		}
	}
	// We're now in the correct directory, time to get prefs.
	explicitSingletons->globalPrefs = std::make_unique<GlobalPrefs>();

	auto &prefs = GlobalPrefs::Ref();
	scale = prefs.Get("Scale", 1);
	resizable = prefs.Get("Resizable", false);
	fullscreen = prefs.Get("Fullscreen", false);
	altFullscreen = prefs.Get("AltFullscreen", false);
	forceIntegerScaling = prefs.Get("ForceIntegerScaling", true);
	momentumScroll = prefs.Get("MomentumScroll", true);
	showAvatars = prefs.Get("ShowAvatars", true);

	auto true_string = [](ByteString str) {
		str = str.ToLower();
		return str == "true" ||
		       str == "t" ||
		       str == "on" ||
		       str == "yes" ||
		       str == "y" ||
		       str == "1" ||
		       str == ""; // standalone "redirect" or "disable-bluescreen" or similar arguments
	};
	auto true_arg = [&true_string](Argument arg) {
		return arg.has_value() && true_string(arg.value());
	};

	auto kioskArg = arguments["kiosk"];
	if (kioskArg.has_value())
	{
		fullscreen = true_string(kioskArg.value());
		prefs.Set("Fullscreen", fullscreen);
	}

	if (true_arg(arguments["redirect"]))
	{
		FILE *new_stdout = freopen("stdout.log", "w", stdout);
		FILE *new_stderr = freopen("stderr.log", "w", stderr);
		if (!new_stdout || !new_stderr)
		{
			Platform::Exit(42);
		}
	}

	auto scaleArg = arguments["scale"];
	if (scaleArg.has_value())
	{
		try
		{
			scale = scaleArg.value().ToNumber<int>();
			prefs.Set("Scale", scale);
		}
		catch (const std::runtime_error &e)
		{
			std::cerr << "failed to set scale: " << e.what() << std::endl;
		}
	}

	auto clientConfig = [&prefs](Argument arg, ByteString name, ByteString defaultValue) {
		ByteString value;
		if (arg.has_value())
		{
			value = arg.value();
			if (value == "")
			{
				value = defaultValue;
			}
			prefs.Set(name, value);
		}
		else
		{
			value = prefs.Get(name, defaultValue);
		}
		return value;
	};
	ByteString proxyString = clientConfig(arguments["proxy"], "Proxy", "");
	ByteString cafileString = clientConfig(arguments["cafile"], "CAFile", "");
	ByteString capathString = clientConfig(arguments["capath"], "CAPath", "");
	bool disableNetwork = true_arg(arguments["disable-network"]);
	explicitSingletons->requestManager = http::RequestManager::Create(proxyString, cafileString, capathString, disableNetwork);

	explicitSingletons->client = std::make_unique<Client>();
	Client::Ref().Initialize();

	explicitSingletons->saveRenderer = std::make_unique<SaveRenderer>();
	explicitSingletons->favorite = std::make_unique<Favorite>();
	explicitSingletons->engine = std::make_unique<ui::Engine>();

	// TODO: maybe bind the maximum allowed scale to screen size somehow
	if(scale < 1 || scale > SCALE_MAXIMUM)
		scale = 1;

	SDLOpen();

	if (Client::Ref().IsFirstRun())
	{
		scale = GuessBestScale();
		if (scale > 1)
		{
			prefs.Set("Scale", scale);
			SDL_SetWindowSize(sdl_window, WINDOWW * scale, WINDOWH * scale);
			showLargeScreenDialog = true;
		}
	}

	StopTextInput();

	auto &engine = ui::Engine::Ref();
	engine.g = new Graphics();
	engine.Scale = scale;
	engine.SetResizable(resizable);
	engine.Fullscreen = fullscreen;
	engine.SetAltFullscreen(altFullscreen);
	engine.SetForceIntegerScaling(forceIntegerScaling);
	engine.MomentumScroll = momentumScroll;
	engine.ShowAvatars = showAvatars;
	engine.Begin();
	engine.SetFastQuit(prefs.Get("FastQuit", true));

	bool enableBluescreen = !DEBUG && !true_arg(arguments["disable-bluescreen"]);
	if (enableBluescreen)
	{
		//Get ready to catch any dodgy errors
		signal(SIGSEGV, SigHandler);
		signal(SIGFPE, SigHandler);
		signal(SIGILL, SigHandler);
		signal(SIGABRT, SigHandler);
	}

	if constexpr (X86)
	{
		X86KillDenormals();
	}

	auto wrapWithBluescreen = [&]() {
		explicitSingletons->gameController = std::make_unique<GameController>();
		auto *gameController = explicitSingletons->gameController.get();
		engine.ShowWindow(gameController->GetView());

		auto openArg = arguments["open"];
		if (openArg.has_value())
		{
			if constexpr (DEBUG)
			{
				std::cout << "Loading " << openArg.value() << std::endl;
			}
			if (Platform::FileExists(openArg.value()))
			{
				try
				{
					std::vector<char> gameSaveData;
					if (!Platform::ReadFile(gameSaveData, openArg.value()))
					{
						new ErrorMessage("Error", "Could not read file");
					}
					else
					{
						SaveFile * newFile = new SaveFile(openArg.value());
						GameSave * newSave = new GameSave(std::move(gameSaveData));
						newFile->SetGameSave(newSave);
						gameController->LoadSaveFile(newFile);
						delete newFile;
					}

				}
				catch (std::exception & e)
				{
					new ErrorMessage("Error", "Could not open save file:\n" + ByteString(e.what()).FromUtf8()) ;
				}
			}
			else
			{
				new ErrorMessage("Error", "Could not open file");
			}
		}

		auto ptsaveArg = arguments["ptsave"];
		if (ptsaveArg.has_value())
		{
			engine.g->Clear();
			engine.g->DrawRect(RectSized(engine.g->Size() / 2 - Vec2(100, 25), Vec2(200, 50)), 0xB4B4B4_rgb);
			String loadingText = "Loading save...";
			engine.g->BlendText(engine.g->Size() / 2 - Vec2(Graphics::textwidth(loadingText) / 2, 5), loadingText, style::Colour::InformationTitle);

			blit(engine.g->Data());
			try
			{
				ByteString saveIdPart;
				if (ByteString::Split split = ptsaveArg.value().SplitBy(':'))
				{
					if (split.Before() != "ptsave")
						throw std::runtime_error("Not a ptsave link");
					saveIdPart = split.After().SplitBy('#').Before();
				}
				else
					throw std::runtime_error("Invalid save link");

				if (!saveIdPart.size())
					throw std::runtime_error("No Save ID");
				if constexpr (DEBUG)
				{
					std::cout << "Got Ptsave: id: " << saveIdPart << std::endl;
				}
				int saveId = saveIdPart.ToNumber<int>();

				SaveInfo * newSave = Client::Ref().GetSave(saveId, 0);
				if (!newSave)
					throw std::runtime_error("Could not load save info");
				auto saveData = Client::Ref().GetSaveData(saveId, 0);
				if (!saveData.size())
					throw std::runtime_error(("Could not load save\n" + Client::Ref().GetLastError()).ToUtf8());
				GameSave * newGameSave = new GameSave(std::move(saveData));
				newSave->SetGameSave(newGameSave);

				gameController->LoadSave(newSave);
				delete newSave;
			}
			catch (std::exception & e)
			{
				new ErrorMessage("Error", ByteString(e.what()).FromUtf8());
			}
		}

		EngineProcess();
		SaveWindowPosition();
	};

	if (enableBluescreen)
	{
		try
		{
			wrapWithBluescreen();
		}
		catch (const std::exception &e)
		{
			BlueScreen(ByteString(e.what()).FromUtf8());
		}
	}
	else
	{
		wrapWithBluescreen();
	}
	Platform::Exit(0);
	return 0;
}
