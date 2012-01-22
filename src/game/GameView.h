#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <queue>
#include "GameController.h"
#include "GameModel.h"
#include "interface/Window.h"
#include "interface/Point.h"
#include "interface/Button.h"
#include "Brush.h"

using namespace std;

class GameController;
class GameModel;
class GameView: public ui::Window
{
private:
	bool isMouseDown;
	queue<ui::Point*> pointQueue;
	GameController * c;
	Renderer * ren;
	Brush * activeBrush;
	//UI Elements
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
public:
    GameView();
	void AttachController(GameController * _c){ c = _c; }
	void NotifyRendererChanged(GameModel * sender);
	void NotifySimulationChanged(GameModel * sender);
	void NotifyPausedChanged(GameModel * sender);
	void NotifySaveChanged(GameModel * sender);
	void NotifyBrushChanged(GameModel * sender);
	virtual void OnMouseMove(int x, int y, int dx, int dy);
	virtual void OnMouseDown(int x, int y, unsigned button);
	virtual void OnMouseUp(int x, int y, unsigned button);
	virtual void OnMouseWheel(int x, int y, int d);
	//virtual void OnKeyPress(int key, bool shift, bool ctrl, bool alt) {}
	//virtual void OnKeyRelease(int key, bool shift, bool ctrl, bool alt) {}
	virtual void OnTick(float dt);
	virtual void OnDraw();
};

#endif // GAMEVIEW_H
