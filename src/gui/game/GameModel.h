#pragma once
#include "gui/interface/Colour.h"
#include "client/User.h"
#include "gui/interface/Point.h"
#include "graphics/RendererSettings.h"
#include "simulation/CustomGOLData.h"
#include <vector>
#include <deque>
#include <memory>
#include <optional>
#include <array>

constexpr auto NUM_TOOLINDICES = 4;

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
struct SnapshotDelta;
class GameSave;

namespace http
{
	class ExecVoteRequest;
};

class ToolSelection
{
public:
	enum
	{
		ToolPrimary, ToolSecondary, ToolTertiary
	};
};

struct HistoryEntry
{
	std::unique_ptr<Snapshot> snap;
	std::unique_ptr<SnapshotDelta> delta;

	~HistoryEntry();
};

class GameModel
{
	std::unique_ptr<http::ExecVoteRequest> execVoteRequest;

private:
	std::vector<Notification*> notifications;
	std::unique_ptr<GameSave> placeSave;
	std::unique_ptr<GameSave> transformedPlaceSave;
	std::deque<String> consoleLog;
	std::vector<GameView*> observers;

	std::vector<std::unique_ptr<Tool>> tools;

	void SanitizeToolsets();
	void DeselectTool(ByteString identifier);
	void InitTools();

	Simulation * sim;
	Renderer * ren;
	RendererSettings rendererSettings;
	std::vector<std::unique_ptr<Menu>> menuList;
	std::vector<QuickOption*> quickOptions;
	int activeMenu;
	int currentBrush;
	std::vector<std::unique_ptr<Brush>> brushList;
	std::unique_ptr<SaveInfo> currentSave;
	std::unique_ptr<SaveFile> currentFile;
	Tool *lastTool = nullptr;
	Tool **activeTools = nullptr;
	std::array<Tool *, NUM_TOOLINDICES> decoToolset;
	std::array<Tool *, NUM_TOOLINDICES> regularToolset;
	User currentUser;
	float toolStrength;
	std::deque<HistoryEntry> history;
	std::unique_ptr<Snapshot> historyCurrent;
	unsigned int historyPosition;
	unsigned int undoHistoryLimit;
	bool mouseClickRequired;
	bool includePressure;
	bool perfectCircle = true;
	int temperatureScale;

	size_t activeColourPreset;
	std::vector<ui::Colour> colourPresets;
	bool colourSelector;
	ui::Colour colour;

	int edgeMode;
	float ambientAirTemp;
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
	void notifyActiveMenuToolListChanged();
	void notifyActiveToolsChanged();
	void notifyUserChanged();
	void notifyZoomChanged();
	void notifyClipboardChanged();
	void notifyPlaceSaveChanged();
	void notifyTransformedPlaceSaveChanged();
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

	void SaveToSimParameters(const GameSave &saveData);

	std::optional<int> queuedVote;
	bool threadedRendering = false;

public:
	GameModel();
	~GameModel();

	void Tick();

	Tool *GetToolByIndex(int index);

	void SetEdgeMode(int edgeMode);
	int GetEdgeMode();
	void SetTemperatureScale(int temperatureScale);
	inline int GetTemperatureScale() const
	{
		return temperatureScale;
	}
	void SetThreadedRendering(bool newThreadedRendering);
	bool GetThreadedRendering() const
	{
		return threadedRendering;
	}
	void SetAmbientAirTemperature(float ambientAirTemp);
	float GetAmbientAirTemperature();
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
	void BuildBrushList();
	void BuildQuickOptionMenu(GameController * controller);

	const Snapshot *HistoryCurrent() const;
	bool HistoryCanRestore() const;
	void HistoryRestore();
	bool HistoryCanForward() const;
	void HistoryForward();
	void HistoryPush(std::unique_ptr<Snapshot> last);
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
	std::optional<int> GetToolIndex(Tool *tool);
	std::vector<Tool *> GetActiveMenuToolList();
	void AllocTool(std::unique_ptr<Tool> tool);
	void AllocElementTool(int element);
	void UpdateElementTool(int element);
	void AllocCustomGolTool(const CustomGOLData &gd);
	void FreeTool(Tool *tool);

	const std::vector<std::unique_ptr<Tool>> &GetTools()
	{
		return tools;
	}

	Brush &GetBrush();
	Brush *GetBrushByID(int i);
	int GetBrushID();
	int GetBrushIndex(const Brush &brush);
	int BrushListSize() const
	{
		return int(brushList.size());
	}
	void SetBrushID(int i);

	void SetVote(int direction);
	SaveInfo *GetSave(); // non-owning
	std::unique_ptr<SaveInfo> TakeSave();
	const SaveFile *GetSaveFile() const;
	std::unique_ptr<SaveFile> TakeSaveFile();
	void SetSave(std::unique_ptr<SaveInfo> newSave, bool invertIncludePressure);
	void SetSaveFile(std::unique_ptr<SaveFile> newSave, bool invertIncludePressure);
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
	RendererSettings &GetRendererSettings()
	{
		return rendererSettings;
	}
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
	void SetClipboard(std::unique_ptr<GameSave> save);
	void SetPlaceSave(std::unique_ptr<GameSave> save);
	void TransformPlaceSave(Mat2<int> transform, Vec2<int> nudge);
	void Log(String message, bool printToFile);
	std::deque<String> GetLog();
	const GameSave *GetClipboard() const;
	const GameSave *GetPlaceSave() const;
	const GameSave *GetTransformedPlaceSave() const;
	bool GetMouseClickRequired();
	void SetMouseClickRequired(bool mouseClickRequired);
	bool GetIncludePressure();
	void SetIncludePressure(bool includePressure);
	void SetPerfectCircle(bool perfectCircle);
	inline bool GetPerfectCircle() const
	{
		return perfectCircle;
	}

	std::vector<Notification*> GetNotifications();
	void AddNotification(Notification * notification);
	void RemoveNotification(Notification * notification);

	bool AddCustomGol(String ruleString, String nameString, RGB<uint8_t> color1, RGB<uint8_t> color2);
	bool RemoveCustomGol(const ByteString &identifier);
	void LoadCustomGol();
	void SaveCustomGol();
	std::optional<CustomGOLData> CheckCustomGol(String ruleString, String nameString, RGB<uint8_t> color1, RGB<uint8_t> color2);

	ByteString SelectNextIdentifier;
	int SelectNextTool;

	void UpdateUpTo(int upTo);
	void BeforeSim();
	void AfterSim();

	GameView *view = nullptr;
};
