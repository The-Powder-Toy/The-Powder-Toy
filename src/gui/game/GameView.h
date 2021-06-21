#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <vector>
#include <deque>
#include "common/String.h"
#include "gui/interface/Window.h"
#include "simulation/Sample.h"

enum DrawMode
{
	DrawPoints, DrawLine, DrawRect, DrawFill
};

enum SelectMode
{
	SelectNone, SelectStamp, SelectCopy, SelectCut, PlaceSave
};

namespace ui
{
	class Button;
	class Slider;
	class Textbox;
}

class SplitButton;

class MenuButton;
class Renderer;
class VideoBuffer;
class ToolButton;
class GameController;
class Brush;
class GameModel;
class GameView: public ui::Window
{
private:
	bool isMouseDown;
	bool skipDraw;
	bool zoomEnabled;
	bool zoomCursorFixed;
	bool mouseInZoom;
	bool drawSnap;
	bool shiftBehaviour;
	bool ctrlBehaviour;
	bool altBehaviour;
	bool showHud;
	bool showBrush;
	bool showDebug;
	int delayedActiveMenu;
	bool wallBrush;
	bool toolBrush;
	bool decoBrush;
	bool windTool;
	int toolIndex;
	int currentSaveType;
	int lastMenu;

	int toolTipPresence;
	String toolTip;
	bool isToolTipFadingIn;
	ui::Point toolTipPosition;
	int infoTipPresence;
	String infoTip;
	int buttonTipShow;
	String buttonTip;
	bool isButtonTipFadingIn;
	int introText;
	String introTextMessage;

	bool doScreenshot;
	int screenshotIndex;
	bool recording;
	int recordingFolder;

	ui::Point currentPoint, lastPoint;
	GameController * c;
	Renderer * ren;
	Brush * activeBrush;
	//UI Elements
	std::vector<ui::Button*> quickOptionButtons;

	std::vector<MenuButton*> menuButtons;

	std::vector<ToolButton*> toolButtons;
	std::vector<ui::Component*> notificationComponents;
	std::deque<std::pair<String, int> > logEntries;
	ui::Button * scrollBar;
	ui::Button * searchButton;
	ui::Button * reloadButton;
	SplitButton * saveSimulationButton;
	bool saveSimulationButtonEnabled;
	bool saveReuploadAllowed;
	ui::Button * downVoteButton;
	ui::Button * upVoteButton;
	ui::Button * tagSimulationButton;
	ui::Button * clearSimButton;
	SplitButton * loginButton;
	ui::Button * simulationOptionButton;
	ui::Button * displayModeButton;
	ui::Button * pauseButton;

	ui::Button * colourPicker;
	std::vector<ToolButton*> colourPresets;

	DrawMode drawMode;
	ui::Point drawPoint1;
	ui::Point drawPoint2;

	SelectMode selectMode;
	ui::Point selectPoint1;
	ui::Point selectPoint2;

	ui::Point currentMouse;
	ui::Point mousePosition;

	VideoBuffer * placeSaveThumb;
	ui::Point placeSaveOffset;

	SimulationSample sample;

	void updateToolButtonScroll();

	void SetSaveButtonTooltips();

	void screenshot();

	void enableShiftBehaviour();
	void disableShiftBehaviour();
	void enableCtrlBehaviour();
	void disableCtrlBehaviour();
	void enableAltBehaviour();
	void disableAltBehaviour();
	void UpdateDrawMode();
	void UpdateToolStrength();
public:
	GameView();
	virtual ~GameView();

	//Breaks MVC, but any other way is going to be more of a mess.
	ui::Point GetMousePosition();
	void SetSample(SimulationSample sample);
	void SetHudEnable(bool hudState);
	bool GetHudEnable();
	void SetBrushEnable(bool hudState);
	bool GetBrushEnable();
	void SetDebugHUD(bool mode);
	bool GetDebugHUD();
	bool GetPlacingSave();
	bool GetPlacingZoom();
	void SetActiveMenuDelayed(int activeMenu) { delayedActiveMenu = activeMenu; }
	bool CtrlBehaviour(){ return ctrlBehaviour; }
	bool ShiftBehaviour(){ return shiftBehaviour; }
	bool AltBehaviour(){ return altBehaviour; }
	SelectMode GetSelectMode() { return selectMode; }
	void BeginStampSelection();
	ui::Point GetPlaceSaveOffset() { return placeSaveOffset; }
	void SetPlaceSaveOffset(ui::Point offset) { placeSaveOffset = offset; }
	int Record(bool record);

	//all of these are only here for one debug lines
	bool GetMouseDown() { return isMouseDown; }
	bool GetDrawingLine() { return drawMode == DrawLine && isMouseDown; }
	bool GetDrawSnap() { return drawSnap; }
	ui::Point GetLineStartCoords() { return drawPoint1; }
	ui::Point GetLineFinishCoords() { return currentMouse; }
	ui::Point lineSnapCoords(ui::Point point1, ui::Point point2);
	ui::Point rectSnapCoords(ui::Point point1, ui::Point point2);

	void AttachController(GameController * _c){ c = _c; }
	void NotifyRendererChanged(GameModel * sender);
	void NotifySimulationChanged(GameModel * sender);
	void NotifyPausedChanged(GameModel * sender);
	void NotifySaveChanged(GameModel * sender);
	void NotifyBrushChanged(GameModel * sender);
	void NotifyMenuListChanged(GameModel * sender);
	void NotifyToolListChanged(GameModel * sender);
	void NotifyActiveToolsChanged(GameModel * sender);
	void NotifyUserChanged(GameModel * sender);
	void NotifyZoomChanged(GameModel * sender);
	void NotifyColourSelectorVisibilityChanged(GameModel * sender);
	void NotifyColourSelectorColourChanged(GameModel * sender);
	void NotifyColourPresetsChanged(GameModel * sender);
	void NotifyColourActivePresetChanged(GameModel * sender);
	void NotifyPlaceSaveChanged(GameModel * sender);
	void NotifyNotificationsChanged(GameModel * sender);
	void NotifyLogChanged(GameModel * sender, String entry);
	void NotifyToolTipChanged(GameModel * sender);
	void NotifyInfoTipChanged(GameModel * sender);
	void NotifyQuickOptionsChanged(GameModel * sender);
	void NotifyLastToolChanged(GameModel * sender);


	void ToolTip(ui::Point senderPosition, String toolTip) override;

	void OnMouseMove(int x, int y, int dx, int dy) override;
	void OnMouseDown(int x, int y, unsigned button) override;
	void OnMouseUp(int x, int y, unsigned button) override;
	void OnMouseWheel(int x, int y, int d) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnTick(float dt) override;
	void OnDraw() override;
	void OnBlur() override;
	void OnFileDrop(ByteString filename) override;

	//Top-level handlers, for Lua interface
	void DoExit() override;
	void DoDraw() override;
	void DoMouseMove(int x, int y, int dx, int dy) override;
	void DoMouseDown(int x, int y, unsigned button) override;
	void DoMouseUp(int x, int y, unsigned button) override;
	void DoMouseWheel(int x, int y, int d) override;
	void DoTextInput(String text) override;
	void DoTextEditing(String text) override;
	void DoKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void DoKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;

	class OptionListener;
};

#endif // GAMEVIEW_H
