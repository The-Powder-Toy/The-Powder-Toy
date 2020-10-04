#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <vector>
#include <utility>

#include "client/ClientListener.h"

#include "gui/interface/Point.h"
#include "gui/interface/Colour.h"

#include "simulation/Sign.h"
#include "simulation/Particle.h"

#include "Misc.h"

class DebugInfo;
class SaveFile;
class Notification;
class GameModel;
class GameView;
class OptionsController;
class LocalBrowserController;
class SearchController;
class PreviewController;
class RenderController;
class CommandInterface;
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
	CommandInterface * commandInterface;
	std::vector<DebugInfo*> debugInfo;
	unsigned int debugFlags;
	
	void OpenSaveDone();
public:
	bool HasDone;
	GameController();
	~GameController();
	GameView * GetView();
	int GetSignAt(int x, int y);
	String GetSignText(int signID);
	std::pair<int, sign::Type> GetSignSplit(int signID);

	bool MouseMove(int x, int y, int dx, int dy);
	bool MouseDown(int x, int y, unsigned button);
	bool MouseUp(int x, int y, unsigned button, char type);
	bool MouseWheel(int x, int y, int d);
	bool TextInput(String text);
	bool KeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	bool KeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	void Tick();
	void Blur();
	void Exit();

	void Install();

	void HistoryRestore();
	void HistorySnapshot();
	void HistoryForward();

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
	void SetDebugHUD(bool hudState);
	bool GetDebugHUD();
	void SetDebugFlags(unsigned int flags) { debugFlags = flags; }
	void SetActiveMenu(int menuID);
	std::vector<Menu*> GetMenuList();
	int GetNumMenus(bool onlyEnabled);
	void RebuildFavoritesMenu();
	Tool * GetActiveTool(int selection);
	void SetActiveTool(int toolSelection, Tool * tool);
	void SetActiveTool(int toolSelection, ByteString identifier);
	void SetLastTool(Tool * tool);
	int GetReplaceModeFlags();
	void SetReplaceModeFlags(int flags);
	void SetActiveColourPreset(int preset);
	void SetColour(ui::Colour colour);
	void SetToolStrength(float value);
	void LoadSaveFile(SaveFile * file);
	void LoadSave(SaveInfo * save);
	void OpenSearch(String searchText);
	void OpenLogin();
	void OpenProfile();
	void OpenTags();
	void OpenSavePreview(int saveID, int saveDate, bool instant);
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
	void TranslateSave(ui::Point point);
	void TransformSave(matrix2d transform);
	bool MouseInZoom(ui::Point position);
	ui::Point PointTranslate(ui::Point point);
	ui::Point NormaliseBlockCoord(ui::Point point);
	String ElementResolve(int type, int ctype);
	String BasicParticleInfo(Particle const &sample_part);
	bool IsValidElement(int type);
	String WallName(int type);
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
	void LoadStamp(GameSave *stamp);

	void RemoveNotification(Notification * notification);

	void NotifyUpdateAvailable(Client * sender) override;
	void NotifyAuthUserChanged(Client * sender) override;
	void NotifyNewNotification(Client * sender, std::pair<String, ByteString> notification) override;
	void RunUpdater();
	bool GetMouseClickRequired();

	void RemoveCustomGOLType(const ByteString &identifier);
};

#endif // GAMECONTROLLER_H
