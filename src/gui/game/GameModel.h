#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <vector>
#include <deque>
#include "client/SaveInfo.h"
#include "simulation/Simulation.h"
#include "gui/interface/Colour.h"
#include "graphics/Renderer.h"
#include "GameView.h"
#include "GameController.h"
#include "Brush.h"
#include "client/User.h"
#include "Notification.h"

#include "Tool.h"
#include "Menu.h"

using namespace std;

class GameView;
class GameController;
class Simulation;
class Renderer;

class QuickOption;
class ToolSelection
{
public:
	enum
	{
		ToolPrimary, ToolSecondary, ToolTertiary
	};
};

class GameModel
{
private:
	vector<Notification*> notifications;
	//int clipboardSize;
	//unsigned char * clipboardData;
	GameSave * stamp;
	GameSave * clipboard;
	GameSave * placeSave;
	deque<string> consoleLog;
	vector<GameView*> observers;
	vector<Tool*> toolList;

	//All tools that are associated with elements
	vector<Tool*> elementTools;
	//Tools that are present in elementTools, but don't have an associated menu and need to be freed manually
	vector<Tool*> extraElementTools;

	vector<Menu*> menuList;
	vector<QuickOption*> quickOptions;
	int activeMenu;
	int currentBrush;
	vector<Brush *> brushList;
	SaveInfo * currentSave;
	SaveFile * currentFile;
	Simulation * sim;
	Renderer * ren;
	Tool * lastTool;
	Tool ** activeTools;
	Tool * decoToolset[3];
	Tool * regularToolset[3];
	User currentUser;
	float toolStrength;
	std::deque<Snapshot*> history;

	int activeColourPreset;
	std::vector<ui::Colour> colourPresets;
	bool colourSelector;
	ui::Colour colour;

	int edgeMode;

	std::string infoTip;
	std::string toolTip;
	//bool zoomEnabled;
	void notifyRendererChanged();
	void notifySimulationChanged();
	void notifyPausedChanged();
	void notifyDecorationChanged();
	void notifySaveChanged();
	void notifyBrushChanged();
	void notifyMenuListChanged();
	void notifyToolListChanged();
	void notifyActiveToolsChanged();
	void notifyUserChanged();
	void notifyZoomChanged();
	void notifyClipboardChanged();
	void notifyPlaceSaveChanged();
	void notifyColourSelectorColourChanged();
	void notifyColourSelectorVisibilityChanged();
	void notifyColourPresetsChanged();
	void notifyColourActivePresetChanged();
	void notifyNotificationsChanged();
	void notifyLogChanged(string entry);
	void notifyInfoTipChanged();
	void notifyToolTipChanged();
	void notifyQuickOptionsChanged();
	void notifyLastToolChanged();
public:
	GameModel();
	~GameModel();

	void SetEdgeMode(int edgeMode);
	int GetEdgeMode();

	void SetActiveColourPreset(int preset);
	int GetActiveColourPreset();

	void SetPresetColour(ui::Colour colour);

	std::vector<ui::Colour> GetColourPresets();

	void SetColourSelectorVisibility(bool visibility);
	bool GetColourSelectorVisibility();

	void SetColourSelectorColour(ui::Colour colour);
	ui::Colour GetColourSelectorColour();

	void SetToolTip(std::string text);
	void SetInfoTip(std::string text);
	std::string GetToolTip();
	std::string GetInfoTip();

	void BuildMenus();
	void BuildQuickOptionMenu(GameController * controller);

	std::deque<Snapshot*> GetHistory();
	void SetHistory(std::deque<Snapshot*> newHistory);

	void UpdateQuickOptions();

	Tool * GetActiveTool(int selection);
	void SetActiveTool(int selection, Tool * tool);
	void SetToolStrength(float value);
	float GetToolStrength();
	Tool * GetLastTool();
	void SetLastTool(Tool * newTool);
	Tool * GetToolFromIdentifier(std::string identifier);
	Tool * GetElementTool(int elementID);
	vector<Tool*> GetToolList();
	vector<Tool*> GetUnlistedTools();

	Brush * GetBrush();
	vector<Brush*> GetBrushList();
	int GetBrushID();
	void SetBrushID(int i);

	void SetVote(int direction);
	SaveInfo * GetSave();
	SaveFile * GetSaveFile();
	void SetSave(SaveInfo * newSave);
	void SetSaveFile(SaveFile * newSave);
	void AddObserver(GameView * observer);

	bool GetPaused();
	void SetPaused(bool pauseState);
	bool GetDecoration();
	void SetDecoration(bool decorationState);
	bool GetAHeatEnable();
	void SetAHeatEnable(bool aHeat);
	bool GetGravityGrid();
	void ShowGravityGrid(bool showGrid);
	void ClearSimulation();
	vector<Menu*> GetMenuList();
	vector<QuickOption*> GetQuickOptions();
	void SetActiveMenu(int menuID);
	int GetActiveMenu();
	void FrameStep(int frames);
	User GetUser();
	void SetUser(User user);
	Simulation * GetSimulation();
	Renderer * GetRenderer();
	void SetZoomEnabled(bool enabled);
	bool GetZoomEnabled();
	void SetZoomSize(int size);
	int GetZoomSize();
	void SetZoomFactor(int factor);
	int GetZoomFactor();
	void SetZoomPosition(ui::Point position);
	ui::Point GetZoomPosition();
	void SetZoomWindowPosition(ui::Point position);
	ui::Point GetZoomWindowPosition();
	void SetStamp(GameSave * newStamp);
	std::string AddStamp(GameSave * save);
	void SetClipboard(GameSave * save);
	void SetPlaceSave(GameSave * save);
	void Log(string message);
	deque<string> GetLog();
	GameSave * GetClipboard();
	GameSave * GetStamp();
	GameSave * GetPlaceSave();

	std::vector<Notification*> GetNotifications();
	void AddNotification(Notification * notification);
	void RemoveNotification(Notification * notification);
};

#endif // GAMEMODEL_H
