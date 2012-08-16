#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <vector>
#include <deque>
#include "client/SaveInfo.h"
#include "simulation/Simulation.h"
#include "interface/Colour.h"
#include "graphics/Renderer.h"
#include "GameView.h"
#include "Brush.h"
#include "client/User.h"
#include "Notification.h"

#include "Tool.h"
#include "Menu.h"

using namespace std;

class GameView;
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
	vector<Menu*> menuList;
	vector<QuickOption*> quickOptions;
	Menu * activeMenu;
	int currentBrush;
	vector<Brush *> brushList;
	SaveInfo * currentSave;
	Simulation * sim;
	Renderer * ren;
	Tool * lastTool;
	Tool * activeTools[3];
	User currentUser;
	bool colourSelector;
	ui::Colour colour;
	float toolStrength;
	std::deque<Snapshot*> history;

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
	void notifyNotificationsChanged();
	void notifyLogChanged(string entry);
	void notifyInfoTipChanged();
	void notifyToolTipChanged();
	void notifyQuickOptionsChanged();
	void notifyLastToolChanged();
public:
	GameModel();
	~GameModel();

	void SetColourSelectorVisibility(bool visibility);
	bool GetColourSelectorVisibility();

	void SetColourSelectorColour(ui::Colour colour);
	ui::Colour GetColourSelectorColour();

	void SetToolTip(std::string text);
	void SetInfoTip(std::string text);
	std::string GetToolTip();
	std::string GetInfoTip();

	void BuildMenus();
	void BuildQuickOptionMenu();

	std::deque<Snapshot*> GetHistory();
	void SetHistory(std::deque<Snapshot*> newHistory);

	void UpdateQuickOptions();

	void SetToolStrength(float value);
	float GetToolStrength();

	Tool * GetLastTool();
	void SetLastTool(Tool * newTool);

	void SetVote(int direction);
	SaveInfo * GetSave();
	Brush * GetBrush();
	void SetSave(SaveInfo * newSave);
	void SetSaveFile(SaveFile * newSave);
	void AddObserver(GameView * observer);
	Tool * GetActiveTool(int selection);
	void SetActiveTool(int selection, Tool * tool);
	bool GetPaused();
	void SetPaused(bool pauseState);
	bool GetDecoration();
	void SetDecoration(bool decorationState);
	void ClearSimulation();
	vector<Menu*> GetMenuList();
	vector<Tool*> GetToolList();
	vector<QuickOption*> GetQuickOptions();
	void SetActiveMenu(Menu * menu);
	Menu * GetActiveMenu();
	void FrameStep(int frames);
	User GetUser();
	void SetUser(User user);
	void SetBrush(int i);
	int GetBrushID();
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
	void AddStamp(GameSave * save);
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
