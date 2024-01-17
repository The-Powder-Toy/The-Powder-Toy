#pragma once
#include "client/ClientListener.h"
#include "client/StartupInfo.h"
#include "gui/interface/Point.h"
#include "gui/interface/Colour.h"
#include "gui/SavePreviewType.h"
#include "simulation/Sign.h"
#include "simulation/Particle.h"
#include "Misc.h"
#include <vector>
#include <utility>
#include <memory>

constexpr auto DEBUG_PARTS      = 0x0001;
constexpr auto DEBUG_ELEMENTPOP = 0x0002;
constexpr auto DEBUG_LINES      = 0x0004;
constexpr auto DEBUG_PARTICLE   = 0x0008;
constexpr auto DEBUG_SURFNORM   = 0x0010;

class DebugInfo;
class SaveFile;
class Notification;
class GameModel;
class GameView;
class Snapshot;
class OptionsController;
class LocalBrowserController;
class SearchController;
class PreviewController;
class RenderController;
class CommandInterface;
class VideoBuffer;
class Tool;
class Menu;
class SaveInfo;
class GameSave;
class LoginController;
class TagsController;
class ConsoleController;
class GameController: public ClientListener
{
private:
	bool firstTick;
	int foundSignID;

	PreviewController * activePreview;
	GameView * gameView;
	GameModel * gameModel;
	SearchController * search;
	RenderController * renderOptions;
	LoginController * loginWindow;
	ConsoleController * console;
	TagsController * tagsWindow;
	LocalBrowserController * localBrowser;
	OptionsController * options;
	std::vector<std::unique_ptr<DebugInfo>> debugInfo;
	std::unique_ptr<Snapshot> beforeRestore;
	unsigned int debugFlags;
	
	void OpenSaveDone();
public:
	enum MouseupReason
	{
		mouseUpNormal,
		mouseUpBlur,
		mouseUpDrawEnd,
	};

	bool HasDone;
	GameController();
	~GameController();
	GameView * GetView();
	int GetSignAt(int x, int y);
	String GetSignText(int signID);
	std::pair<int, sign::Type> GetSignSplit(int signID);

	bool MouseMove(int x, int y, int dx, int dy);
	bool MouseDown(int x, int y, unsigned button);
	bool MouseUp(int x, int y, unsigned button, MouseupReason reason);
	bool MouseWheel(int x, int y, int d);
	bool TextInput(String text);
	bool TextEditing(String text);
	bool KeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	bool KeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	void Tick();
	void Blur();
	void Exit();

	void Install();

	bool HistoryRestore();
	void HistorySnapshot();
	bool HistoryForward();

	void AdjustGridSize(int direction);
	void InvertAirSim();
	void LoadRenderPreset(int presetNum);
	void SetZoomEnabled(bool zoomEnable);
	void SetZoomPosition(ui::Point position);
	void AdjustBrushSize(int direction, bool logarithmic = false, bool xAxis = false, bool yAxis = false);
	void SetBrushSize(ui::Point newSize);
	void AdjustZoomSize(int direction, bool logarithmic = false);
	void ToolClick(int toolSelection, ui::Point point);
	void DrawPoints(int toolSelection, ui::Point oldPos, ui::Point newPos, bool held);
	void DrawRect(int toolSelection, ui::Point point1, ui::Point point2);
	void DrawLine(int toolSelection, ui::Point point1, ui::Point point2);
	void DrawFill(int toolSelection, ui::Point point);
	ByteString StampRegion(ui::Point point1, ui::Point point2);
	ByteString StampRegion(ui::Point point1, ui::Point point2, bool includePressure);
	void CopyRegion(ui::Point point1, ui::Point point2);
	void CutRegion(ui::Point point1, ui::Point point2);
	void Update();
	void SetPaused(bool pauseState);
	void SetPaused();
	void SetDecoration(bool decorationState);
	void SetDecoration();
	void ShowGravityGrid();
	void SetHudEnable(bool hudState);
	bool GetHudEnable();
	void SetBrushEnable(bool brushState);
	bool GetBrushEnable();
	void SetDebugHUD(bool hudState);
	bool GetDebugHUD();
	void SetTemperatureScale(int temperatureScale);
	int GetTemperatureScale();
	int GetEdgeMode();
	void SetEdgeMode(int edgeMode);
	void SetDebugFlags(unsigned int flags) { debugFlags = flags; }
	unsigned int GetDebugFlags() const { return debugFlags; }
	void SetActiveMenu(int menuID);
	std::vector<Menu*> GetMenuList();
	int GetNumMenus(bool onlyEnabled);
	void RebuildFavoritesMenu();
	Tool * GetActiveTool(int selection);
	void SetActiveTool(int toolSelection, Tool * tool);
	void SetActiveTool(int toolSelection, ByteString identifier);
	void SetLastTool(Tool * tool);
	Tool *GetLastTool();
	int GetReplaceModeFlags();
	void SetReplaceModeFlags(int flags);
	void SetActiveColourPreset(int preset);
	void SetColour(ui::Colour colour);
	void SetToolStrength(float value);
	void LoadSaveFile(std::unique_ptr<SaveFile> file);
	void LoadSave(std::unique_ptr<SaveInfo> save);
	void OpenSearch(String searchText);
	void OpenLogin();
	void OpenProfile();
	void OpenTags();
	void OpenSavePreview(int saveID, int saveDate, SavePreviewType savePreiviewType);
	void OpenSavePreview();
	void OpenLocalSaveWindow(bool asCurrent);
	void OpenLocalBrowse();
	void OpenOptions();
	void OpenRenderOptions();
	void OpenSaveWindow();
	void SaveAsCurrent();
	void OpenStamps();
	void OpenElementSearch();
	void OpenColourPicker();
	void PlaceSave(ui::Point position);
	void ClearSim();
	void ReloadSim();
	void Vote(int direction);
	void ChangeBrush();
	void ShowConsole();
	void HideConsole();
	void FrameStep();
	void TransformPlaceSave(Mat2<int> transform, Vec2<int> nudge);
	bool MouseInZoom(ui::Point position);
	ui::Point PointTranslate(ui::Point point);
	ui::Point PointTranslateNoClamp(ui::Point point);
	ui::Point NormaliseBlockCoord(ui::Point point);
	String ElementResolve(int type, int ctype);
	String BasicParticleInfo(Particle const &sample_part);
	bool IsValidElement(int type);
	String WallName(int type);
	ByteString TakeScreenshot(int captureUI, int fileType);
	int Record(bool record);

	void ResetAir();
	void ResetSpark();
	void SwitchGravity();
	void SwitchAir();
	void ToggleAHeat();
	bool GetAHeatEnable();
	void ResetAHeat();
	void ToggleNewtonianGravity();

	bool LoadClipboard();
	void LoadStamp(std::unique_ptr<GameSave> stamp);

	void RemoveNotification(Notification * notification);

	void NotifyUpdateAvailable(Client * sender) override;
	void NotifyAuthUserChanged(Client * sender) override;
	void NotifyNewNotification(Client * sender, ServerNotification notification) override;
	void RunUpdater(UpdateInfo info);
	bool GetMouseClickRequired();

	void RemoveCustomGOLType(const ByteString &identifier);

	void BeforeSimDraw();
	void AfterSimDraw();
};
