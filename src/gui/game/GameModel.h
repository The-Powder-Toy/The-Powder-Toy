#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <vector>
#include <deque>

#include "gui/interface/Colour.h"
#include "client/User.h"
#include "gui/interface/Point.h"

class Menu;
class Tool;
class QuickOption;
class Brush;
class GameView;
class Notification;
class GameController;
class SaveInfo;
class SaveFile;
class Simulation;
class Renderer;
class Snapshot;
class GameSave;

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
	std::vector<Notification*> notifications;
	//int clipboardSize;
	//unsigned char * clipboardData;
	GameSave * clipboard;
	GameSave * placeSave;
	std::deque<String> consoleLog;
	std::vector<GameView*> observers;
	std::vector<Tool*> toolList;

	//All tools that are associated with elements
	std::vector<Tool*> elementTools;
	//Tools that are present in elementTools, but don't have an associated menu and need to be freed manually
	std::vector<Tool*> extraElementTools;

	Simulation * sim;
	Renderer * ren;
	std::vector<Menu*> menuList;
	std::vector<QuickOption*> quickOptions;
	int activeMenu;
	int currentBrush;
	std::vector<Brush *> brushList;
	SaveInfo * currentSave;
	SaveFile * currentFile;
	Tool * lastTool;
	Tool ** activeTools;
	Tool * decoToolset[4];
	Tool * regularToolset[4];
	User currentUser;
	float toolStrength;
	std::deque<Snapshot*> history;
	Snapshot *redoHistory;
	unsigned int historyPosition;
	unsigned int undoHistoryLimit;
	bool mouseClickRequired;
	bool includePressure;
	bool perfectCircle = true;

	size_t activeColourPreset;
	std::vector<ui::Colour> colourPresets;
	bool colourSelector;
	ui::Colour colour;

	int edgeMode;
	int decoSpace;

	String infoTip;
	String toolTip;
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
	void notifyLogChanged(String entry);
	void notifyInfoTipChanged();
	void notifyToolTipChanged();
	void notifyQuickOptionsChanged();
	void notifyLastToolChanged();
public:
	GameModel();
	~GameModel();

	void SetEdgeMode(int edgeMode);
	int GetEdgeMode();
	void SetDecoSpace(int decoSpace);
	int GetDecoSpace();

	void SetActiveColourPreset(size_t preset);
	size_t GetActiveColourPreset();

	void SetPresetColour(ui::Colour colour);

	std::vector<ui::Colour> GetColourPresets();

	void SetColourSelectorVisibility(bool visibility);
	bool GetColourSelectorVisibility();

	void SetColourSelectorColour(ui::Colour colour);
	ui::Colour GetColourSelectorColour();

	void SetToolTip(String text);
	void SetInfoTip(String text);
	String GetToolTip();
	String GetInfoTip();

	void BuildMenus();
	void BuildFavoritesMenu();
	void BuildBrushList();
	void BuildQuickOptionMenu(GameController * controller);

	std::deque<Snapshot*> GetHistory();
	unsigned int GetHistoryPosition();
	void SetHistory(std::deque<Snapshot*> newHistory);
	void SetHistoryPosition(unsigned int newHistoryPosition);
	Snapshot * GetRedoHistory();
	void SetRedoHistory(Snapshot * redo);
	unsigned int GetUndoHistoryLimit();
	void SetUndoHistoryLimit(unsigned int undoHistoryLimit_);

	void UpdateQuickOptions();

	Tool * GetActiveTool(int selection);
	void SetActiveTool(int selection, Tool * tool);
	void SetToolStrength(float value);
	float GetToolStrength();
	Tool * GetLastTool();
	void SetLastTool(Tool * newTool);
	Tool *GetToolFromIdentifier(ByteString const &identifier);
	Tool * GetElementTool(int elementID);
	std::vector<Tool*> GetToolList();
	std::vector<Tool*> GetUnlistedTools();

	Brush * GetBrush();
	std::vector<Brush*> GetBrushList();
	int GetBrushID();
	void SetBrushID(int i);

	void SetVote(int direction);
	SaveInfo * GetSave();
	SaveFile * GetSaveFile();
	void SetSave(SaveInfo * newSave, bool invertIncludePressure);
	void SetSaveFile(SaveFile * newSave, bool invertIncludePressure);
	void AddObserver(GameView * observer);

	void SetPaused(bool pauseState);
	bool GetPaused();
	void SetDecoration(bool decorationState);
	bool GetDecoration();
	void SetAHeatEnable(bool aHeat);
	bool GetAHeatEnable();
	void ResetAHeat();
	void SetNewtonianGravity(bool newtonainGravity);
	bool GetNewtonianGrvity();
	void ShowGravityGrid(bool showGrid);
	bool GetGravityGrid();
	void ClearSimulation();
	std::vector<Menu*> GetMenuList();
	std::vector<QuickOption*> GetQuickOptions();
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
	bool MouseInZoom(ui::Point position);
	ui::Point AdjustZoomCoords(ui::Point position);
	void SetZoomWindowPosition(ui::Point position);
	ui::Point GetZoomWindowPosition();
	void SetClipboard(GameSave * save);
	void SetPlaceSave(GameSave * save);
	void Log(String message, bool printToFile);
	std::deque<String> GetLog();
	GameSave * GetClipboard();
	GameSave * GetPlaceSave();
	bool GetMouseClickRequired();
	void SetMouseClickRequired(bool mouseClickRequired);
	bool GetIncludePressure();
	void SetIncludePressure(bool includePressure);
	void SetPerfectCircle(bool perfectCircle);

	std::vector<Notification*> GetNotifications();
	void AddNotification(Notification * notification);
	void RemoveNotification(Notification * notification);

	void RemoveCustomGOLType(const ByteString &identifier);

	ByteString SelectNextIdentifier;
	int SelectNextTool;
};

#endif // GAMEMODEL_H
