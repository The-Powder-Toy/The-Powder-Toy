#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <vector>
#include <queue>
#include <deque>
#include <string>
#include "GameController.h"
#include "GameModel.h"
#include "interface/Window.h"
#include "interface/Point.h"
#include "interface/Button.h"
#include "interface/Slider.h"
#include "interface/Textbox.h"
#include "ToolButton.h"
#include "RenderPreset.h"
#include "Brush.h"

using namespace std;

enum DrawMode
{
	DrawPoints, DrawLine, DrawRect, DrawFill
};

enum SelectMode
{
	SelectNone, SelectStamp, SelectCopy, PlaceSave
};

class GameController;
class GameModel;
class GameView: public ui::Window
{
private:
	DrawMode drawMode;
	bool isMouseDown;
	bool zoomEnabled;
	bool zoomCursorFixed;
	bool drawSnap;
	bool shiftBehaviour;
	bool ctrlBehaviour;
	bool altBehaviour;
	bool showHud;
	bool showDebug;
	bool wallBrush;
	int introText;
	std::string introTextMessage;
	int toolIndex;

	int infoTipPresence;
	std::string toolTip;
	ui::Point toolTipPosition;
	std::string infoTip;

	queue<ui::Point*> pointQueue;
	GameController * c;
	Renderer * ren;
	Brush * activeBrush;
	//UI Elements
	vector<ui::Button*> quickOptionButtons;
	vector<ui::Button*> menuButtons;
	vector<ToolButton*> toolButtons;
	vector<ui::Component*> notificationComponents;
	deque<string> logEntries;
	float lastLogEntry;
	ui::Button * scrollBar;
	ui::Button * searchButton;
    ui::Button * reloadButton;
    ui::Button * saveSimulationButton;
    ui::Button * downVoteButton;
    ui::Button * upVoteButton;
    ui::Button * tagSimulationButton;
    ui::Button * clearSimButton;
    ui::Button * loginButton;
    ui::Button * simulationOptionButton;
    ui::Button * displayModeButton;
	ui::Button * pauseButton;
	ui::Point currentMouse;

	ui::Slider * colourRSlider;
	ui::Slider * colourGSlider;
	ui::Slider * colourBSlider;
	ui::Slider * colourASlider;

	ui::Textbox * colourRValue;
	ui::Textbox * colourGValue;
	ui::Textbox * colourBValue;
	ui::Textbox * colourAValue;

	bool drawModeReset;
	ui::Point drawPoint1;
	ui::Point drawPoint2;

	SelectMode selectMode;
	ui::Point selectPoint1;
	ui::Point selectPoint2;

	ui::Point mousePosition;

	RenderPreset * renderModePresets;

	Thumbnail * placeSaveThumb;

	SimulationSample sample;

	int lastOffset;
	void setToolButtonOffset(int offset);
	void changeColourSlider();
	void changeColourText();
	virtual ui::Point lineSnapCoords(ui::Point point1, ui::Point point2);
	virtual ui::Point rectSnapCoords(ui::Point point1, ui::Point point2);

	void enableShiftBehaviour();
	void disableShiftBehaviour();
	void enableCtrlBehaviour();
	void disableCtrlBehaviour();
	void enableAltBehaviour();
	void disableAltBehaviour();
public:
    GameView();
    virtual ~GameView();

    //Breaks MVC, but any other way is going to be more of a mess.
    ui::Point GetMousePosition();
    void SetSample(SimulationSample sample);
    bool CtrlBehaviour(){ return ctrlBehaviour; }
    bool ShiftBehaviour(){ return shiftBehaviour; }

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
	void NotifyPlaceSaveChanged(GameModel * sender);
	void NotifyNotificationsChanged(GameModel * sender);
	void NotifyLogChanged(GameModel * sender, string entry);
	void NotifyToolTipChanged(GameModel * sender);
	void NotifyInfoTipChanged(GameModel * sender);
	void NotifyQuickOptionsChanged(GameModel * sender);
	void NotifyLastToolChanged(GameModel * sender);

	void ExitPrompt();

	virtual void ToolTip(ui::Component * sender, ui::Point mousePosition, std::string toolTip);

	virtual void OnMouseMove(int x, int y, int dx, int dy);
	virtual void OnMouseDown(int x, int y, unsigned button);
	virtual void OnMouseUp(int x, int y, unsigned button);
	virtual void OnMouseWheel(int x, int y, int d);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnTick(float dt);
	virtual void OnDraw();
	virtual void OnBlur();

	//Top-level handers, for Lua interface
	virtual void DoDraw();
	virtual void DoMouseMove(int x, int y, int dx, int dy);
	virtual void DoMouseDown(int x, int y, unsigned button);
	virtual void DoMouseUp(int x, int y, unsigned button);
	virtual void DoMouseWheel(int x, int y, int d);
	virtual void DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void DoKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);

	class MenuAction;
	class ToolAction;
	class OptionAction;
	class OptionListener;
};

#endif // GAMEVIEW_H
