#include "interface/Engine.h"
#include "GameModel.h"
#include "GameView.h"
#include "simulation/Simulation.h"
#include "simulation/Air.h"
#include "simulation/Tools.h"
#include "graphics/Renderer.h"
#include "interface/Point.h"
#include "Brush.h"
#include "EllipseBrush.h"
#include "TriangleBrush.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "game/DecorationTool.h"
#include "GameModelException.h"
#include "QuickOptions.h"

GameModel::GameModel():
	sim(NULL),
	ren(NULL),
	currentBrush(0),
	currentUser(0, ""),
	currentSave(NULL),
	colourSelector(false),
	clipboard(NULL),
	stamp(NULL),
	placeSave(NULL),
	colour(255, 0, 0, 255),
	toolStrength(1.0f)
{
	sim = new Simulation();
	ren = new Renderer(ui::Engine::Ref().g, sim);

    memset(activeTools, 0, sizeof(activeTools));
    
	//Load config into renderer
	try
	{
		ren->SetColourMode(Client::Ref().GetPrefUInteger("Renderer.ColourMode", 0));

		vector<unsigned int> tempArray = Client::Ref().GetPrefUIntegerArray("Renderer.DisplayModes");
		if(tempArray.size())
		{
			std::vector<unsigned int> displayModes(tempArray.begin(), tempArray.end());
			ren->SetDisplayMode(displayModes);
		}

		tempArray = Client::Ref().GetPrefUIntegerArray("Renderer.RenderModes");
		if(tempArray.size())
		{
			std::vector<unsigned int> renderModes(tempArray.begin(), tempArray.end());
			ren->SetRenderMode(renderModes);
		}
	}
	catch(json::Exception & e)
	{

	}

	//Load config into simulation
	sim->SetEdgeMode(Client::Ref().GetPrefInteger("Simulation.EdgeMode", 0));

	//Load last user
	if(Client::Ref().GetAuthUser().ID)
	{
		currentUser = Client::Ref().GetAuthUser();
	}

	//Set stamp to first stamp in list
	vector<string> stamps = Client::Ref().GetStamps(0, 1);
	if(stamps.size()>0)
	{
		SaveFile * stampFile = Client::Ref().GetStamp(stamps[0]);
		if(stampFile && stampFile->GetGameSave())
			stamp = stampFile->GetGameSave();
	}

	BuildMenus();
	BuildQuickOptionMenu();

	//Set default decoration colour
	unsigned char colourR = min(Client::Ref().GetPrefInteger("Decoration.Red", 200), 255);
	unsigned char colourG = min(Client::Ref().GetPrefInteger("Decoration.Green", 100), 255);
	unsigned char colourB = min(Client::Ref().GetPrefInteger("Decoration.Blue", 50), 255);
	unsigned char colourA = min(Client::Ref().GetPrefInteger("Decoration.Alpha", 255), 255);

	SetColourSelectorColour(ui::Colour(colourR, colourG, colourB, colourA));
}

GameModel::~GameModel()
{
	//Save to config:
	Client::Ref().SetPref("Renderer.ColourMode", ren->GetColourMode());

	std::vector<unsigned int> displayModes = ren->GetDisplayMode();
	Client::Ref().SetPref("Renderer.DisplayModes", std::vector<unsigned int>(displayModes.begin(), displayModes.end()));

	std::vector<unsigned int> renderModes = ren->GetRenderMode();
	Client::Ref().SetPref("Renderer.RenderModes", std::vector<unsigned int>(renderModes.begin(), renderModes.end()));

	Client::Ref().SetPref("Simulation.EdgeMode", sim->edgeMode);

	Client::Ref().SetPref("Decoration.Red", (int)colour.Red);
	Client::Ref().SetPref("Decoration.Green", (int)colour.Green);
	Client::Ref().SetPref("Decoration.Blue", (int)colour.Blue);
	Client::Ref().SetPref("Decoration.Alpha", (int)colour.Alpha);

	for(int i = 0; i < menuList.size(); i++)
	{
		delete menuList[i];
	}
	for(int i = 0; i < brushList.size(); i++)
	{
		delete brushList[i];
	}
	delete sim;
	delete ren;
	if(clipboard)
		delete clipboard;
	if(stamp)
		delete stamp;
	if(currentSave)
		delete currentSave;
	//if(activeTools)
	//	delete[] activeTools;
}

void GameModel::UpdateQuickOptions()
{
	for(std::vector<QuickOption*>::iterator iter = quickOptions.begin(), end = quickOptions.end(); iter != end; ++iter)
	{
		QuickOption * option = *iter;
		option->Update();
	}	
}

void GameModel::BuildQuickOptionMenu()
{
	for(std::vector<QuickOption*>::iterator iter = quickOptions.begin(), end = quickOptions.end(); iter != end; ++iter)
	{
		delete *iter;
	}
	quickOptions.clear();

	quickOptions.push_back(new SandEffectOption(this));
	quickOptions.push_back(new DrawGravOption(this));
	quickOptions.push_back(new DecorationsOption(this));
	quickOptions.push_back(new NGravityOption(this));
	quickOptions.push_back(new AHeatOption(this));
	quickOptions.push_back(new DrawGravWallOption(this));

	notifyQuickOptionsChanged();
	UpdateQuickOptions();
}

void GameModel::BuildMenus()
{
	//Empty current menus
	for(std::vector<Menu*>::iterator iter = menuList.begin(), end = menuList.end(); iter != end; ++iter)
	{
		delete *iter;
	}
	menuList.clear();
	toolList.clear();

	//Create menus
	for(int i = 0; i < SC_TOTAL; i++)
	{
		menuList.push_back(new Menu((const char)sim->msections[i].icon[0], sim->msections[i].name));
	}

	//Build menus from Simulation elements
	for(int i = 0; i < PT_NUM; i++)
	{
		if(sim->elements[i].MenuSection < 12 && sim->elements[i].Enabled && sim->elements[i].MenuVisible)
		{
			Tool * tempTool;
			if(i == PT_LIGH)
			{
				tempTool = new Element_LIGH_Tool(i, sim->elements[i].Name, sim->elements[i].Description, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), sim->elements[i].IconGenerator);
			}
			else if(i == PT_TESC)
			{
				tempTool = new Element_TESC_Tool(i, sim->elements[i].Name, sim->elements[i].Description, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), sim->elements[i].IconGenerator);
			}
			else if(i == PT_STKM || i == PT_FIGH || i == PT_STKM2)
			{
				tempTool = new PlopTool(i, sim->elements[i].Name, sim->elements[i].Description, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), sim->elements[i].IconGenerator);
			}
			else
			{
				tempTool = new ElementTool(i, sim->elements[i].Name, sim->elements[i].Description, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), sim->elements[i].IconGenerator);
			}
			menuList[sim->elements[i].MenuSection]->AddTool(tempTool);
		}
	}

	//Build menu for GOL types
	for(int i = 0; i < NGOL; i++)
	{
		Tool * tempTool = new GolTool(i, sim->gmenu[i].name, std::string(sim->gmenu[i].description), PIXR(sim->gmenu[i].colour), PIXG(sim->gmenu[i].colour), PIXB(sim->gmenu[i].colour));
		menuList[SC_LIFE]->AddTool(tempTool);
	}

	//Build other menus from wall data
	for(int i = 0; i < UI_WALLCOUNT; i++)
	{
		Tool * tempTool = new WallTool(i, "", std::string(sim->wtypes[i].descs), PIXR(sim->wtypes[i].colour), PIXG(sim->wtypes[i].colour), PIXB(sim->wtypes[i].colour), sim->wtypes[i].textureGen);
		menuList[SC_WALL]->AddTool(tempTool);
		//sim->wtypes[i]
	}
	
	//Add special sign and prop tools
	menuList[SC_TOOL]->AddTool(new SignTool());
	menuList[SC_TOOL]->AddTool(new PropertyTool());
	menuList[SC_TOOL]->AddTool(new WindTool(0, "WIND", "Create air movement", 64, 64, 64));
	
	//Build menu for simtools
	for(int i = 0; i < sim->tools.size(); i++)
	{
		Tool * tempTool;
		tempTool = new Tool(i, sim->tools[i]->Name, sim->tools[i]->Description, PIXR(sim->tools[i]->Colour), PIXG(sim->tools[i]->Colour), PIXB(sim->tools[i]->Colour));
		menuList[SC_TOOL]->AddTool(tempTool);
	}

	//Add decoration tools to menu
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendAdd, "ADD", "Colour blending: Add", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendRemove, "SUB", "Colour blending: Subtract", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendMultiply, "MUL", "Colour blending: Multiply", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendDivide, "DIV", "Colour blending: Divide" , 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendSmudge, "SMDG", "Smudge colour", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendSet, "SET", "Set colour (No blending)", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::Remove, "CLR", "Clear any set decoration", 0, 0, 0));

	//Set default brush palette
	brushList.push_back(new EllipseBrush(ui::Point(4, 4)));
	brushList.push_back(new Brush(ui::Point(4, 4)));
	brushList.push_back(new TriangleBrush(ui::Point(4, 4)));

	//Set default tools
	activeTools[0] = menuList[SC_POWDERS]->GetToolList()[0];
	activeTools[1] = menuList[SC_SPECIAL]->GetToolList()[0];
	activeTools[2] = NULL;
	lastTool = activeTools[0];

	//Set default menu
	activeMenu = menuList[SC_POWDERS];
	toolList = menuList[SC_POWDERS]->GetToolList();

	notifyMenuListChanged();
	notifyToolListChanged();
	notifyActiveToolsChanged();
	notifyLastToolChanged();
}

std::deque<Snapshot*> GameModel::GetHistory()
{
	return history;
}
void GameModel::SetHistory(std::deque<Snapshot*> newHistory)
{
	history = newHistory;
}

void GameModel::SetVote(int direction)
{
	if(currentSave)
	{
		RequestStatus status = Client::Ref().ExecVote(currentSave->GetID(), direction);
		if(status == RequestOkay)
		{
			currentSave->vote = direction;
			notifySaveChanged();
		}
		else
		{
			throw GameModelException("Could not vote: "+Client::Ref().GetLastError());
		}
	}
}

Brush * GameModel::GetBrush()
{
	return brushList[currentBrush];
}

int GameModel::GetBrushID()
{
	return currentBrush;
}

void GameModel::SetBrush(int i)
{
	currentBrush = i%brushList.size();
	notifyBrushChanged();
}

void GameModel::AddObserver(GameView * observer){
	observers.push_back(observer);

	observer->NotifySimulationChanged(this);
	observer->NotifyRendererChanged(this);
	observer->NotifyPausedChanged(this);
	observer->NotifySaveChanged(this);
	observer->NotifyBrushChanged(this);
	observer->NotifyMenuListChanged(this);
	observer->NotifyToolListChanged(this);
	observer->NotifyUserChanged(this);
	observer->NotifyZoomChanged(this);
	observer->NotifyColourSelectorVisibilityChanged(this);
	observer->NotifyColourSelectorColourChanged(this);
	observer->NotifyQuickOptionsChanged(this);
	observer->NotifyLastToolChanged(this);
	UpdateQuickOptions();
}

void GameModel::SetToolStrength(float value)
{
	toolStrength = value;
}

float GameModel::GetToolStrength()
{
	return toolStrength;
}

void GameModel::SetActiveMenu(Menu * menu)
{
	for(int i = 0; i < menuList.size(); i++)
	{
		if(menuList[i]==menu)
		{
			activeMenu = menu;
			toolList = menu->GetToolList();
			notifyToolListChanged();
		}
	}
}

vector<Tool*> GameModel::GetToolList()
{
	return toolList;
}

Menu * GameModel::GetActiveMenu()
{
	return activeMenu;
}

Tool * GameModel::GetActiveTool(int selection)
{
	return activeTools[selection];
}

void GameModel::SetActiveTool(int selection, Tool * tool)
{
	activeTools[selection] = tool;
	notifyActiveToolsChanged();
}

vector<QuickOption*> GameModel::GetQuickOptions()
{
	return quickOptions;
}

vector<Menu*> GameModel::GetMenuList()
{
	return menuList;
}

SaveInfo * GameModel::GetSave()
{
	return currentSave;
}

void GameModel::SetSave(SaveInfo * newSave)
{
	if(currentSave != newSave)
	{
		if(currentSave)
			delete currentSave;
		if(newSave == NULL)
			currentSave = NULL;
		else
			currentSave = new SaveInfo(*newSave);
	}

	if(currentSave && currentSave->GetGameSave())
	{
		GameSave * saveData = currentSave->GetGameSave();
		SetPaused(saveData->paused | GetPaused());
		sim->gravityMode = saveData->gravityMode;
		sim->air->airMode = saveData->airMode;
		sim->legacy_enable = saveData->legacyEnable;
		sim->water_equal_test = saveData->waterEEnabled;
		if(saveData->gravityEnable)
			sim->grav->start_grav_async();
		else
			sim->grav->stop_grav_async();
		sim->clear_sim();
		sim->Load(saveData);
	}
	notifySaveChanged();
	UpdateQuickOptions();
}

void GameModel::SetSaveFile(SaveFile * newSave)
{
	SetSave(NULL);

	if(newSave && newSave->GetGameSave())
	{
		GameSave * saveData = newSave->GetGameSave();
		SetPaused(saveData->paused & GetPaused());
		sim->gravityMode = saveData->gravityMode;
		sim->air->airMode = saveData->airMode;
		sim->legacy_enable = saveData->legacyEnable;
		sim->water_equal_test = saveData->waterEEnabled;
		if(saveData->gravityEnable && !sim->grav->ngrav_enable)
		{
			sim->grav->start_grav_async();
		}
		else if(!saveData->gravityEnable && sim->grav->ngrav_enable)
		{
			sim->grav->stop_grav_async();
		}
		sim->clear_sim();
		sim->Load(saveData);
	}
	
	notifySaveChanged();
	UpdateQuickOptions();
}

Simulation * GameModel::GetSimulation()
{
	return sim;
}

Renderer * GameModel::GetRenderer()
{
	return ren;
}

User GameModel::GetUser()
{
	return currentUser;
}

Tool * GameModel::GetLastTool()
{
	return lastTool;
}

void GameModel::SetLastTool(Tool * newTool)
{
	if(lastTool != newTool)
	{
		lastTool = newTool;
		notifyLastToolChanged();
	}
}

void GameModel::SetZoomEnabled(bool enabled)
{
	ren->zoomEnabled = enabled;
	notifyZoomChanged();
}

bool GameModel::GetZoomEnabled()
{
	return ren->zoomEnabled;
}

void GameModel::SetZoomPosition(ui::Point position)
{
	ren->zoomScopePosition = position;
	notifyZoomChanged();
}

ui::Point GameModel::GetZoomPosition()
{
	return ren->zoomScopePosition;
}

void GameModel::SetZoomWindowPosition(ui::Point position)
{
	ren->zoomWindowPosition = position;
	notifyZoomChanged();
}

ui::Point GameModel::GetZoomWindowPosition()
{
	return ren->zoomWindowPosition;
}

void GameModel::SetZoomSize(int size)
{
	ren->zoomScopeSize = size;
	notifyZoomChanged();
}

int GameModel::GetZoomSize()
{
	return ren->zoomScopeSize;
}

void GameModel::SetZoomFactor(int factor)
{
	ren->ZFACTOR = factor;
	notifyZoomChanged();
}

int GameModel::GetZoomFactor()
{
	return ren->ZFACTOR;
}

void GameModel::SetColourSelectorVisibility(bool visibility)
{
	if(colourSelector != visibility)
	{
		colourSelector = visibility;
		notifyColourSelectorVisibilityChanged();
	}
}

bool GameModel::GetColourSelectorVisibility()
{
	return colourSelector;
}

void GameModel::SetColourSelectorColour(ui::Colour colour_)
{
	colour = colour_;
	notifyColourSelectorColourChanged();

	vector<Tool*> tools = GetMenuList()[SC_DECO]->GetToolList();
	for(int i = 0; i < tools.size(); i++)
	{
		((DecorationTool*)tools[i])->Red = colour.Red;
		((DecorationTool*)tools[i])->Green = colour.Green;
		((DecorationTool*)tools[i])->Blue = colour.Blue;
		((DecorationTool*)tools[i])->Alpha = colour.Alpha;
	}
}

ui::Colour GameModel::GetColourSelectorColour()
{
	return colour;
}

void GameModel::SetUser(User user)
{
	currentUser = user;
	Client::Ref().SetAuthUser(user);
	notifyUserChanged();
}

void GameModel::SetPaused(bool pauseState)
{
	sim->sys_pause = pauseState?1:0;
	notifyPausedChanged();
}

bool GameModel::GetPaused()
{
	return sim->sys_pause?true:false;
}

void GameModel::SetDecoration(bool decorationState)
{
	ren->decorations_enable = decorationState?1:0;
	notifyDecorationChanged();
	UpdateQuickOptions();
}

bool GameModel::GetDecoration()
{
	return ren->decorations_enable?true:false;
}

void GameModel::FrameStep(int frames)
{
	sim->framerender += frames;
}

void GameModel::ClearSimulation()
{
	sim->clear_sim();
}

void GameModel::SetStamp(GameSave * save)
{
	if(stamp != save)
	{
		if(stamp)
			delete stamp;
		if(save)
			stamp = new GameSave(*save);
		else
			stamp = NULL;
	}
}

void GameModel::SetPlaceSave(GameSave * save)
{
	if(save != placeSave)
		delete placeSave;
	if(save != placeSave)
		placeSave = new GameSave(*save);
	else if(!save)
		placeSave = NULL;
	notifyPlaceSaveChanged();
}

void GameModel::AddStamp(GameSave * save)
{
	if(stamp)
		delete stamp;
	stamp = new GameSave(*save);
	Client::Ref().AddStamp(save);
}

void GameModel::SetClipboard(GameSave * save)
{
	if(clipboard)
		delete clipboard;
	clipboard = save;
}

GameSave * GameModel::GetClipboard()
{
	return clipboard;
}

GameSave * GameModel::GetPlaceSave()
{
	return placeSave;
}

GameSave * GameModel::GetStamp()
{
	return stamp;
}

void GameModel::Log(string message)
{
	consoleLog.push_front(message);
	if(consoleLog.size()>100)
		consoleLog.pop_back();
	notifyLogChanged(message);
}

deque<string> GameModel::GetLog()
{
	return consoleLog;
}

std::vector<Notification*> GameModel::GetNotifications()
{
	return notifications;
}

void GameModel::AddNotification(Notification * notification)
{
	notifications.push_back(notification);
	notifyNotificationsChanged();
}

void GameModel::RemoveNotification(Notification * notification)
{
	for(std::vector<Notification*>::iterator iter = notifications.begin(); iter != notifications.end(); ++iter)
	{
		if(*iter == notification)
		{
			notifications.erase(iter);
			delete *iter;
			break;
		}
	}
	notifyNotificationsChanged();
}

void GameModel::SetToolTip(std::string text)
{
	toolTip = text;
	notifyToolTipChanged();
}

void GameModel::SetInfoTip(std::string text)
{
	infoTip = text;
	notifyInfoTipChanged();
}

std::string GameModel::GetToolTip()
{
	return toolTip;
}

std::string GameModel::GetInfoTip()
{
	return infoTip;
}

void GameModel::notifyNotificationsChanged()
{
	for(std::vector<GameView*>::iterator iter = observers.begin(); iter != observers.end(); ++iter)
	{
		(*iter)->NotifyNotificationsChanged(this);
	}
}

void GameModel::notifyColourSelectorColourChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyColourSelectorColourChanged(this);
	}
}

void GameModel::notifyColourSelectorVisibilityChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyColourSelectorVisibilityChanged(this);
	}
}

void GameModel::notifyRendererChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyRendererChanged(this);
	}
}

void GameModel::notifySaveChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySaveChanged(this);
	}
}

void GameModel::notifySimulationChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySimulationChanged(this);
	}
}

void GameModel::notifyPausedChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPausedChanged(this);
	}
}

void GameModel::notifyDecorationChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		//observers[i]->NotifyPausedChanged(this);
	}
}

void GameModel::notifyBrushChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyBrushChanged(this);
	}
}

void GameModel::notifyMenuListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyMenuListChanged(this);
	}
}

void GameModel::notifyToolListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyToolListChanged(this);
	}
}

void GameModel::notifyActiveToolsChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyActiveToolsChanged(this);
	}
}

void GameModel::notifyUserChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyUserChanged(this);
	}
}

void GameModel::notifyZoomChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyZoomChanged(this);
	}
}

void GameModel::notifyPlaceSaveChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPlaceSaveChanged(this);
	}
}

void GameModel::notifyLogChanged(string entry)
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyLogChanged(this, entry);
	}
}

void GameModel::notifyInfoTipChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyInfoTipChanged(this);
	}
}

void GameModel::notifyToolTipChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyToolTipChanged(this);
	}
}

void GameModel::notifyQuickOptionsChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyQuickOptionsChanged(this);
	}
}

void GameModel::notifyLastToolChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyLastToolChanged(this);
	}
}
