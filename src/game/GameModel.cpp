#include "interface/Engine.h"
#include "GameModel.h"
#include "GameView.h"
#include "simulation/Simulation.h"
#include "simulation/Air.h"
#include "Renderer.h"
#include "interface/Point.h"
#include "Brush.h"
#include "EllipseBrush.h"
#include "client/Client.h"
#include "game/DecorationTool.h"
#include "GameModelException.h"

GameModel::GameModel():
	sim(NULL),
	ren(NULL),
	currentBrush(0),
	currentUser(0, ""),
	currentSave(NULL),
	colourSelector(false),
	clipboard(NULL),
	stamp(NULL),
	placeSave(NULL)
{
	sim = new Simulation();
	ren = new Renderer(ui::Engine::Ref().g, sim);

    memset(activeTools, 0, sizeof(activeTools));
    
	//Load config into renderer
	try
	{
		json::Number tempNumber = Client::Ref().configDocument["Renderer"]["ColourMode"];
		if(tempNumber.Value())
			ren->SetColourMode(tempNumber.Value());

		json::Array tempArray = Client::Ref().configDocument["Renderer"]["DisplayModes"];
		if(tempArray.Size())
		{
			std::vector<unsigned int> displayModes;
			json::Array::const_iterator itDisplayModes(tempArray.Begin()), itDisplayModesEnd(tempArray.End());
			for (; itDisplayModes != itDisplayModesEnd; ++itDisplayModes)
			{
				json::Number tempNumberI = *itDisplayModes;
				displayModes.push_back(tempNumberI.Value());
			}
			ren->SetDisplayMode(displayModes);
		}

		tempArray = Client::Ref().configDocument["Renderer"]["RenderModes"];
		if(tempArray.Size())
		{
			std::vector<unsigned int> renderModes;
			json::Array::const_iterator itRenderModes(tempArray.Begin()), itRenderModesEnd(tempArray.End());
			for (; itRenderModes != itRenderModesEnd; ++itRenderModes)
			{
				json::Number tempNumberI = *itRenderModes;
				renderModes.push_back(tempNumberI.Value());
			}
			ren->SetRenderMode(renderModes);
		}
	}
	catch(json::Exception & e)
	{

	}

	menuList.clear();
	for(int i = 0; i < SC_TOTAL; i++)
	{
		menuList.push_back(new Menu((const char)sim->msections[i].icon[0], sim->msections[i].name));
	}
	//Build menus from Simulation elements
	for(int i = 0; i < PT_NUM; i++)
	{
		if(sim->elements[i].MenuSection < 12 && sim->elements[i].Enabled && sim->elements[i].MenuVisible)
		{
			Tool * tempTool = new ElementTool(i, sim->elements[i].Name, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour));
			menuList[sim->elements[i].MenuSection]->AddTool(tempTool);
		}
	}

	//Build menu for GOL types
	for(int i = 0; i < NGOL; i++)
	{
		Tool * tempTool = new GolTool(i, sim->gmenu[i].name, PIXR(sim->gmenu[i].colour), PIXG(sim->gmenu[i].colour), PIXB(sim->gmenu[i].colour));
		menuList[SC_LIFE]->AddTool(tempTool);
	}

	//Build other menus from wall data
	for(int i = 0; i < UI_WALLCOUNT; i++)
	{
		Tool * tempTool = new WallTool(i, "", PIXR(sim->wtypes[i].colour), PIXG(sim->wtypes[i].colour), PIXB(sim->wtypes[i].colour));
		menuList[SC_WALL]->AddTool(tempTool);
		//sim->wtypes[i]
	}
	
	//Add special sign and prop tools
	menuList[SC_TOOL]->AddTool(new SignTool());
	menuList[SC_TOOL]->AddTool(new PropertyTool());
	
	//Build menu for simtools
	for(int i = 0; i < sim->tools.size(); i++)
	{
		Tool * tempTool = new Tool(i, sim->tools[i]->Name, PIXR(sim->tools[i]->Colour), PIXG(sim->tools[i]->Colour), PIXB(sim->tools[i]->Colour));
		menuList[SC_TOOL]->AddTool(tempTool);
	}

	//Add decoration tools to menu
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendAdd, "ADD", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendRemove, "SUB", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendMultiply, "MUL", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendDivide, "DIV", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendSmudge, "SMDG", 0, 0, 0));
	menuList[SC_DECO]->AddTool(new DecorationTool(DecorationTool::BlendSet, "SET", 0, 0, 0));

	//Set default brush palette
	brushList.push_back(new EllipseBrush(ui::Point(4, 4)));
	brushList.push_back(new Brush(ui::Point(4, 4)));

	//Set default tools
	activeTools[0] = menuList[SC_POWDERS]->GetToolList()[0];
	activeTools[1] = menuList[SC_SPECIAL]->GetToolList()[0];

	//Set default menu
	activeMenu = menuList[SC_POWDERS];
	toolList = menuList[SC_POWDERS]->GetToolList();

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
}

GameModel::~GameModel()
{
	//Save to config:
	try
	{
		Client::Ref().configDocument["Renderer"]["ColourMode"] = json::Number(ren->GetColourMode());

		Client::Ref().configDocument["Renderer"]["DisplayModes"] = json::Array();
		std::vector<unsigned int> displayModes = ren->GetDisplayMode();
		for (int i = 0; i < displayModes.size(); i++)
		{
			Client::Ref().configDocument["Renderer"]["DisplayModes"][i] = json::Number(displayModes[i]);
		}

		Client::Ref().configDocument["Renderer"]["RenderModes"] = json::Array();
		std::vector<unsigned int> renderModes = ren->GetRenderMode();
		for (int i = 0; i < renderModes.size(); i++)
		{
			Client::Ref().configDocument["Renderer"]["RenderModes"][i] = json::Number(renderModes[i]);
		}
	}
	catch(json::Exception & e)
	{

	}

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
	//if(activeTools)
	//	delete[] activeTools;
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
		delete currentSave;
	currentSave = newSave;
	if(currentSave && currentSave->GetGameSave())
	{
		GameSave * saveData = currentSave->GetGameSave();
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
	if(stamp)
		delete stamp;
	if(save)
		stamp = new GameSave(*save);
	else
		stamp = NULL;
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
