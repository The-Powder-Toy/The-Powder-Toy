#include "GameModel.h"
#include "GameView.h"
#include "ToolClasses.h"
#include "Brush.h"
#include "EllipseBrush.h"
#include "TriangleBrush.h"
#include "BitmapBrush.h"
#include "QuickOptions.h"
#include "GameModelException.h"
#include "Format.h"
#include "Favorite.h"

#include "client/Client.h"
#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "common/tpt-minmax.h"
#include "graphics/Renderer.h"
#include "simulation/Air.h"
#include "simulation/Simulation.h"
#include "simulation/Snapshot.h"

#include "gui/game/DecorationTool.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Point.h"


GameModel::GameModel():
	clipboard(NULL),
	placeSave(NULL),
	activeMenu(-1),
	currentBrush(0),
	currentSave(NULL),
	currentFile(NULL),
	currentUser(0, ""),
	toolStrength(1.0f),
	redoHistory(NULL),
	historyPosition(0),
	activeColourPreset(0),
	colourSelector(false),
	colour(255, 0, 0, 255),
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
	ren->SetColourMode(Client::Ref().GetPrefUInteger("Renderer.ColourMode", 0));

	tempArray = Client::Ref().GetPrefUIntegerArray("Renderer.DisplayModes");
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

	//Load config into simulation
	edgeMode = Client::Ref().GetPrefInteger("Simulation.EdgeMode", 0);
	sim->SetEdgeMode(edgeMode);
	int ngrav_enable = Client::Ref().GetPrefInteger("Simulation.NewtonianGravity", 0);
	if (ngrav_enable)
		sim->grav->start_grav_async();
	sim->aheat_enable =  Client::Ref().GetPrefInteger("Simulation.AmbientHeat", 0);
	sim->pretty_powder =  Client::Ref().GetPrefInteger("Simulation.PrettyPowder", 0);

	Favorite::Ref().LoadFavoritesFromPrefs();

	//Load last user
	if(Client::Ref().GetAuthUser().ID)
	{
		currentUser = Client::Ref().GetAuthUser();
	}

	BuildMenus();

	//Set default brush palette
	brushList.push_back(new EllipseBrush(ui::Point(4, 4)));
	brushList.push_back(new Brush(ui::Point(4, 4)));
	brushList.push_back(new TriangleBrush(ui::Point(4, 4)));

	//Load more from brushes folder
	std::vector<string> brushFiles = Client::Ref().DirectorySearch(BRUSH_DIR, "", ".ptb");
	for (size_t i = 0; i < brushFiles.size(); i++)
	{
		std::vector<unsigned char> brushData = Client::Ref().ReadFile(brushFiles[i]);
		if(!brushData.size())
		{
			std::cout << "Brushes: Skipping " << brushFiles[i] << ". Could not open" << std::endl;
			continue;
		}
		size_t dimension = std::sqrt((float)brushData.size());
		if (dimension * dimension != brushData.size())
		{
			std::cout << "Brushes: Skipping " << brushFiles[i] << ". Invalid bitmap size" << std::endl;
			continue;
		}
		brushList.push_back(new BitmapBrush(brushData, ui::Point(dimension, dimension)));
	}

	//Set default decoration colour
	unsigned char colourR = std::min(Client::Ref().GetPrefInteger("Decoration.Red", 200), 255);
	unsigned char colourG = std::min(Client::Ref().GetPrefInteger("Decoration.Green", 100), 255);
	unsigned char colourB = std::min(Client::Ref().GetPrefInteger("Decoration.Blue", 50), 255);
	unsigned char colourA = std::min(Client::Ref().GetPrefInteger("Decoration.Alpha", 255), 255);

	SetColourSelectorColour(ui::Colour(colourR, colourG, colourB, colourA));

	colourPresets.push_back(ui::Colour(255, 255, 255));
	colourPresets.push_back(ui::Colour(0, 255, 255));
	colourPresets.push_back(ui::Colour(255, 0, 255));
	colourPresets.push_back(ui::Colour(255, 255, 0));
	colourPresets.push_back(ui::Colour(255, 0, 0));
	colourPresets.push_back(ui::Colour(0, 255, 0));
	colourPresets.push_back(ui::Colour(0, 0, 255));
	colourPresets.push_back(ui::Colour(0, 0, 0));

	undoHistoryLimit = Client::Ref().GetPrefInteger("Simulation.UndoHistoryLimit", 5);
	// cap due to memory usage (this is about 3.4GB of RAM)
	if (undoHistoryLimit > 200)
		undoHistoryLimit = 200;
}

GameModel::~GameModel()
{
	//Save to config:
	Client::Ref().SetPref("Renderer.ColourMode", ren->GetColourMode());

	std::vector<unsigned int> displayModes = ren->GetDisplayMode();
	Client::Ref().SetPref("Renderer.DisplayModes", std::vector<Json::Value>(displayModes.begin(), displayModes.end()));

	std::vector<unsigned int> renderModes = ren->GetRenderMode();
	Client::Ref().SetPref("Renderer.RenderModes", std::vector<Json::Value>(renderModes.begin(), renderModes.end()));

	Client::Ref().SetPref("Renderer.GravityField", (bool)ren->gravityFieldEnabled);
	Client::Ref().SetPref("Renderer.Decorations", (bool)ren->decorations_enable);
	Client::Ref().SetPref("Renderer.DebugMode", ren->debugLines); //These two should always be equivalent, even though they are different things

	Client::Ref().SetPref("Simulation.EdgeMode", edgeMode);
	Client::Ref().SetPref("Simulation.NewtonianGravity", sim->grav->ngrav_enable);
	Client::Ref().SetPref("Simulation.AmbientHeat", sim->aheat_enable);
	Client::Ref().SetPref("Simulation.PrettyPowder", sim->pretty_powder);

	Client::Ref().SetPref("Decoration.Red", (int)colour.Red);
	Client::Ref().SetPref("Decoration.Green", (int)colour.Green);
	Client::Ref().SetPref("Decoration.Blue", (int)colour.Blue);
	Client::Ref().SetPref("Decoration.Alpha", (int)colour.Alpha);

	Client::Ref().SetPref("Simulation.UndoHistoryLimit", undoHistoryLimit);

	Favorite::Ref().SaveFavoritesToPrefs();

	for (size_t i = 0; i < menuList.size(); i++)
	{
		if (i == SC_FAVORITES)
			menuList[i]->ClearTools();
		delete menuList[i];
	}
	for (std::vector<Tool*>::iterator iter = extraElementTools.begin(), end = extraElementTools.end(); iter != end; ++iter)
	{
		delete *iter;
	}
	for (size_t i = 0; i < brushList.size(); i++)
	{
		delete brushList[i];
	}
	delete sim;
	delete ren;
	delete placeSave;
	delete clipboard;
	delete currentSave;
	delete currentFile;
	delete redoHistory;
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
	for (size_t i = 0; i < menuList.size(); i++)
	{
		if (i == SC_FAVORITES)
			menuList[i]->ClearTools();
		delete menuList[i];
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
	for (int i = 0; i < SC_TOTAL; i++)
	{
		menuList.push_back(new Menu((const char)sim->msections[i].icon[0], sim->msections[i].name, sim->msections[i].doshow));
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

			if (sim->elements[i].MenuSection >= 0 && sim->elements[i].MenuSection < SC_TOTAL && sim->elements[i].MenuVisible)
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
		Tool * tempTool = new WallTool(i, "", std::string(sim->wtypes[i].descs), PIXR(sim->wtypes[i].colour), PIXG(sim->wtypes[i].colour), PIXB(sim->wtypes[i].colour), sim->wtypes[i].identifier, sim->wtypes[i].textureGen);
		menuList[SC_WALL]->AddTool(tempTool);
		//sim->wtypes[i]
	}
	
	//Build menu for tools
	for (size_t i = 0; i < sim->tools.size(); i++)
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
	menuList[SC_DECO]->AddTool(new DecorationTool(ren, DECO_ADD, "ADD", "Colour blending: Add.", 0, 0, 0, "DEFAULT_DECOR_ADD"));
	menuList[SC_DECO]->AddTool(new DecorationTool(ren, DECO_SUBTRACT, "SUB", "Colour blending: Subtract.", 0, 0, 0, "DEFAULT_DECOR_SUB"));
	menuList[SC_DECO]->AddTool(new DecorationTool(ren, DECO_MULTIPLY, "MUL", "Colour blending: Multiply.", 0, 0, 0, "DEFAULT_DECOR_MUL"));
	menuList[SC_DECO]->AddTool(new DecorationTool(ren, DECO_DIVIDE, "DIV", "Colour blending: Divide." , 0, 0, 0, "DEFAULT_DECOR_DIV"));
	menuList[SC_DECO]->AddTool(new DecorationTool(ren, DECO_SMUDGE, "SMDG", "Smudge tool, blends surrounding deco together.", 0, 0, 0, "DEFAULT_DECOR_SMDG"));
	menuList[SC_DECO]->AddTool(new DecorationTool(ren, DECO_CLEAR, "CLR", "Erase any set decoration.", 0, 0, 0, "DEFAULT_DECOR_CLR"));
	menuList[SC_DECO]->AddTool(new DecorationTool(ren, DECO_DRAW, "SET", "Draw decoration (No blending).", 0, 0, 0, "DEFAULT_DECOR_SET"));
	SetColourSelectorColour(colour); // update tool colors
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

	//Build menu for favorites
	BuildFavoritesMenu();
}

void GameModel::BuildFavoritesMenu()
{
	menuList[SC_FAVORITES]->ClearTools();
	
	std::vector<std::string> favList = Favorite::Ref().GetFavoritesList();
	for (size_t i = 0; i < favList.size(); i++)
	{
		Tool *tool = GetToolFromIdentifier(favList[i]);
		if (tool)
			menuList[SC_FAVORITES]->AddTool(tool);
	}

	if (activeMenu == SC_FAVORITES)
		toolList = menuList[SC_FAVORITES]->GetToolList();

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

unsigned int GameModel::GetHistoryPosition()
{
	return historyPosition;
}

void GameModel::SetHistory(std::deque<Snapshot*> newHistory)
{
	history = newHistory;
}

void GameModel::SetHistoryPosition(unsigned int newHistoryPosition)
{
	historyPosition = newHistoryPosition;
}

Snapshot * GameModel::GetRedoHistory()
{
	return redoHistory;
}

void GameModel::SetRedoHistory(Snapshot * redo)
{
	redoHistory = redo;
}

unsigned int GameModel::GetUndoHistoryLimit()
{
	return undoHistoryLimit;
}

void GameModel::SetUndoHistoryLimit(unsigned int undoHistoryLimit_)
{
	undoHistoryLimit = undoHistoryLimit_;
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
		delete currentSave;
		if(newSave == NULL)
			currentSave = NULL;
		else
			currentSave = new SaveInfo(*newSave);
	}
	delete currentFile;
	currentFile = NULL;

	if(currentSave && currentSave->GetGameSave())
	{
		GameSave * saveData = currentSave->GetGameSave();
		SetPaused(saveData->paused | GetPaused());
		sim->gravityMode = saveData->gravityMode;
		sim->air->airMode = saveData->airMode;
		sim->edgeMode = saveData->edgeMode;
		sim->legacy_enable = saveData->legacyEnable;
		sim->water_equal_test = saveData->waterEEnabled;
		sim->aheat_enable = saveData->aheatEnable;
		if(saveData->gravityEnable)
			sim->grav->start_grav_async();
		else
			sim->grav->stop_grav_async();
		sim->clear_sim();
		ren->ClearAccumulation();
		if (!sim->Load(saveData))
		{
			// This save was created before logging existed
			// Add in the correct info
			if (saveData->authors.size() == 0)
			{
				saveData->authors["type"] = "save";
				saveData->authors["id"] = newSave->id;
				saveData->authors["username"] = newSave->userName;
				saveData->authors["title"] = newSave->name;
				saveData->authors["description"] = newSave->Description;
				saveData->authors["published"] = (int)newSave->Published;
				saveData->authors["date"] = newSave->updatedDate;
			}
			// This save was probably just created, and we didn't know the ID when creating it
			// Update with the proper ID
			else if (saveData->authors.get("id", -1) == 0 || saveData->authors.get("id", -1) == -1)
			{
				saveData->authors["id"] = newSave->id;
			}
			Client::Ref().OverwriteAuthorInfo(saveData->authors);
		}
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
		delete currentFile;
		if(newSave == NULL)
			currentFile = NULL;
		else
			currentFile = new SaveFile(*newSave);
	}
	delete currentSave;
	currentSave = NULL;

	if(newSave && newSave->GetGameSave())
	{
		GameSave * saveData = newSave->GetGameSave();
		SetPaused(saveData->paused | GetPaused());
		sim->gravityMode = saveData->gravityMode;
		sim->air->airMode = saveData->airMode;
		sim->edgeMode = saveData->edgeMode;
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
		sim->clear_sim();
		ren->ClearAccumulation();
		if (!sim->Load(saveData))
		{
			Client::Ref().OverwriteAuthorInfo(saveData->authors);
		}
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

bool GameModel::MouseInZoom(ui::Point position)
{
	if (!GetZoomEnabled())
		return false;

	int zoomFactor = GetZoomFactor();
	ui::Point zoomWindowPosition = GetZoomWindowPosition();
	ui::Point zoomWindowSize = ui::Point(GetZoomSize()*zoomFactor, GetZoomSize()*zoomFactor);

	if (position.X >= zoomWindowPosition.X && position.X >= zoomWindowPosition.Y && position.X <= zoomWindowPosition.X+zoomWindowSize.X && position.Y <= zoomWindowPosition.Y+zoomWindowSize.Y)
		return true;
	return false;
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

void GameModel::SetActiveColourPreset(size_t preset)
{
	if (activeColourPreset-1 != preset)
		activeColourPreset = preset+1;
	else
	{
		activeTools[0] = GetToolFromIdentifier("DEFAULT_DECOR_SET");
		notifyActiveToolsChanged();
	}
	notifyColourActivePresetChanged();
}

size_t GameModel::GetActiveColourPreset()
{
	return activeColourPreset-1;
}

void GameModel::SetPresetColour(ui::Colour colour)
{
	if (activeColourPreset > 0 && activeColourPreset <= colourPresets.size())
	{
		colourPresets[activeColourPreset-1] = colour;
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
	for (size_t i = 0; i < tools.size(); i++)
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
	if (!pauseState && sim->debug_currentParticle > 0)
	{
		std::stringstream logmessage;
		logmessage << "Updated particles from #" << sim->debug_currentParticle << " to end due to unpause";
		sim->UpdateParticles(sim->debug_currentParticle, NPART);
		sim->AfterSim();
		sim->debug_currentParticle = 0;
		Log(logmessage.str(), false);
	}

	sim->sys_pause = pauseState?1:0;
	notifyPausedChanged();
}

bool GameModel::GetPaused()
{
	return sim->sys_pause?true:false;
}

void GameModel::SetDecoration(bool decorationState)
{
	if (ren->decorations_enable != (decorationState?1:0))
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
	Client::Ref().ClearAuthorInfo();

	notifySaveChanged();
	UpdateQuickOptions();
}

void GameModel::SetPlaceSave(GameSave * save)
{
	if(save != placeSave)
	{
		delete placeSave;
		if(save)
			placeSave = new GameSave(*save);
		else
			placeSave = NULL;
	}
	notifyPlaceSaveChanged();
}

void GameModel::SetClipboard(GameSave * save)
{
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

void GameModel::Log(string message, bool printToFile)
{
	consoleLog.push_front(message);
	if(consoleLog.size()>100)
		consoleLog.pop_back();
	notifyLogChanged(message);
	if (printToFile)
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
	for (std::vector<GameView*>::iterator iter = observers.begin(); iter != observers.end(); ++iter)
	{
		(*iter)->NotifyNotificationsChanged(this);
	}
}

void GameModel::notifyColourPresetsChanged()
{
	for (std::vector<GameView*>::iterator iter = observers.begin(); iter != observers.end(); ++iter)
	{
		(*iter)->NotifyColourPresetsChanged(this);
	}
}

void GameModel::notifyColourActivePresetChanged()
{
	for (std::vector<GameView*>::iterator iter = observers.begin(); iter != observers.end(); ++iter)
	{
		(*iter)->NotifyColourActivePresetChanged(this);
	}
}

void GameModel::notifyColourSelectorColourChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyColourSelectorColourChanged(this);
	}
}

void GameModel::notifyColourSelectorVisibilityChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyColourSelectorVisibilityChanged(this);
	}
}

void GameModel::notifyRendererChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyRendererChanged(this);
	}
}

void GameModel::notifySaveChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySaveChanged(this);
	}
}

void GameModel::notifySimulationChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySimulationChanged(this);
	}
}

void GameModel::notifyPausedChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPausedChanged(this);
	}
}

void GameModel::notifyDecorationChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		//observers[i]->NotifyPausedChanged(this);
	}
}

void GameModel::notifyBrushChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyBrushChanged(this);
	}
}

void GameModel::notifyMenuListChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyMenuListChanged(this);
	}
}

void GameModel::notifyToolListChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyToolListChanged(this);
	}
}

void GameModel::notifyActiveToolsChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyActiveToolsChanged(this);
	}
}

void GameModel::notifyUserChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyUserChanged(this);
	}
}

void GameModel::notifyZoomChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyZoomChanged(this);
	}
}

void GameModel::notifyPlaceSaveChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPlaceSaveChanged(this);
	}
}

void GameModel::notifyLogChanged(string entry)
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyLogChanged(this, entry);
	}
}

void GameModel::notifyInfoTipChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyInfoTipChanged(this);
	}
}

void GameModel::notifyToolTipChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyToolTipChanged(this);
	}
}

void GameModel::notifyQuickOptionsChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyQuickOptionsChanged(this);
	}
}

void GameModel::notifyLastToolChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyLastToolChanged(this);
	}
}
