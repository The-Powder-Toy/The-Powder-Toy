#include "gui/interface/Engine.h"
#include "GameModel.h"
#include "GameView.h"
#include "simulation/Simulation.h"
#include "simulation/Air.h"
#include "ToolClasses.h"
#include "graphics/Renderer.h"
#include "gui/interface/Point.h"
#include "Brush.h"
#include "EllipseBrush.h"
#include "TriangleBrush.h"
#include "BitmapBrush.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "gui/game/DecorationTool.h"
#include "QuickOptions.h"
#include "GameModelException.h"
#include "Format.h"

GameModel::GameModel():
	sim(NULL),
	ren(NULL),
	currentBrush(0),
	currentUser(0, ""),
	currentSave(NULL),
	currentFile(NULL),
	colourSelector(false),
	clipboard(NULL),
	stamp(NULL),
	placeSave(NULL),
	colour(255, 0, 0, 255),
	toolStrength(1.0f),
	activeColourPreset(-1),
	activeMenu(-1),
	edgeMode(0)
{
	sim = new Simulation();
	ren = new Renderer(ui::Engine::Ref().g, sim);

	activeTools = regularToolset;

	std::fill(decoToolset, decoToolset+4, (Tool*)NULL);
	std::fill(regularToolset, regularToolset+4, (Tool*)NULL);

	//Default render prefs
	std::vector<unsigned int> tempArray;
	tempArray.push_back(RENDER_FIRE);
	tempArray.push_back(RENDER_EFFE);
	tempArray.push_back(RENDER_BASC);
	ren->SetRenderMode(tempArray);
	tempArray.clear();

	ren->SetDisplayMode(tempArray);

	ren->SetColourMode(0);

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

		ren->gravityFieldEnabled = Client::Ref().GetPrefBool("Renderer.GravityField", false);
		ren->decorations_enable = Client::Ref().GetPrefBool("Renderer.Decorations", true);
	}
	catch(json::Exception & e)
	{
	}

	//Load config into simulation
	edgeMode = Client::Ref().GetPrefInteger("Simulation.EdgeMode", 0);
	sim->SetEdgeMode(edgeMode);
	int ngrav_enable = Client::Ref().GetPrefInteger("Simulation.NewtonianGravity", 0);
	if (ngrav_enable)
		sim->grav->start_grav_async();
	sim->aheat_enable =  Client::Ref().GetPrefInteger("Simulation.AmbientHeat", 0);
	sim->pretty_powder =  Client::Ref().GetPrefInteger("Simulation.PrettyPowder", 0);

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

	//Set default brush palette
	brushList.push_back(new EllipseBrush(ui::Point(4, 4)));
	brushList.push_back(new Brush(ui::Point(4, 4)));
	brushList.push_back(new TriangleBrush(ui::Point(4, 4)));

	//Load more from brushes folder
	std::vector<string> brushFiles = Client::Ref().DirectorySearch(BRUSH_DIR, "", ".ptb");
	for(int i = 0; i < brushFiles.size(); i++)
	{
		std::vector<unsigned char> brushData = Client::Ref().ReadFile(brushFiles[i]);
		if(!brushData.size())
		{
			std::cout << "Brushes: Skipping " << brushFiles[i] << ". Could not open" << std::endl;
			continue;
		}
		int dimension = std::sqrt((float)brushData.size());
		if(dimension * dimension != brushData.size())
		{
			std::cout << "Brushes: Skipping " << brushFiles[i] << ". Invalid bitmap size" << std::endl;
			continue;
		}
		brushList.push_back(new BitmapBrush(brushData, ui::Point(dimension, dimension)));
	}

	//Set default decoration colour
	unsigned char colourR = min(Client::Ref().GetPrefInteger("Decoration.Red", 200), 255);
	unsigned char colourG = min(Client::Ref().GetPrefInteger("Decoration.Green", 100), 255);
	unsigned char colourB = min(Client::Ref().GetPrefInteger("Decoration.Blue", 50), 255);
	unsigned char colourA = min(Client::Ref().GetPrefInteger("Decoration.Alpha", 255), 255);

	SetColourSelectorColour(ui::Colour(colourR, colourG, colourB, colourA));

	colourPresets.push_back(ui::Colour(255, 255, 255));
	colourPresets.push_back(ui::Colour(0, 255, 255));
	colourPresets.push_back(ui::Colour(255, 0, 255));
	colourPresets.push_back(ui::Colour(255, 255, 0));
	colourPresets.push_back(ui::Colour(255, 0, 0));
	colourPresets.push_back(ui::Colour(0, 255, 0));
	colourPresets.push_back(ui::Colour(0, 0, 255));
	colourPresets.push_back(ui::Colour(0, 0, 0));
}

GameModel::~GameModel()
{
	//Save to config:
	Client::Ref().SetPref("Renderer.ColourMode", ren->GetColourMode());

	std::vector<unsigned int> displayModes = ren->GetDisplayMode();
	Client::Ref().SetPref("Renderer.DisplayModes", std::vector<unsigned int>(displayModes.begin(), displayModes.end()));

	std::vector<unsigned int> renderModes = ren->GetRenderMode();
	Client::Ref().SetPref("Renderer.RenderModes", std::vector<unsigned int>(renderModes.begin(), renderModes.end()));

	Client::Ref().SetPref("Renderer.GravityField", (bool)ren->gravityFieldEnabled);
	Client::Ref().SetPref("Renderer.Decorations", (bool)ren->decorations_enable);
	Client::Ref().SetPref("Renderer.DebugMode", ren->debugLines); //These two should always be equivalent, even though they are different things

	Client::Ref().SetPref("Simulation.EdgeMode", sim->edgeMode);
	Client::Ref().SetPref("Simulation.NewtonianGravity", sim->grav->ngrav_enable);
	Client::Ref().SetPref("Simulation.AmbientHeat", sim->aheat_enable);
	Client::Ref().SetPref("Simulation.PrettyPowder", sim->pretty_powder);

	Client::Ref().SetPref("Decoration.Red", (int)colour.Red);
	Client::Ref().SetPref("Decoration.Green", (int)colour.Green);
	Client::Ref().SetPref("Decoration.Blue", (int)colour.Blue);
	Client::Ref().SetPref("Decoration.Alpha", (int)colour.Alpha);

	for(int i = 0; i < menuList.size(); i++)
	{
		delete menuList[i];
	}
	for(std::vector<Tool*>::iterator iter = extraElementTools.begin(), end = extraElementTools.end(); iter != end; ++iter)
	{
		delete *iter;
	}
	for(int i = 0; i < brushList.size(); i++)
	{
		delete brushList[i];
	}
	delete sim;
	delete ren;
	if(placeSave)
		delete placeSave;
	if(clipboard)
		delete clipboard;
	if(stamp)
		delete stamp;
	if(currentSave)
		delete currentSave;
	if(currentFile)
		delete currentFile;
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

void GameModel::BuildQuickOptionMenu(GameController * controller)
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
	quickOptions.push_back(new ConsoleShowOption(this, controller));

	notifyQuickOptionsChanged();
	UpdateQuickOptions();
}

void GameModel::BuildMenus()
{
	int lastMenu = -1;
	if(activeMenu != -1)
		lastMenu = activeMenu;

	std::string activeToolIdentifiers[4];
	if(regularToolset[0])
		activeToolIdentifiers[0] = regularToolset[0]->GetIdentifier();
	if(regularToolset[1])
		activeToolIdentifiers[1] = regularToolset[1]->GetIdentifier();
	if(regularToolset[2])
		activeToolIdentifiers[2] = regularToolset[2]->GetIdentifier();
	if(regularToolset[3])
		activeToolIdentifiers[3] = regularToolset[3]->GetIdentifier();

	//Empty current menus
	for(std::vector<Menu*>::iterator iter = menuList.begin(), end = menuList.end(); iter != end; ++iter)
	{
		delete *iter;
	}
	menuList.clear();
	toolList.clear();

	for(std::vector<Tool*>::iterator iter = extraElementTools.begin(), end = extraElementTools.end(); iter != end; ++iter)
	{
		delete *iter;
	}
	extraElementTools.clear();
	elementTools.clear();

	//Create menus
	for(int i = 0; i < SC_TOTAL; i++)
	{
		menuList.push_back(new Menu((const char)sim->msections[i].icon[0], sim->msections[i].name));
	}

	//Build menus from Simulation elements
	for(int i = 0; i < PT_NUM; i++)
	{
		if(sim->elements[i].Enabled)
		{
			Tool * tempTool;
			if(i == PT_LIGH)
			{
				tempTool = new Element_LIGH_Tool(i, sim->elements[i].Name, sim->elements[i].Description, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), sim->elements[i].Identifier, sim->elements[i].IconGenerator);
			}
			else if(i == PT_TESC)
			{
				tempTool = new Element_TESC_Tool(i, sim->elements[i].Name, sim->elements[i].Description, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), sim->elements[i].Identifier, sim->elements[i].IconGenerator);
			}
			else if(i == PT_STKM || i == PT_FIGH || i == PT_STKM2)
			{
				tempTool = new PlopTool(i, sim->elements[i].Name, sim->elements[i].Description, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), sim->elements[i].Identifier, sim->elements[i].IconGenerator);
			}
			else
			{
				tempTool = new ElementTool(i, sim->elements[i].Name, sim->elements[i].Description, PIXR(sim->elements[i].Colour), PIXG(sim->elements[i].Colour), PIXB(sim->elements[i].Colour), sim->elements[i].Identifier, sim->elements[i].IconGenerator);
			}

			if(sim->elements[i].MenuSection < SC_TOTAL && sim->elements[i].MenuVisible)
			{
				menuList[sim->elements[i].MenuSection]->AddTool(tempTool);
			}
			else
			{
				extraElementTools.push_back(tempTool);
			}
			elementTools.push_back(tempTool);
		}
	}

	//Build menu for GOL types
	for(int i = 0; i < NGOL; i++)
	{
		Tool * tempTool = new ElementTool(PT_LIFE|(i<<8), sim->gmenu[i].name, std::string(sim->gmenu[i].description), PIXR(sim->gmenu[i].colour), PIXG(sim->gmenu[i].colour), PIXB(sim->gmenu[i].colour), "DEFAULT_PT_LIFE_"+std::string(sim->gmenu[i].name));
		menuList[SC_LIFE]->AddTool(tempTool);
	}

	//Build other menus from wall data
	for(int i = 0; i < UI_WALLCOUNT; i++)
	{
		Tool * tempTool = new WallTool(i, "", std::string(sim->wtypes[i].descs), PIXR(sim->wtypes[i].colour), PIXG(sim->wtypes[i].colour), PIXB(sim->wtypes[i].colour), "DEFAULT_WL_"+format::NumberToString<int>(i), sim->wtypes[i].textureGen);
		menuList[SC_WALL]->AddTool(tempTool);
		//sim->wtypes[i]
	}
	
	//Build menu for tools
	for(int i = 0; i < sim->tools.size(); i++)
	{
		Tool * tempTool;
		tempTool = new Tool(i, sim->tools[i]->Name, sim->tools[i]->Description, PIXR(sim->tools[i]->Colour), PIXG(sim->tools[i]->Colour), PIXB(sim->tools[i]->Colour), sim->tools[i]->Identifier);
		menuList[SC_TOOL]->AddTool(tempTool);
	}
	//Add special sign and prop tools
	menuList[SC_TOOL]->AddTool(new WindTool(0, "WIND", "Creates air movement.", 64, 64, 64, "DEFAULT_UI_WIND"));
	menuList[SC_TOOL]->AddTool(new PropertyTool());
	menuList[SC_TOOL]->AddTool(new SignTool(this));
	menuList[SC_TOOL]->AddTool(new SampleTool(this));

	//Add decoration tools to menu
	menuList[SC_DECO]->AddTool(new DecorationTool(DECO_ADD, "ADD", "Colour blending: Add.", 0, 0, 0, "DEFAULT_DECOR_ADD"));
	menuList[SC_DECO]->AddTool(new DecorationTool(DECO_SUBTRACT, "SUB", "Colour blending: Subtract.", 0, 0, 0, "DEFAULT_DECOR_SUB"));
	menuList[SC_DECO]->AddTool(new DecorationTool(DECO_MULTIPLY, "MUL", "Colour blending: Multiply.", 0, 0, 0, "DEFAULT_DECOR_MUL"));
	menuList[SC_DECO]->AddTool(new DecorationTool(DECO_DIVIDE, "DIV", "Colour blending: Divide." , 0, 0, 0, "DEFAULT_DECOR_DIV"));
	menuList[SC_DECO]->AddTool(new DecorationTool(DECO_SMUDGE, "SMDG", "Smudge tool, blends surrounding deco together.", 0, 0, 0, "DEFAULT_DECOR_SMDG"));
	menuList[SC_DECO]->AddTool(new DecorationTool(DECO_CLEAR, "CLR", "Erase any set decoration.", 0, 0, 0, "DEFAULT_DECOR_CLR"));
	menuList[SC_DECO]->AddTool(new DecorationTool(DECO_DRAW, "SET", "Draw decoration (No blending).", 0, 0, 0, "DEFAULT_DECOR_SET"));
	decoToolset[0] = GetToolFromIdentifier("DEFAULT_DECOR_SET");
	decoToolset[1] = GetToolFromIdentifier("DEFAULT_DECOR_CLR");
	decoToolset[2] = GetToolFromIdentifier("DEFAULT_UI_SAMPLE");
	decoToolset[3] = GetToolFromIdentifier("DEFAULT_PT_NONE");

	//Set default tools
	regularToolset[0] = GetToolFromIdentifier("DEFAULT_PT_DUST");
	regularToolset[1] = GetToolFromIdentifier("DEFAULT_PT_NONE");
	regularToolset[2] = GetToolFromIdentifier("DEFAULT_UI_SAMPLE");
	regularToolset[3] = GetToolFromIdentifier("DEFAULT_PT_NONE");


	if(activeToolIdentifiers[0].length())
		regularToolset[0] = GetToolFromIdentifier(activeToolIdentifiers[0]);
	if(activeToolIdentifiers[1].length())
		regularToolset[1] = GetToolFromIdentifier(activeToolIdentifiers[1]);
	if(activeToolIdentifiers[2].length())
		regularToolset[2] = GetToolFromIdentifier(activeToolIdentifiers[2]);
	if(activeToolIdentifiers[3].length())
		regularToolset[3] = GetToolFromIdentifier(activeToolIdentifiers[3]);

	lastTool = activeTools[0];

	//Set default menu
	activeMenu = SC_POWDERS;

	if(lastMenu != -1)
		activeMenu = lastMenu;

	if(activeMenu != -1)
		toolList = menuList[activeMenu]->GetToolList();
	else
		toolList = std::vector<Tool*>();

	notifyMenuListChanged();
	notifyToolListChanged();
	notifyActiveToolsChanged();
	notifyLastToolChanged();
}

Tool * GameModel::GetToolFromIdentifier(std::string identifier)
{
	for (std::vector<Menu*>::iterator iter = menuList.begin(), end = menuList.end(); iter != end; ++iter)
	{
		std::vector<Tool*> menuTools = (*iter)->GetToolList();
		for (std::vector<Tool*>::iterator titer = menuTools.begin(), tend = menuTools.end(); titer != tend; ++titer)
		{
			if (identifier == (*titer)->GetIdentifier())
				return *titer;
		}
	}
	for (std::vector<Tool*>::iterator iter = extraElementTools.begin(), end = extraElementTools.end(); iter != end; ++iter)
	{
		if (identifier == (*iter)->GetIdentifier())
			return *iter;
	}

	return NULL;
}

void GameModel::SetEdgeMode(int edgeMode)
{
	this->edgeMode = edgeMode;
	sim->SetEdgeMode(edgeMode);
}

int GameModel::GetEdgeMode()
{
	return this->edgeMode;
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

vector<Brush*> GameModel::GetBrushList()
{
	return brushList;
}

int GameModel::GetBrushID()
{
	return currentBrush;
}

void GameModel::SetBrushID(int i)
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
	observer->NotifyColourPresetsChanged(this);
	observer->NotifyColourActivePresetChanged(this);
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

void GameModel::SetActiveMenu(int menuID)
{
	activeMenu = menuID;
	toolList = menuList[menuID]->GetToolList();
	notifyToolListChanged();

	if(menuID == SC_DECO)
	{
		if(activeTools != decoToolset)
		{
			activeTools = decoToolset;
			notifyActiveToolsChanged();
		}
	}
	else
	{
		if(activeTools != regularToolset)
		{
			activeTools = regularToolset;
			notifyActiveToolsChanged();
		}
	}
}

vector<Tool*> GameModel::GetUnlistedTools()
{
	return extraElementTools;
}

vector<Tool*> GameModel::GetToolList()
{
	return toolList;
}

int GameModel::GetActiveMenu()
{
	return activeMenu;
}

//Get an element tool from an element ID
Tool * GameModel::GetElementTool(int elementID)
{
#ifdef DEBUG
	std::cout << elementID << std::endl;
#endif
	for(std::vector<Tool*>::iterator iter = elementTools.begin(), end = elementTools.end(); iter != end; ++iter)
	{
		if((*iter)->GetToolID() == elementID)
			return *iter;
	}
	return NULL;
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
	if(currentFile)
		delete currentFile;
	currentFile = NULL;

	if(currentSave && currentSave->GetGameSave())
	{
		GameSave * saveData = currentSave->GetGameSave();
		SetPaused(saveData->paused | GetPaused());
		sim->gravityMode = saveData->gravityMode;
		sim->air->airMode = saveData->airMode;
		sim->legacy_enable = saveData->legacyEnable;
		sim->water_equal_test = saveData->waterEEnabled;
		sim->aheat_enable = saveData->aheatEnable;
		if(saveData->gravityEnable)
			sim->grav->start_grav_async();
		else
			sim->grav->stop_grav_async();
		sim->SetEdgeMode(0);
		sim->clear_sim();
		ren->ClearAccumulation();
		sim->Load(saveData);
	}
	notifySaveChanged();
	UpdateQuickOptions();
}

SaveFile * GameModel::GetSaveFile()
{
	return currentFile;
}

void GameModel::SetSaveFile(SaveFile * newSave)
{
	if(currentFile != newSave)
	{
		if(currentFile)
			delete currentFile;
		if(newSave == NULL)
			currentFile = NULL;
		else
			currentFile = new SaveFile(*newSave);
	}
	if (currentSave)
		delete currentSave;
	currentSave = NULL;

	if(newSave && newSave->GetGameSave())
	{
		GameSave * saveData = newSave->GetGameSave();
		SetPaused(saveData->paused | GetPaused());
		sim->gravityMode = saveData->gravityMode;
		sim->air->airMode = saveData->airMode;
		sim->legacy_enable = saveData->legacyEnable;
		sim->water_equal_test = saveData->waterEEnabled;
		sim->aheat_enable = saveData->aheatEnable;
		if(saveData->gravityEnable && !sim->grav->ngrav_enable)
		{
			sim->grav->start_grav_async();
		}
		else if(!saveData->gravityEnable && sim->grav->ngrav_enable)
		{
			sim->grav->stop_grav_async();
		}
		sim->SetEdgeMode(0);
		sim->clear_sim();
		ren->ClearAccumulation();
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

ui::Point GameModel::AdjustZoomCoords(ui::Point position)
{
	if (!GetZoomEnabled())
		return position;

	int zoomFactor = GetZoomFactor();
	ui::Point zoomWindowPosition = GetZoomWindowPosition();
	ui::Point zoomWindowSize = ui::Point(GetZoomSize()*zoomFactor, GetZoomSize()*zoomFactor);

	if (position.X >= zoomWindowPosition.X && position.X >= zoomWindowPosition.Y && position.X <= zoomWindowPosition.X+zoomWindowSize.X && position.Y <= zoomWindowPosition.Y+zoomWindowSize.Y)
		return ((position-zoomWindowPosition)/GetZoomFactor())+GetZoomPosition();
	return position;
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

void GameModel::SetActiveColourPreset(int preset)
{
	if (activeColourPreset != preset)
		activeColourPreset = preset;
	else
	{
		activeTools[0] = GetToolFromIdentifier("DEFAULT_DECOR_SET");
		notifyActiveToolsChanged();
	}
	notifyColourActivePresetChanged();
}

int GameModel::GetActiveColourPreset()
{
	return activeColourPreset;
}

void GameModel::SetPresetColour(ui::Colour colour)
{
	if(activeColourPreset >= 0 && activeColourPreset < colourPresets.size())
	{
		colourPresets[activeColourPreset] = colour;
		notifyColourPresetsChanged();
	}
}

std::vector<ui::Colour> GameModel::GetColourPresets()
{
	return colourPresets;
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

	vector<Tool*> tools = GetMenuList()[SC_DECO]->GetToolList();
	for(int i = 0; i < tools.size(); i++)
	{
		((DecorationTool*)tools[i])->Red = colour.Red;
		((DecorationTool*)tools[i])->Green = colour.Green;
		((DecorationTool*)tools[i])->Blue = colour.Blue;
		((DecorationTool*)tools[i])->Alpha = colour.Alpha;
	}

	notifyColourSelectorColourChanged();
}

ui::Colour GameModel::GetColourSelectorColour()
{
	return colour;
}

void GameModel::SetUser(User user)
{
	currentUser = user;
	//Client::Ref().SetAuthUser(user);
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
	if (ren->decorations_enable != decorationState)
	{
		ren->decorations_enable = decorationState?1:0;
		notifyDecorationChanged();
		UpdateQuickOptions();
		if (decorationState)
			SetInfoTip("Decorations Layer: On");
		else
			SetInfoTip("Decorations Layer: Off");
	}
}

bool GameModel::GetDecoration()
{
	return ren->decorations_enable?true:false;
}

void GameModel::SetAHeatEnable(bool aHeat)
{
	sim->aheat_enable = aHeat;
	UpdateQuickOptions();
	if (aHeat)
		SetInfoTip("Ambient Heat: On");
	else
		SetInfoTip("Ambient Heat: Off");
}

bool GameModel::GetAHeatEnable()
{
	return sim->aheat_enable;
}

void GameModel::ShowGravityGrid(bool showGrid)
{
	ren->gravityFieldEnabled = showGrid;
	if (showGrid)
		SetInfoTip("Gravity Grid: On");
	else
		SetInfoTip("Gravity Grid: Off");
}

bool GameModel::GetGravityGrid()
{
	return ren->gravityFieldEnabled;
}

void GameModel::FrameStep(int frames)
{
	sim->framerender += frames;
}

void GameModel::ClearSimulation()
{
	//Load defaults
	sim->gravityMode = 0;
	sim->air->airMode = 0;
	sim->legacy_enable = false;
	sim->water_equal_test = false;
	sim->SetEdgeMode(edgeMode);

	sim->clear_sim();
	ren->ClearAccumulation();

	notifySaveChanged();
	UpdateQuickOptions();
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
	{
		if(placeSave)
			delete placeSave;
		if(save)
			placeSave = new GameSave(*save);
		else
			placeSave = NULL;
	}
	notifyPlaceSaveChanged();
}

std::string GameModel::AddStamp(GameSave * save)
{
	if(stamp)
		delete stamp;
	stamp = save;
	return Client::Ref().AddStamp(save);
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
	std::cout << message << std::endl;
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
			delete *iter;
			notifications.erase(iter);
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

void GameModel::notifyColourPresetsChanged()
{
	for(std::vector<GameView*>::iterator iter = observers.begin(); iter != observers.end(); ++iter)
	{
		(*iter)->NotifyColourPresetsChanged(this);
	}
}

void GameModel::notifyColourActivePresetChanged()
{
	for(std::vector<GameView*>::iterator iter = observers.begin(); iter != observers.end(); ++iter)
	{
		(*iter)->NotifyColourActivePresetChanged(this);
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
