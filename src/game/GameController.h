 #ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <queue>
#include "GameView.h"
#include "GameModel.h"
#include "interface/Point.h"
#include "simulation/Simulation.h"
#include "search/SearchController.h"
#include "render/RenderController.h"
#include "preview/PreviewController.h"
#include "login/LoginController.h"
#include "tags/TagsController.h"
#include "console/ConsoleController.h"
#include "localbrowser/LocalBrowserController.h"
//#include "cat/TPTScriptInterface.h"
#include "cat/LuaScriptInterface.h"
#include "options/OptionsController.h"
#include "client/ClientListener.h"
#include "RenderPreset.h"
#include "Menu.h"

using namespace std;

class Notification;
class GameModel;
class GameView;
class CommandInterface;
class ConsoleController;
class GameController: public ClientListener
{
private:
	//Simulation * sim;
	int screenshotIndex;
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
public:
	bool HasDone;
	class LoginCallback;
	class SearchCallback;
	class RenderCallback;
	class SSaveCallback;
	class TagsCallback;
	class StampsCallback;
	class OptionsCallback;
	class SaveOpenCallback;
	friend class SaveOpenCallback;
	GameController();
	~GameController();
	GameView * GetView();

	bool BrushChanged(int brushType, int rx, int ry);
	bool MouseMove(int x, int y, int dx, int dy);
	bool MouseDown(int x, int y, unsigned button);
	bool MouseUp(int x, int y, unsigned button);
	bool MouseWheel(int x, int y, int d);
	bool KeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	bool KeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	void Tick();
	void Exit();

	void Install();

	void HistoryRestore();
	void HistorySnapshot();

	void AdjustGridSize(int direction);
	void InvertAirSim();
	void LoadRenderPreset(RenderPreset preset);
	void SetZoomEnabled(bool zoomEnable);
	void SetZoomPosition(ui::Point position);
	void AdjustBrushSize(int direction, bool logarithmic = false, bool xAxis = false, bool yAxis = false);
	void AdjustZoomSize(int direction, bool logarithmic = false);
	void ToolClick(int toolSelection, ui::Point point);
	void DrawPoints(int toolSelection, queue<ui::Point*> & pointQueue);
	void DrawRect(int toolSelection, ui::Point point1, ui::Point point2);
	void DrawLine(int toolSelection, ui::Point point1, ui::Point point2);
	void DrawFill(int toolSelection, ui::Point point);
	void StampRegion(ui::Point point1, ui::Point point2);
	void CopyRegion(ui::Point point1, ui::Point point2);
	void CutRegion(ui::Point point1, ui::Point point2);
	void Update();
	void SetPaused(bool pauseState);
	void SetPaused();
	void SetDecoration(bool decorationState);
	void SetDecoration();
	void SetActiveMenu(Menu * menu);
	void SetActiveTool(int toolSelection, Tool * tool);
	void SetColour(ui::Colour colour);
	void SetToolStrength(float value);
	void LoadSaveFile(SaveFile * file);
	void LoadSave(SaveInfo * save);
	void OpenSearch();
	void OpenLogin();
	void OpenTags();
	void OpenSavePreview(int saveID, int saveDate);
	void OpenLocalSaveWindow();
	void OpenLocalBrowse();
	void OpenOptions();
	void OpenRenderOptions();
	void OpenSaveWindow();
	void SaveAsCurrent();
	void OpenStamps();
	void OpenElementSearch();
	void PlaceSave(ui::Point position);
	void ClearSim();
	void ReloadSim();
	void Vote(int direction);
	void ChangeBrush();
	void ShowConsole();
	void FrameStep();
	void TranslateSave(ui::Point point);
	void TransformSave(matrix2d transform);
	ui::Point PointTranslate(ui::Point point);
	ui::Point NormaliseBlockCoord(ui::Point point);
	std::string ElementResolve(int type);

	void ResetAir();
	void ResetSpark();
	void SwitchGravity();
	void SwitchAir();
	void ToggleAHeat();

	void LoadClipboard();
	void LoadStamp();

	void RemoveNotification(Notification * notification);

	virtual void NotifyUpdateAvailable(Client * sender);
	void RunUpdater();
};

#endif // GAMECONTROLLER_H
