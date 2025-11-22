#include "GameController.h"

#include "Brush.h"
#include "Controller.h"
#include "Format.h"
#include "GameModel.h"
#include "GameModelException.h"
#include "GameView.h"
#include "Menu.h"
#include "Notification.h"
#include "QuickOptions.h"
#include "RenderPreset.h"
#include "tool/PropertyTool.h"
#include "tool/GOLTool.h"

#include "GameControllerEvents.h"
#include "lua/CommandInterface.h"

#include "prefs/GlobalPrefs.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "common/platform/Platform.h"
#include "debug/DebugInfo.h"
#include "debug/DebugLines.h"
#include "debug/DebugParts.h"
#include "debug/ElementPopulation.h"
#include "debug/ParticleDebug.h"
#include "debug/SurfaceNormals.h"
#include "debug/AirVelocity.h"
#include "graphics/Renderer.h"
#include "simulation/Air.h"
#include "simulation/ElementClasses.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationData.h"
#include "simulation/Snapshot.h"
#include "simulation/elements/STKM.h"

#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Engine.h"

#include "gui/colourpicker/ColourPickerActivity.h"
#include "gui/elementsearch/ElementSearchActivity.h"
#include "gui/filebrowser/FileBrowserActivity.h"
#include "gui/profile/ProfileActivity.h"
#include "gui/save/LocalSaveActivity.h"
#include "gui/save/ServerSaveActivity.h"
#include "gui/update/UpdateActivity.h"

#include "gui/console/ConsoleController.h"
#include "gui/console/ConsoleView.h"
#include "gui/localbrowser/LocalBrowserController.h"
#include "gui/localbrowser/LocalBrowserView.h"
#include "gui/login/LoginController.h"
#include "gui/login/LoginView.h"
#include "gui/options/OptionsController.h"
#include "gui/options/OptionsView.h"
#include "gui/preview/PreviewController.h"
#include "gui/preview/PreviewView.h"
#include "gui/render/RenderController.h"
#include "gui/render/RenderView.h"
#include "gui/search/SearchController.h"
#include "gui/search/SearchView.h"
#include "gui/tags/TagsController.h"
#include "gui/tags/TagsView.h"

#include "Config.h"
#include <SDL.h>
#include <iostream>

GameController::GameController():
	firstTick(true),
	foundSignID(-1),
	activePreview(nullptr),
	search(nullptr),
	renderOptions(nullptr),
	loginWindow(nullptr),
	console(nullptr),
	tagsWindow(nullptr),
	localBrowser(nullptr),
	options(nullptr),
	debugFlags(0),
	HasDone(false)
{
	gameView = new GameView();
	gameModel = new GameModel(gameView); // mvc is a joke
	gameModel->BuildQuickOptionMenu(this);

	gameView->AttachController(this);
	gameModel->AddObserver(gameView);

	gameView->SetDebugHUD(GlobalPrefs::Ref().Get("Renderer.DebugMode", false));

	commandInterface = CommandInterface::Create(this, gameModel);

	Client::Ref().AddListener(this);

	debugInfo.push_back(std::make_unique<DebugParts            >(DEBUG_PARTS     , gameModel->GetSimulation()));
	debugInfo.push_back(std::make_unique<ElementPopulationDebug>(DEBUG_ELEMENTPOP, gameModel->GetSimulation()));
	debugInfo.push_back(std::make_unique<DebugLines            >(DEBUG_LINES     , gameView, this));
	debugInfo.push_back(std::make_unique<ParticleDebug         >(DEBUG_PARTICLE  , gameModel->GetSimulation(), gameModel));
	debugInfo.push_back(std::make_unique<SurfaceNormals        >(DEBUG_SURFNORM  , gameModel->GetSimulation(), gameView, this));
	debugInfo.push_back(std::make_unique<AirVelocity           >(DEBUG_AIRVEL    , gameModel->GetSimulation(), gameView, this));
}

GameController::~GameController()
{
	if(search)
	{
		delete search;
	}
	if(renderOptions)
	{
		delete renderOptions;
	}
	if(loginWindow)
	{
		delete loginWindow;
	}
	if(tagsWindow)
	{
		delete tagsWindow;
	}
	if(console)
	{
		delete console;
	}
	if(activePreview)
	{
		delete activePreview;
	}
	if(localBrowser)
	{
		delete localBrowser;
	}
	if (options)
	{
		delete options;
	}
	debugInfo.clear();
	std::vector<QuickOption*> quickOptions = gameModel->GetQuickOptions();
	for(std::vector<QuickOption*>::iterator iter = quickOptions.begin(), end = quickOptions.end(); iter != end; ++iter)
	{
		delete *iter;
	}
	std::vector<Notification*> notifications = gameModel->GetNotifications();
	for(std::vector<Notification*>::iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
	{
		delete *iter;
	}
	gameView->PauseRendererThread();
	commandInterface->RemoveComponents();
	gameView->CloseActiveWindow();
	delete gameView;
	commandInterface.reset();
	delete gameModel;
}

bool GameController::HistoryRestore()
{
	if (!gameModel->HistoryCanRestore())
	{
		return false;
	}
	// * When undoing for the first time since the last call to HistorySnapshot, save the current state.
	//   Ctrl+Y needs this in order to bring you back to the point right before your last Ctrl+Z, because
	//   the last history entry is what this Ctrl+Z brings you back to, not the current state.
	if (!beforeRestore)
	{
		beforeRestore = gameModel->GetSimulation()->CreateSnapshot();
		beforeRestore->Authors = Client::Ref().GetAuthorInfo();
	}
	gameModel->HistoryRestore();
	auto &current = *gameModel->HistoryCurrent();
	gameModel->GetSimulation()->Restore(current);
	Client::Ref().OverwriteAuthorInfo(current.Authors);

	return true;
}

void GameController::HistorySnapshot()
{
	// * Calling HistorySnapshot means the user decided to use the current state and
	//   forfeit the option to go back to whatever they Ctrl+Z'd their way back from.
	beforeRestore.reset();
	gameModel->HistoryPush(gameModel->GetSimulation()->CreateSnapshot());
}

bool GameController::HistoryForward()
{
	if (!gameModel->HistoryCanForward())
	{
		return false;
	}
	gameModel->HistoryForward();
	// * If gameModel has nothing more to give, we've Ctrl+Y'd our way back to the original
	//   state; restore this instead, then get rid of it.
	auto &current = gameModel->HistoryCurrent() ? *gameModel->HistoryCurrent() : *beforeRestore;
	gameModel->GetSimulation()->Restore(current);
	Client::Ref().OverwriteAuthorInfo(current.Authors);
	if (&current == beforeRestore.get())
	{
		beforeRestore.reset();
	}

	return true;
}

GameView * GameController::GetView()
{
	return gameView;
}

int GameController::GetSignAt(int x, int y)
{
	Simulation * sim = gameModel->GetSimulation();
	for (int i = sim->signs.size()-1; i >= 0; i--)
	{
		int signx, signy, signw, signh;
		sim->signs[i].getDisplayText(sim, signx, signy, signw, signh);
		if (x>=signx && x<=signx+signw && y>=signy && y<=signy+signh)
			return i;
	}
	return -1;
}

// assumed to already be a valid sign
String GameController::GetSignText(int signID)
{
	return gameModel->GetSimulation()->signs[signID].text;
}

std::pair<int, sign::Type> GameController::GetSignSplit(int signID)
{
	return gameModel->GetSimulation()->signs[signID].split();
}

void GameController::PlaceSave(ui::Point position)
{
	auto *placeSave = gameModel->GetTransformedPlaceSave();
	if (placeSave)
	{
		HistorySnapshot();
		gameModel->GetSimulation()->Load(placeSave, !gameView->ShiftBehaviour(), position);
		gameModel->SetPaused(placeSave->paused | gameModel->GetPaused());
		Client::Ref().MergeStampAuthorInfo(placeSave->authors);
	}
	gameModel->SetPlaceSave(nullptr);
}

void GameController::Install()
{
	if constexpr (CAN_INSTALL)
	{
		new ConfirmPrompt("Install " + String(APPNAME), "Do you wish to install " + String(APPNAME) + " on this computer?\nThis allows you to open save files and saves directly from the website.", { [] {
			if (Platform::Install())
			{
				new InformationMessage("Success", "Installation completed", false);
			}
			else
			{
				new ErrorMessage("Could not install", "The installation did not complete due to an error");
			}
		} });
	}
	else
	{
		new InformationMessage("No installation necessary", "You don't need to install " + String(APPNAME) + " on this platform", false);
	}
}

void GameController::AdjustGridSize(int direction)
{
	if(direction > 0)
		gameModel->GetRendererSettings().gridSize = (gameModel->GetRendererSettings().gridSize+1)%10;
	else
		gameModel->GetRendererSettings().gridSize = (gameModel->GetRendererSettings().gridSize+9)%10;
}

void GameController::InvertAirSim()
{
	gameModel->GetSimulation()->air->Invert();
}


void GameController::AdjustBrushSize(int delta, bool logarithmic, bool keepX, bool keepY)
{
	gameModel->GetBrush().AdjustSize(delta, logarithmic, keepX, keepY);
}

void GameController::SetBrushSize(ui::Point newSize)
{
	gameModel->GetBrush().SetRadius(newSize);
}

void GameController::AdjustZoomSize(int delta, bool logarithmic)
{
	int newSize;
	if(logarithmic)
		newSize = gameModel->GetZoomSize() + std::max(gameModel->GetZoomSize() / 10, 1) * delta;
	else
		newSize = gameModel->GetZoomSize() + delta;
	if(newSize<5)
			newSize = 5;
	if(newSize>64)
			newSize = 64;
	gameModel->SetZoomSize(newSize);

	int newZoomFactor = 256/newSize;
	if(newZoomFactor<3)
		newZoomFactor = 3;
	gameModel->SetZoomFactor(newZoomFactor);
}

bool GameController::MouseInZoom(ui::Point position)
{
	if(position.X >= XRES)
		position.X = XRES-1;
	if(position.Y >= YRES)
		position.Y = YRES-1;
	if(position.Y < 0)
		position.Y = 0;
	if(position.X < 0)
		position.X = 0;

	return gameModel->MouseInZoom(position);
}

ui::Point GameController::PointTranslate(ui::Point point)
{
	if(point.X >= XRES)
		point.X = XRES-1;
	if(point.Y >= YRES)
		point.Y = YRES-1;
	if(point.Y < 0)
		point.Y = 0;
	if(point.X < 0)
		point.X = 0;

	return gameModel->AdjustZoomCoords(point);
}

ui::Point GameController::NormaliseBlockCoord(ui::Point point)
{
	return (point/CELL)*CELL;
}

void GameController::DrawRect(int toolSelection, ui::Point point1, ui::Point point2)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	gameModel->SetLastTool(activeTool);
	Brush &cBrush = gameModel->GetBrush();
	if (!activeTool)
		return;
	activeTool->Strength = 1.0f;
	activeTool->DrawRect(sim, cBrush, point1, point2);
}

void GameController::DrawLine(int toolSelection, ui::Point point1, ui::Point point2)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	gameModel->SetLastTool(activeTool);
	Brush &cBrush = gameModel->GetBrush();
	if (!activeTool)
		return;
	activeTool->Strength = 1.0f;
	activeTool->DrawLine(sim, cBrush, point1, point2, false);
}

void GameController::DrawFill(int toolSelection, ui::Point point)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	gameModel->SetLastTool(activeTool);
	Brush &cBrush = gameModel->GetBrush();
	if (!activeTool)
		return;
	activeTool->Strength = 1.0f;
	activeTool->DrawFill(sim, cBrush, point);
}

void GameController::DrawPoints(int toolSelection, ui::Point oldPos, ui::Point newPos, bool held)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	gameModel->SetLastTool(activeTool);
	Brush &cBrush = gameModel->GetBrush();
	if (!activeTool)
	{
		return;
	}

	activeTool->Strength = gameModel->GetToolStrength();
	// This is a joke, the game mvc has to go >_>
	activeTool->shiftBehaviour = gameView->ShiftBehaviour();
	activeTool->ctrlBehaviour = gameView->CtrlBehaviour();
	activeTool->altBehaviour = gameView->AltBehaviour();
	if (!held)
		activeTool->Draw(sim, cBrush, newPos);
	else
		activeTool->DrawLine(sim, cBrush, oldPos, newPos, true);
}

bool GameController::LoadClipboard()
{
	auto *clip = gameModel->GetClipboard();
	if (!clip)
		return false;
	gameModel->SetPlaceSave(std::make_unique<GameSave>(*clip));
	return true;
}

void GameController::LoadStamp(std::unique_ptr<GameSave> stamp)
{
	gameModel->SetPlaceSave(std::move(stamp));
}

void GameController::TransformPlaceSave(Mat2<int> transform, Vec2<int> nudge)
{
	gameModel->TransformPlaceSave(transform, nudge);
}

void GameController::ToolClick(int toolSelection, ui::Point point)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	Brush &cBrush = gameModel->GetBrush();
	if (!activeTool)
		return;
	activeTool->Click(sim, cBrush, point);
}

void GameController::ToolDrag(int toolSelection, ui::Point point1, ui::Point point2)
{
	Simulation * sim = gameModel->GetSimulation();
	Tool * activeTool = gameModel->GetActiveTool(toolSelection);
	Brush &cBrush = gameModel->GetBrush();
	if (!activeTool)
		return;
	activeTool->Drag(sim, cBrush, point1, point2);
}

static Rect<int> SaneSaveRect(Vec2<int> point1, Vec2<int> point2)
{
	point1 = point1.Clamp(RES.OriginRect());
	point2 = point2.Clamp(RES.OriginRect());
	auto tlx = std::min(point1.X, point2.X);
	auto tly = std::min(point1.Y, point2.Y);
	auto brx = std::max(point1.X, point2.X);
	auto bry = std::max(point1.Y, point2.Y);
	return RectBetween(Vec2{ tlx, tly }, Vec2{ brx, bry });
}

ByteString GameController::StampRegion(ui::Point point1, ui::Point point2)
{
	return StampRegion(point1, point2, gameModel->GetIncludePressure() != gameView->ShiftBehaviour());
}

ByteString GameController::StampRegion(ui::Point point1, ui::Point point2, bool includePressure)
{
	auto newSave = gameModel->GetSimulation()->Save(includePressure, SaneSaveRect(point1, point2));
	if(newSave)
	{
		newSave->paused = gameModel->GetPaused();
		ByteString stampName = Client::Ref().AddStamp(std::move(newSave));
		if (stampName.length() == 0)
			new ErrorMessage("Could not create stamp", "Error serializing save file");
		return stampName;
	}
	else
	{
		new ErrorMessage("Could not create stamp", "Error generating save file");
		return "";
	}
}

void GameController::CopyRegion(ui::Point point1, ui::Point point2)
{
	auto newSave = gameModel->GetSimulation()->Save(gameModel->GetIncludePressure() != gameView->ShiftBehaviour(), SaneSaveRect(point1, point2));
	if(newSave)
	{
		Bson clipboardInfo;
		clipboardInfo["type"] = "clipboard";
		auto user = Client::Ref().GetAuthUser();
		clipboardInfo["username"] = user ? user->Username : ByteString("");
		clipboardInfo["date"] = int64_t(time(nullptr));
		Client::Ref().SaveAuthorInfo(clipboardInfo);
		newSave->authors = clipboardInfo;

		newSave->paused = gameModel->GetPaused();
		gameModel->SetClipboard(std::move(newSave));
	}
}

void GameController::CutRegion(ui::Point point1, ui::Point point2)
{
	CopyRegion(point1, point2);
	HistorySnapshot();
	gameModel->GetSimulation()->clear_area(point1.X, point1.Y, point2.X-point1.X, point2.Y-point1.Y);
}

bool GameController::MouseMove(int x, int y, int dx, int dy)
{
	return commandInterface->HandleEvent(MouseMoveEvent{ x, y, dx, dy });
}

bool GameController::MouseDown(int x, int y, unsigned button)
{
	bool ret = commandInterface->HandleEvent(MouseDownEvent{ x, y, button });
	if (ret && y<YRES && x<XRES && !gameView->GetPlacingSave() && !gameView->GetPlacingZoom())
	{
		ui::Point point = gameModel->AdjustZoomCoords(ui::Point(x, y));
		x = point.X;
		y = point.Y;
		if (!gameModel->GetActiveTool(0) || gameModel->GetActiveTool(0)->Identifier != "DEFAULT_UI_SIGN" || button != SDL_BUTTON_LEFT) //If it's not a sign tool or you are right/middle clicking
		{
			foundSignID = GetSignAt(x, y);
			if (foundSignID != -1)
			{
				if (gameModel->GetSimulation()->signs[foundSignID].split().first)
				{
					return false;
				}
			}
		}
	}
	return ret;
}

bool GameController::MouseUp(int x, int y, unsigned button, MouseupReason reason)
{
	bool ret = commandInterface->HandleEvent(MouseUpEvent{ x, y, button, reason });
	if (reason != mouseUpNormal)
		return ret;
	if (ret && foundSignID != -1 && y<YRES && x<XRES && !gameView->GetPlacingSave())
	{
		ui::Point point = gameModel->AdjustZoomCoords(ui::Point(x, y));
		x = point.X;
		y = point.Y;
		if (!gameModel->GetActiveTool(0) || gameModel->GetActiveTool(0)->Identifier != "DEFAULT_UI_SIGN" || button != SDL_BUTTON_LEFT) //If it's not a sign tool or you are right/middle clicking
		{
			int foundSignID = GetSignAt(x, y);
			if (foundSignID != -1)
			{
				sign &foundSign = gameModel->GetSimulation()->signs[foundSignID];
				String str = foundSign.text;
				auto si = gameModel->GetSimulation()->signs[foundSignID].split();
				if (si.first)
				{
					ret = false;
					switch (si.second)
					{
					case sign::Type::Save:
						{
							int saveID = str.Substr(3, si.first - 3).ToNumber<int>(true);
							if (saveID)
								OpenSavePreview(saveID, 0, savePreviewNormal);
						}
						break;
					case sign::Type::Thread:
						Platform::OpenURI(ByteString::Build(SERVER, "/Discussions/Thread/View.html?Thread=", str.Substr(3, si.first - 3).ToUtf8()));
						break;
					case sign::Type::Search:
						OpenSearch(str.Substr(3, si.first - 3));
						break;
					case sign::Type::Button:
						gameModel->GetSimulation()->create_part(-1, foundSign.x, foundSign.y, PT_SPRK);
						break;
					default: break;
					}
				}
			}
		}
	}
	foundSignID = -1;
	return ret;
}

bool GameController::MouseWheel(int x, int y, int d)
{
	return commandInterface->HandleEvent(MouseWheelEvent{ x, y, d });
}

bool GameController::TextInput(String text)
{
	return commandInterface->HandleEvent(TextInputEvent{ text });
}

bool GameController::TextEditing(String text)
{
	return commandInterface->HandleEvent(TextEditingEvent{ text });
}

bool GameController::KeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	bool ret = commandInterface->HandleEvent(KeyPressEvent{ { key, scan, repeat, shift, ctrl, alt } });
	if (repeat)
		return ret;
	if (ret)
	{
		Simulation * sim = gameModel->GetSimulation();
		if (!gameView->GetPlacingSave())
		{
			// Go right command
			if (key == SDLK_RIGHT)
			{
				sim->player.comm = (int)(sim->player.comm)|0x02;
			}
			// Go left command
			else if (key == SDLK_LEFT)
			{
				sim->player.comm = (int)(sim->player.comm)|0x01;
			}
			// Use element command
			else if (key == SDLK_DOWN && ((int)(sim->player.comm)&0x08)!=0x08)
			{
				sim->player.comm = (int)(sim->player.comm)|0x08;
			}
			// Jump command
			else if (key == SDLK_UP && ((int)(sim->player.comm)&0x04)!=0x04)
			{
				sim->player.comm = (int)(sim->player.comm)|0x04;
			}
		}

		// Go right command
		if (scan == SDL_SCANCODE_D)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x02;
		}
		// Go left command
		else if (scan == SDL_SCANCODE_A)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x01;
		}
		// Use element command
		else if (scan == SDL_SCANCODE_S && ((int)(sim->player2.comm)&0x08)!=0x08)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x08;
		}
		// Jump command
		else if (scan == SDL_SCANCODE_W && ((int)(sim->player2.comm)&0x04)!=0x04)
		{
			sim->player2.comm = (int)(sim->player2.comm)|0x04;
		}

		if (!sim->elementCount[PT_STKM2] || ctrl)
		{
			switch(scan)
			{
			case SDL_SCANCODE_W:
				SwitchGravity();
				break;
			case SDL_SCANCODE_D:
				gameView->SetDebugHUD(!gameView->GetDebugHUD());
				break;
			case SDL_SCANCODE_S:
				gameView->BeginStampSelection();
				break;
			}
		}

		for (auto &debug : debugInfo)
		{
			if (debug->debugID & debugFlags)
			{
				if (!debug->KeyPress(key, scan, shift, ctrl, alt, gameView->GetMousePosition()))
				{
					ret = false;
				}
			}
		}
	}
	return ret;
}

bool GameController::KeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	bool ret = commandInterface->HandleEvent(KeyReleaseEvent{ { key, scan, repeat, shift, ctrl, alt } });
	if (repeat)
		return ret;
	if (ret)
	{
		Simulation * sim = gameModel->GetSimulation();
		if (key == SDLK_RIGHT || key == SDLK_LEFT)
		{
			sim->player.pcomm = sim->player.comm;  //Saving last movement
			sim->player.comm = (int)(sim->player.comm)&12;  //Stop command
		}
		else if (key == SDLK_UP)
		{
			sim->player.comm = (int)(sim->player.comm)&11;
		}
		else if (key == SDLK_DOWN)
		{
			sim->player.comm = (int)(sim->player.comm)&7;
		}

		if (scan == SDL_SCANCODE_D || scan == SDL_SCANCODE_A)
		{
			sim->player2.pcomm = sim->player2.comm;  //Saving last movement
			sim->player2.comm = (int)(sim->player2.comm)&12;  //Stop command
		}
		else if (scan == SDL_SCANCODE_W)
		{
			sim->player2.comm = (int)(sim->player2.comm)&11;
		}
		else if (scan == SDL_SCANCODE_S)
		{
			sim->player2.comm = (int)(sim->player2.comm)&7;
		}
	}
	return ret;
}

void GameController::InitCommandInterface()
{
	commandInterface->Init();
}

void GameController::Tick()
{
	gameModel->Tick();
	if(firstTick)
	{
		if constexpr (INSTALL_CHECK)
		{
			if (Client::Ref().IsFirstRun())
			{
				Install();
			}
		}
		firstTick = false;
	}
	if (gameModel->SelectNextIdentifier.length())
	{
		gameModel->BuildMenus();
		gameModel->SetActiveTool(gameModel->SelectNextTool, gameModel->GetToolFromIdentifier(gameModel->SelectNextIdentifier));
		gameModel->SelectNextIdentifier.clear();
	}
	for (auto &debug : debugInfo)
	{
		if (debug->debugID & debugFlags)
		{
			debug->Draw();
		}
	}
	commandInterface->OnTick();
}

void GameController::Blur()
{
	// Tell lua that mouse is up (even if it really isn't)
	MouseUp(0, 0, 0, mouseUpBlur);
	commandInterface->HandleEvent(BlurEvent{});
}

void GameController::Exit()
{
	commandInterface->HandleEvent(CloseEvent{});
	gameView->CloseActiveWindow();
	HasDone = true;
}

void GameController::ResetAir()
{
	Simulation * sim = gameModel->GetSimulation();
	sim->air->Clear();
	for (int i = 0; i < NPART; i++)
	{
		if (GameSave::PressureInTmp3(sim->parts[i].type))
		{
			sim->parts[i].tmp3 = 0;
		}
	}
}

void GameController::ResetSpark()
{
	auto &sd = SimulationData::CRef();
	Simulation * sim = gameModel->GetSimulation();
	for (int i = 0; i < NPART; i++)
	{
		if (sim->parts[i].type == PT_SPRK)
		{
			if (sim->parts[i].ctype >= 0 && sim->parts[i].ctype < PT_NUM && sd.elements[sim->parts[i].ctype].Enabled)
			{
				sim->parts[i].type = sim->parts[i].ctype;
				sim->parts[i].ctype = sim->parts[i].life = 0;
			}
			else
			{
				sim->kill_part(i);
			}
		}
		else if (sim->parts[i].type == PT_WIRE)
		{
			sim->parts[i].ctype = sim->parts[i].tmp = 0;
		}
	}
	memset(sim->wireless, 0, sizeof(sim->wireless));
}

void GameController::SwitchGravity()
{
	gameModel->GetSimulation()->gravityMode = (gameModel->GetSimulation()->gravityMode + 1) % NUM_GRAVMODES;

	switch (gameModel->GetSimulation()->gravityMode)
	{
	case GRAV_VERTICAL:
		gameModel->SetInfoTip("Gravity: Vertical");
		break;
	case GRAV_OFF:
		gameModel->SetInfoTip("Gravity: Off");
		break;
	case GRAV_RADIAL:
		gameModel->SetInfoTip("Gravity: Radial");
		break;
	case GRAV_CUSTOM:
		gameModel->SetInfoTip("Gravity: Custom");
		break;
	}
}

void GameController::SwitchAir()
{
	gameModel->GetSimulation()->air->airMode = (gameModel->GetSimulation()->air->airMode + 1) % NUM_AIRMODES;

	switch (gameModel->GetSimulation()->air->airMode)
	{
	case AIR_ON:
		gameModel->SetInfoTip("Air: On");
		break;
	case AIR_PRESSUREOFF:
		gameModel->SetInfoTip("Air: Pressure Off");
		break;
	case AIR_VELOCITYOFF:
		gameModel->SetInfoTip("Air: Velocity Off");
		break;
	case AIR_OFF:
		gameModel->SetInfoTip("Air: Off");
		break;
	case AIR_NOUPDATE:
		gameModel->SetInfoTip("Air: No Update");
		break;
	}
}

void GameController::ToggleAHeat()
{
	gameModel->SetAHeatEnable(!gameModel->GetAHeatEnable());
}

bool GameController::GetAHeatEnable()
{
	return gameModel->GetAHeatEnable();
}

void GameController::ResetAHeat()
{
	gameModel->ResetAHeat();
}

void GameController::ToggleNewtonianGravity()
{
	gameModel->SetNewtonianGravity(!gameModel->GetNewtonianGrvity());
}

void GameController::LoadRenderPreset(int presetNum)
{
	auto &settings = gameModel->GetRendererSettings();
	RenderPreset preset = Renderer::renderModePresets[presetNum];
	gameModel->SetInfoTip(preset.Name);
	settings.renderMode = preset.renderMode;
	settings.displayMode = preset.displayMode;
	settings.colorMode = preset.colorMode;
	settings.wantHdispLimitMin = preset.wantHdispLimitMin;
	settings.wantHdispLimitMax = preset.wantHdispLimitMax;
}

void GameController::Update()
{
	auto &sd = SimulationData::CRef();
	ui::Point pos = gameView->GetMousePosition();
	gameModel->GetRendererSettings().mousePos = PointTranslate(pos);
	if (pos.X < XRES && pos.Y < YRES)
		gameView->SetSample(gameModel->GetSimulation()->GetSample(PointTranslate(pos).X, PointTranslate(pos).Y));
	else
		gameView->SetSample(gameModel->GetSimulation()->GetSample(pos.X, pos.Y));

	Simulation * sim = gameModel->GetSimulation();
	if (gameModel->IsSimRunning())
	{
		gameModel->UpdateUpTo(NPART);
	}
	else
	{
		gameModel->BeforeSim();
	}

	//if either STKM or STK2 isn't out, reset it's selected element. Defaults to PT_DUST unless right selected is something else
	//This won't run if the stickmen dies in a frame, since it respawns instantly
	if (!sim->player.spwn || !sim->player2.spwn)
	{
		int rightSelected = PT_DUST;
		Tool * activeTool = gameModel->GetActiveTool(1);
		if (activeTool->Identifier.BeginsWith("DEFAULT_PT_"))
		{
			int sr = activeTool->ToolID;
			if (sr && sd.IsElementOrNone(sr))
				rightSelected = sr;
		}

		if (!sim->player.spwn)
			Element_STKM_set_element(sim, &sim->player, rightSelected);
		if (!sim->player2.spwn)
			Element_STKM_set_element(sim, &sim->player2, rightSelected);
	}
	if(renderOptions && renderOptions->HasExited)
	{
		delete renderOptions;
		renderOptions = nullptr;
	}

	if(search && search->HasExited)
	{
		delete search;
		search = nullptr;
	}

	if(activePreview && activePreview->HasExited)
	{
		delete activePreview;
		activePreview = nullptr;
	}

	if(loginWindow && loginWindow->HasExited)
	{
		delete loginWindow;
		loginWindow = nullptr;
	}

	if(localBrowser && localBrowser->HasDone)
	{
		delete localBrowser;
		localBrowser = nullptr;
	}
}

void GameController::SetZoomEnabled(bool zoomEnabled)
{
	gameModel->SetZoomEnabled(zoomEnabled);
}

void GameController::SetToolStrength(float value)
{
	gameModel->SetToolStrength(value);
}

void GameController::SetZoomPosition(ui::Point position)
{
	auto zoomhalf = gameModel->GetZoomSize() / 2;
	ui::Point zoomPosition = position - Vec2{ zoomhalf, zoomhalf };
	if(zoomPosition.X < 0)
			zoomPosition.X = 0;
	if(zoomPosition.Y < 0)
			zoomPosition.Y = 0;
	if(zoomPosition.X >= XRES-gameModel->GetZoomSize())
			zoomPosition.X = XRES-gameModel->GetZoomSize();
	if(zoomPosition.Y >= YRES-gameModel->GetZoomSize())
			zoomPosition.Y = YRES-gameModel->GetZoomSize();

	ui::Point zoomWindowPosition = ui::Point(0, 0);
	if(position.X < XRES/2)
		zoomWindowPosition.X = XRES-(gameModel->GetZoomSize()*gameModel->GetZoomFactor());

	gameModel->SetZoomPosition(zoomPosition);
	gameModel->SetZoomWindowPosition(zoomWindowPosition);
}

bool GameController::GetPaused() const
{
	return gameModel->GetPaused();
}

void GameController::SetPaused(bool pauseState)
{
	gameModel->SetPaused(pauseState);
}

void GameController::SetPaused()
{
	gameModel->SetPaused(!gameModel->GetPaused());
}

void GameController::SetDecoration(bool decorationState)
{
	gameModel->SetDecoration(decorationState);
}

void GameController::SetDecoration()
{
	gameModel->SetDecoration(!gameModel->GetDecoration());
}

void GameController::ShowGravityGrid()
{
	gameModel->ShowGravityGrid(!gameModel->GetGravityGrid());
	gameModel->UpdateQuickOptions();
}

void GameController::SetHudEnable(bool hudState)
{
	gameView->SetHudEnable(hudState);
}

bool GameController::GetHudEnable()
{
	return gameView->GetHudEnable();
}

void GameController::SetBrushEnable(bool brushState)
{
	gameView->SetBrushEnable(brushState);
}

bool GameController::GetBrushEnable()
{
	return gameView->GetBrushEnable();
}

void GameController::SetDebugHUD(bool hudState)
{
	gameView->SetDebugHUD(hudState);
}

bool GameController::GetDebugHUD()
{
	return gameView->GetDebugHUD();
}

void GameController::SetTemperatureScale(TempScale temperatureScale)
{
	gameModel->SetTemperatureScale(temperatureScale);
}

TempScale GameController::GetTemperatureScale()
{
	return gameModel->GetTemperatureScale();
}

int GameController::GetEdgeMode()
{
	return gameModel->GetEdgeMode();
}

void GameController::SetEdgeMode(int edgeMode)
{
	if (edgeMode < 0 || edgeMode >= NUM_EDGEMODES)
		edgeMode = 0;

	gameModel->SetEdgeMode(edgeMode);

	switch (edgeMode)
	{
		case EDGE_VOID:
			gameModel->SetInfoTip("Edge Mode: Void");
			break;
		case EDGE_SOLID:
			gameModel->SetInfoTip("Edge Mode: Solid");
			break;
		case EDGE_LOOP:
			gameModel->SetInfoTip("Edge Mode: Loop");
			break;
	}
}

void GameController::SetActiveColourPreset(int preset)
{
	gameModel->SetActiveColourPreset(preset);
}

void GameController::SetColour(ui::Colour colour)
{
	gameModel->SetColourSelectorColour(colour);
	gameModel->SetPresetColour(colour);
}

void GameController::SetActiveMenu(int menuID)
{
	gameModel->SetActiveMenu(menuID);
	if(menuID == SC_DECO)
		gameModel->SetColourSelectorVisibility(true);
	else
		gameModel->SetColourSelectorVisibility(false);
}

std::vector<Menu*> GameController::GetMenuList()
{
	return gameModel->GetMenuList();
}

int GameController::GetNumMenus(bool onlyEnabled)
{
	int count = 0;
	if (onlyEnabled)
	{
		std::vector<Menu*> menuList = gameModel->GetMenuList();
		for (std::vector<Menu*>::iterator it = menuList.begin(), end = menuList.end(); it != end; ++it)
			if ((*it)->GetVisible())
				count++;
	}
	else
		count = gameModel->GetMenuList().size();
	return count;
}

void GameController::RebuildFavoritesMenu()
{
	gameModel->BuildMenus();
}

Tool * GameController::GetActiveTool(int selection)
{
	return gameModel->GetActiveTool(selection);
}

void GameController::SetActiveTool(int toolSelection, Tool * tool)
{
	if (gameModel->GetActiveMenu() == SC_DECO && toolSelection == 2)
		toolSelection = 0;
	gameModel->SetActiveTool(toolSelection, tool);
	gameModel->GetRendererSettings().gravityZonesEnabled = false;
	if (toolSelection == 3)
		gameModel->GetSimulation()->replaceModeSelected = tool->ToolID;
	gameModel->SetLastTool(tool);
	for(int i = 0; i < 3; i++)
	{
		auto *activeTool = gameModel->GetActiveTool(i);
		if (activeTool && activeTool->Identifier == "DEFAULT_WL_GRVTY")
		{
			gameModel->GetRendererSettings().gravityZonesEnabled = true;
		}
	}
	tool->Select(toolSelection);
}

void GameController::SetActiveTool(int toolSelection, ByteString identifier)
{
	Tool *tool = gameModel->GetToolFromIdentifier(identifier);
	if (!tool)
		return;
	SetActiveTool(toolSelection, tool);
}

void GameController::SetLastTool(Tool * tool)
{
	gameModel->SetLastTool(tool);
}

Tool *GameController::GetLastTool()
{
	return gameModel->GetLastTool();
}

int GameController::GetReplaceModeFlags()
{
	return gameModel->GetSimulation()->replaceModeFlags;
}

void GameController::SetReplaceModeFlags(int flags)
{
	int old_flags = gameModel->GetSimulation()->replaceModeFlags;
	if (!(old_flags & REPLACE_MODE) && (flags & REPLACE_MODE))
	{
		// if replace mode has just been enabled, disable specific delete
		flags &= ~SPECIFIC_DELETE;
	}
	if (!(old_flags & SPECIFIC_DELETE) && (flags & SPECIFIC_DELETE))
	{
		// if specific delete has just been enabled, disable replace mode
		flags &= ~REPLACE_MODE;
	}
	if ((flags & SPECIFIC_DELETE) && (flags & REPLACE_MODE))
	{
		// if both have just been enabled, arbitrarily disable one of them
		flags &= ~SPECIFIC_DELETE;
	}
	gameModel->GetSimulation()->replaceModeFlags = flags;
}

void GameController::OpenSearch(String searchText)
{
	if(!search)
		search = new SearchController([this] {
			if (search->GetLoadedSave())
			{
				try
				{
					HistorySnapshot();
					gameModel->SetSave(search->TakeLoadedSave(), gameView->ShiftBehaviour());
				}
				catch(GameModelException & ex)
				{
					new ErrorMessage("Cannot open save", ByteString(ex.what()).FromUtf8());
				}
			}
		});
	if (searchText.length())
		search->DoSearch2(searchText);
	ui::Engine::Ref().ShowWindow(search->GetView());
}

void GameController::OpenLocalSaveWindow(bool asCurrent)
{
	Simulation * sim = gameModel->GetSimulation();
	auto gameSave = sim->Save(gameModel->GetIncludePressure() != gameView->ShiftBehaviour(), RES.OriginRect());
	if(!gameSave)
	{
		new ErrorMessage("Error", "Unable to build save.");
	}
	else
	{
		gameSave->paused = gameModel->GetPaused();

		auto tempSave = std::make_unique<SaveFile>("");
		if (gameModel->GetSaveFile())
		{
			tempSave->SetFileName(gameModel->GetSaveFile()->GetName());
			tempSave->SetDisplayName(gameModel->GetSaveFile()->GetDisplayName());
		}

		if (!asCurrent || !gameModel->GetSaveFile())
		{
			tempSave->SetGameSave(std::move(gameSave));
			new LocalSaveActivity(std::move(tempSave), [this](auto file) {
				gameModel->SetSaveFile(std::move(file), gameView->ShiftBehaviour());
			});
		}
		else if (gameModel->GetSaveFile())
		{
			Bson localSaveInfo;
			localSaveInfo["type"] = "localsave";
			auto user = Client::Ref().GetAuthUser();
			localSaveInfo["username"] = user ? user->Username : ByteString("");
			localSaveInfo["title"] = gameModel->GetSaveFile()->GetName();
			localSaveInfo["date"] = int64_t(time(nullptr));
			Client::Ref().SaveAuthorInfo(localSaveInfo);
			gameSave->authors = localSaveInfo;

			Platform::MakeDirectory(LOCAL_SAVE_DIR);
			std::vector<char> saveData;
			std::tie(std::ignore, saveData) = gameSave->Serialise();
			tempSave->SetGameSave(std::move(gameSave));
			gameModel->SetSaveFile(std::move(tempSave), gameView->ShiftBehaviour());
			if (saveData.size() == 0)
				new ErrorMessage("Error", "Unable to serialize game data.");
			else if (!Platform::WriteFile(saveData, gameModel->GetSaveFile()->GetName()))
				new ErrorMessage("Error", "Unable to write save file.");
			else
				gameModel->SetInfoTip("Saved Successfully");
		}
	}
}

void GameController::LoadSaveFile(std::unique_ptr<SaveFile> file)
{
	gameModel->SetSaveFile(std::move(file), gameView->ShiftBehaviour());
}


void GameController::LoadSave(std::unique_ptr<SaveInfo> save)
{
	gameModel->SetSave(std::move(save), gameView->ShiftBehaviour());
}

void GameController::OpenSaveDone()
{
	if (activePreview->GetDoOpen() && activePreview->GetSaveInfo())
	{
		try
		{
			HistorySnapshot();
			LoadSave(activePreview->TakeSaveInfo());
		}
		catch(GameModelException & ex)
		{
			new ErrorMessage("Cannot open save", ByteString(ex.what()).FromUtf8());
		}
	}
}

void GameController::OpenSavePreview(int saveID, int saveDate, SavePreviewType savePreviewType)
{
	if (savePreviewType == savePreviewUrl)
	{
		gameView->SkipIntroText();
	}
	activePreview = new PreviewController(saveID, saveDate, savePreviewType, [this] { OpenSaveDone(); }, nullptr);
	ui::Engine::Ref().ShowWindow(activePreview->GetView());
}

void GameController::OpenSavePreview()
{
	if(gameModel->GetSave())
	{
		activePreview = new PreviewController(gameModel->GetSave()->GetID(), 0, savePreviewNormal, [this] { OpenSaveDone(); }, nullptr);
		ui::Engine::Ref().ShowWindow(activePreview->GetView());
	}
}

void GameController::OpenLocalBrowse()
{
	new FileBrowserActivity(ByteString::Build(LOCAL_SAVE_DIR, PATH_SEP_CHAR), [this](auto file) {
		HistorySnapshot();
		LoadSaveFile(std::move(file));
	});
}

void GameController::OpenLogin()
{
	loginWindow = new LoginController();
	ui::Engine::Ref().ShowWindow(loginWindow->GetView());
}

void GameController::OpenProfile()
{
	auto user = Client::Ref().GetAuthUser();
	if (user)
	{
		new ProfileActivity(user->Username);
	}
	else
	{
		loginWindow = new LoginController();
		ui::Engine::Ref().ShowWindow(loginWindow->GetView());
	}
}

void GameController::OpenElementSearch()
{
	std::vector<Tool *> toolList;
	for (auto &ptr : gameModel->GetTools())
	{
		if (!ptr)
		{
			continue;
		}
		toolList.push_back(ptr.get());
	}
	new ElementSearchActivity(this, toolList);
}

void GameController::OpenColourPicker()
{
	new ColourPickerActivity(gameModel->GetColourSelectorColour(), [this](ui::Colour colour) {
		SetColour(colour);
	});
}

void GameController::OpenTags()
{
	if(gameModel->GetSave() && gameModel->GetSave()->GetID())
	{
		delete tagsWindow;
		tagsWindow = new TagsController([this] { gameView->NotifySaveChanged(gameModel); }, gameModel->GetSave());
		ui::Engine::Ref().ShowWindow(tagsWindow->GetView());
	}
	else
	{
		new ErrorMessage("Error", "No save open");
	}
}

void GameController::OpenStamps()
{
	localBrowser = new LocalBrowserController([this] {
		auto file = localBrowser->TakeSave();
		if (file)
		{
			if (file->GetError().length())
				new ErrorMessage("Error loading stamp", file->GetError());
			else if (localBrowser->GetMoveToFront())
				Client::Ref().MoveStampToFront(file->GetDisplayName().ToUtf8());
			LoadStamp(file->TakeGameSave());
		}
	});
	ui::Engine::Ref().ShowWindow(localBrowser->GetView());
}

void GameController::OpenOptions()
{
	options = new OptionsController(gameModel, [this] {
		gameModel->UpdateQuickOptions();
	});
	ui::Engine::Ref().ShowWindow(options->GetView());

}

void GameController::ShowConsole()
{
	if (!console)
		console = new ConsoleController(nullptr, commandInterface.get());
	if (console->GetView() != ui::Engine::Ref().GetWindow())
		ui::Engine::Ref().ShowWindow(console->GetView());
}

void GameController::HideConsole()
{
	if (!console)
		return;
	console->GetView()->CloseActiveWindow();
}

void GameController::OpenRenderOptions()
{
	renderOptions = new RenderController(gameModel->GetSimulation(), gameModel->GetRenderer(), &gameModel->GetRendererSettings(), nullptr);
	ui::Engine::Ref().ShowWindow(renderOptions->GetView());
}

void GameController::OpenSaveWindow()
{
	auto user = gameModel->GetUser();
	if (user)
	{
		Simulation * sim = gameModel->GetSimulation();
		auto gameSave = sim->Save(gameModel->GetIncludePressure() != gameView->ShiftBehaviour(), RES.OriginRect());
		if(!gameSave)
		{
			new ErrorMessage("Error", "Unable to build save.");
		}
		else
		{
			gameSave->paused = gameModel->GetPaused();

			if(gameModel->GetSave())
			{
				auto tempSave = gameModel->GetSave()->CloneInfo();
				tempSave->SetGameSave(std::move(gameSave));
				new ServerSaveActivity(std::move(tempSave), [this](auto save) {
					save->SetVote(1);
					save->SetVotesUp(1);
					LoadSave(std::move(save));
				});
			}
			else
			{
				auto tempSave = std::make_unique<SaveInfo>(0, 0, 0, 0, 0, user->Username, "");
				tempSave->SetGameSave(std::move(gameSave));
				new ServerSaveActivity(std::move(tempSave), [this](auto save) {
					save->SetVote(1);
					save->SetVotesUp(1);
					LoadSave(std::move(save));
				});
			}
		}
	}
	else
	{
		new ErrorMessage("Error", "You need to login to upload saves.");
	}
}

void GameController::SaveAsCurrent()
{
	auto user = gameModel->GetUser();
	if (gameModel->GetSave() && user && user->Username == gameModel->GetSave()->GetUserName())
	{
		Simulation * sim = gameModel->GetSimulation();
		auto gameSave = sim->Save(gameModel->GetIncludePressure() != gameView->ShiftBehaviour(), RES.OriginRect());
		if(!gameSave)
		{
			new ErrorMessage("Error", "Unable to build save.");
		}
		else
		{
			gameSave->paused = gameModel->GetPaused();

			if(gameModel->GetSave())
			{
				auto tempSave = gameModel->GetSave()->CloneInfo();
				tempSave->SetGameSave(std::move(gameSave));
				new ServerSaveActivity(std::move(tempSave), true, [this](auto save) { LoadSave(std::move(save)); });
			}
			else
			{
				auto tempSave = std::make_unique<SaveInfo>(0, 0, 0, 0, 0, user->Username, "");
				tempSave->SetGameSave(std::move(gameSave));
				new ServerSaveActivity(std::move(tempSave), true, [this](auto save) { LoadSave(std::move(save)); });
			}
		}
	}
	else if (user)
	{
		OpenSaveWindow();
	}
	else
	{
		new ErrorMessage("Error", "You need to login to upload saves.");
	}
}

void GameController::FrameStep()
{
	gameModel->FrameStep(1);
	gameModel->SetPaused(true);
}

void GameController::Vote(int direction)
{
	if (gameModel->GetSave() && gameModel->GetUser() && gameModel->GetSave()->GetID())
	{
		gameModel->SetVote(direction);
	}
}

void GameController::ChangeBrush()
{
	gameModel->SetBrushID(gameModel->GetBrushID()+1);
}

void GameController::ClearSim()
{
	HistorySnapshot();
	gameModel->SetSave(nullptr, false);
	gameModel->ClearSimulation();
}

String GameController::ElementResolve(int type, int ctype)
{
	// "NONE" should never be displayed in the HUD
	if (!type)
		return "";
	auto &sd = SimulationData::CRef();
	return sd.ElementResolve(type, ctype);
}

String GameController::BasicParticleInfo(Particle const &sample_part)
{
	auto &sd = SimulationData::CRef();
	return sd.BasicParticleInfo(sample_part);
}

void GameController::ReloadSim()
{
	if(gameModel->GetSave() && gameModel->GetSave()->GetGameSave())
	{
		HistorySnapshot();
		gameModel->SetSave(gameModel->TakeSave(), gameView->ShiftBehaviour());
	}
	else if(gameModel->GetSaveFile() && gameModel->GetSaveFile()->GetGameSave())
	{
		HistorySnapshot();
		gameModel->SetSaveFile(gameModel->TakeSaveFile(), gameView->ShiftBehaviour());
	}
}

bool GameController::IsValidElement(int type)
{
	auto &sd = SimulationData::CRef();
	return type && sd.IsElement(type);
}

String GameController::WallName(int type)
{
	auto &sd = SimulationData::CRef();
	if(type >= 0 && type < UI_WALLCOUNT)
		return sd.wtypes[type].name;
	else
		return String();
}

ByteString GameController::TakeScreenshot(int captureUI, int fileType)
{
	return gameView->TakeScreenshot(captureUI, fileType);
}

int GameController::Record(bool record)
{
	return gameView->Record(record);
}

void GameController::NotifyAuthUserChanged(Client * sender)
{
	auto newUser = sender->GetAuthUser();
	gameModel->SetUser(newUser);
}

void GameController::NotifyNewNotification(Client * sender, ServerNotification notification)
{
	class LinkNotification : public Notification
	{
		ByteString link;
	public:
		LinkNotification(ByteString link_, String message) : Notification(message), link(link_) {}
		virtual ~LinkNotification() {}

		void Action() override
		{
			Platform::OpenURI(link);
		}
	};
	gameModel->AddNotification(new LinkNotification(notification.link, notification.text));
}

void GameController::NotifyUpdateAvailable(Client * sender)
{
	class UpdateNotification : public Notification
	{
		GameController * c;
	public:
		UpdateNotification(GameController * c, String message) : Notification(message), c(c) {}
		virtual ~UpdateNotification() {}

		void Action() override
		{
			auto optinfo = Client::Ref().GetUpdateInfo();
			if (!optinfo.has_value())
			{
				std::cerr << "odd, the update has disappeared" << std::endl;
				return;
			}
			UpdateInfo info = optinfo.value();
			StringBuilder updateMessage;
			if (Platform::CanUpdate())
			{
				updateMessage << "Are you sure you want to run the updater? Please save any changes before updating.\n\nCurrent version:\n ";
			}
			else
			{
				updateMessage << "Click \"Continue\" to download the latest version from our website.\n\nCurrent version:\n ";
			}

			if constexpr (MOD)
			{
				updateMessage << "Mod " << MOD_ID << " ";
			}
			if constexpr (SNAPSHOT)
			{
				updateMessage << "Snapshot " << APP_VERSION.build;
			}
			else if constexpr (BETA)
			{
				updateMessage << DISPLAY_VERSION[0] << "." << DISPLAY_VERSION[1] << " Beta, Build " << APP_VERSION.build;
			}
			else
			{
				updateMessage << DISPLAY_VERSION[0] << "." << DISPLAY_VERSION[1] << " Stable, Build " << APP_VERSION.build;
			}

			updateMessage << "\nNew version:\n ";
			if (info.channel == UpdateInfo::channelBeta)
			{
				updateMessage << info.major << "." << info.minor << " Beta, Build " << info.build;
			}
			else if (info.channel == UpdateInfo::channelSnapshot)
			{
				if constexpr (MOD)
				{
					updateMessage << "Mod version " << info.build;
				}
				else
				{
					updateMessage << "Snapshot " << info.build;
				}
			}
			else if(info.channel == UpdateInfo::channelStable)
			{
				updateMessage << info.major << "." << info.minor << " Stable, Build " << info.build;
			}

			if (info.changeLog.length())
				updateMessage << "\n\nChangelog:\n" << info.changeLog;

			new ConfirmPrompt("Run Updater", updateMessage.Build(), { [this, info] { c->RunUpdater(info); } });
		}
	};

	auto optinfo = sender->GetUpdateInfo();
	if (!optinfo.has_value())
	{
		return;
	}
	switch(optinfo.value().channel)
	{
		case UpdateInfo::channelSnapshot:
			if constexpr (MOD)
			{
				gameModel->AddNotification(new UpdateNotification(this, "A new mod update is available - click here to update"));
			}
			else
			{
				gameModel->AddNotification(new UpdateNotification(this, "A new snapshot is available - click here to update"));
			}
			break;
		case UpdateInfo::channelStable:
			gameModel->AddNotification(new UpdateNotification(this, "A new version is available - click here to update"));
			break;
		case UpdateInfo::channelBeta:
			gameModel->AddNotification(new UpdateNotification(this, "A new beta is available - click here to update"));
			break;
	}
}

void GameController::RemoveNotification(Notification * notification)
{
	gameModel->RemoveNotification(notification);
}

void GameController::RunUpdater(UpdateInfo info)
{
	if (Platform::CanUpdate())
	{
		Exit();
		new UpdateActivity(info);
	}
	else
	{
		Platform::OpenURI(info.file);
	}
}

bool GameController::GetMouseClickRequired()
{
	return gameModel->GetMouseClickRequired();
}

bool GameController::GetThreadedRendering()
{
	return gameModel->GetThreadedRendering();
}

void GameController::RemoveCustomGol(const ByteString &identifier)
{
	gameModel->RemoveCustomGol(identifier);
}

void GameController::BeforeSimDraw()
{
	commandInterface->HandleEvent(BeforeSimDrawEvent{});
}

void GameController::AfterSimDraw()
{
	commandInterface->HandleEvent(AfterSimDrawEvent{});
}

bool GameController::ThreadedRenderingAllowed()
{
	return gameModel->GetThreadedRendering() && !GetPaused() && !commandInterface->HaveSimGraphicsEventHandlers();
}

void GameController::SetToolIndex(ByteString identifier, std::optional<int> index)
{
	if (commandInterface)
	{
		commandInterface->SetToolIndex(identifier, index);
	}
}
