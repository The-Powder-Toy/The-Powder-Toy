#pragma once
#include "common/String.h"
#include "gui/interface/Window.h"
#include "gui/interface/Fade.h"
#include "simulation/Sample.h"
#include "graphics/FindingElement.h"
#include "graphics/RendererFrame.h"
#include <ctime>
#include <deque>
#include <memory>
#include <vector>
#include <optional>
#include <thread>
#include <mutex>
#include <condition_variable>

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
class Simulation;
struct RenderableSimulation;

class MenuButton;
class Renderer;
struct RendererSettings;
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
	int toolIndex;
	int currentSaveType;
	int lastMenu;

	ui::Fade toolTipPresence{ ui::Fade::LinearProfile{ 120.f, 60.f }, 0, 0 };
	String toolTip;
	bool isToolTipFadingIn;
	ui::Point toolTipPosition;
	ui::Fade infoTipPresence{ ui::Fade::LinearProfile{ 60.f, 60.f }, 0, 0 };
	String infoTip;
	ui::Fade buttonTipShow{ ui::Fade::LinearProfile{ 120.f, 60.f }, 0, 0 };
	String buttonTip;
	bool isButtonTipFadingIn;
	ui::Fade introText{ ui::Fade::LinearProfile{ 60.f, 60.f }, 0, 2048 };
	String introTextMessage;

	bool doScreenshot;
	int screenshotIndex;
	time_t lastScreenshotTime;
	int recordingIndex;
	bool recording;
	int recordingFolder;

	ui::Point currentPoint, lastPoint;
	GameController * c;
	Renderer *ren = nullptr;
	RendererSettings *rendererSettings = nullptr;
	bool wantFrame = false;
	Simulation *sim = nullptr;
	Brush const *activeBrush;
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
	void ResetVoteButtons();
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

	std::unique_ptr<VideoBuffer> placeSaveThumb;
	Mat2<int> placeSaveTransform = Mat2<int>::Identity;
	Vec2<int> placeSaveTranslate = Vec2<int>::Zero;
	void TranslateSave(Vec2<int> addToTranslate);
	void TransformSave(Mat2<int> mulToTransform);
	void ApplyTransformPlaceSave();

	SimulationSample sample;

	void updateToolButtonScroll();

	void SetSaveButtonTooltips();

	void enableShiftBehaviour();
	void disableShiftBehaviour();
	void enableCtrlBehaviour();
	void disableCtrlBehaviour();
	void enableAltBehaviour();
	void disableAltBehaviour();
	void UpdateDrawMode();
	void UpdateToolStrength();

	Vec2<int> PlaceSavePos() const;

	std::optional<FindingElement> FindingElementCandidate() const;
	enum RendererThreadState
	{
		rendererThreadAbsent,
		rendererThreadRunning,
		rendererThreadPaused,
		rendererThreadStopping,
	};
	RendererThreadState rendererThreadState = rendererThreadAbsent;
	std::thread rendererThread;
	std::mutex rendererThreadMx;
	std::condition_variable rendererThreadCv;
	bool rendererThreadOwnsRenderer = false;
	void StartRendererThread();
	void StopRendererThread();
	void RendererThread();
	void WaitForRendererThread();
	void DispatchRendererThread();
	std::unique_ptr<RenderableSimulation> rendererThreadSim;
	std::unique_ptr<RendererFrame> rendererThreadResult;
	RendererStats rendererStats;
	const RendererFrame *rendererFrame = nullptr;

	SimFpsLimit simFpsLimit = FpsLimitExplicit{ 60.f };
	void ApplySimFpsLimit();

public:
	GameView();
	~GameView();

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
	ByteString TakeScreenshot(int captureUI, int fileType);
	int Record(bool record);

	//all of these are only here for one debug lines
	bool GetMouseDown() { return isMouseDown; }
	bool GetDrawingLine() { return drawMode == DrawLine && isMouseDown && selectMode == SelectNone; }
	bool GetDrawSnap() { return drawSnap; }
	ui::Point GetLineStartCoords() { return drawPoint1; }
	ui::Point GetLineFinishCoords() { return currentMouse; }
	ui::Point GetCurrentMouse() { return currentMouse; }
	ui::Point lineSnapCoords(ui::Point point1, ui::Point point2);
	ui::Point rectSnapCoords(ui::Point point1, ui::Point point2);

	void AttachController(GameController * _c){ c = _c; }
	void NotifyRendererChanged(GameModel * sender);
	void NotifySimulationChanged(GameModel * sender);
	void NotifyPausedChanged(GameModel * sender);
	void NotifySaveChanged(GameModel * sender);
	void NotifyBrushChanged(GameModel * sender);
	void NotifyMenuListChanged(GameModel * sender);
	void NotifyActiveMenuToolListChanged(GameModel * sender);
	void NotifyActiveToolsChanged(GameModel * sender);
	void NotifyUserChanged(GameModel * sender);
	void NotifyZoomChanged(GameModel * sender);
	void NotifyColourSelectorVisibilityChanged(GameModel * sender);
	void NotifyColourSelectorColourChanged(GameModel * sender);
	void NotifyColourPresetsChanged(GameModel * sender);
	void NotifyColourActivePresetChanged(GameModel * sender);
	void NotifyPlaceSaveChanged(GameModel * sender);
	void NotifyTransformedPlaceSaveChanged(GameModel *sender);
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
	void OnTick() override;
	void OnSimTick() override;
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

	void SkipIntroText();
	pixel GetPixelUnderMouse() const;

	const RendererFrame &GetRendererFrame() const
	{
		return *rendererFrame;
	}
	// Call this before accessing Renderer "out of turn", e.g. from RenderView or GameModel. This *does not*
	// include OptionsModel or Lua setting functions because they only access the RendererSettings
	// in GameModel, or Lua drawing functions because they only access Renderer in eventTraitSimGraphics
	// and *SimDraw events, and the renderer thread gets paused anyway if there are handlers
	// installed for such events.
	void PauseRendererThread();

	void RenderSimulation(const RenderableSimulation &sim, bool handleEvents);
	void AfterSimDraw(const RenderableSimulation &sim);

	void SetSimFpsLimit(SimFpsLimit newSimFpsLimit);
	SimFpsLimit GetSimFpsLimit() const
	{
		return simFpsLimit;
	}
};
