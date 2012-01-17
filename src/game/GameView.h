#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <queue>
#include "GameController.h"
#include "GameModel.h"
#include "interface/Window.h"
#include "interface/Point.h"
#include "interface/Button.h"

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
	//UI Elements
	ui::Button * pauseButton;
public:
    GameView();
	void AttachController(GameController * _c){ c = _c; }
	void NotifyRendererChanged(GameModel * sender);
	void NotifySimulationChanged(GameModel * sender);
	void NotifyPausedChanged(GameModel * sender);
	/*virtual void DoMouseMove(int x, int y, int dx, int dy);
	virtual void DoMouseDown(int x, int y, unsigned button);
	virtual void DoMouseUp(int x, int y, unsigned button);
	//virtual void DoMouseWheel(int x, int y, int d);
	//virtual void DoKeyPress(int key, bool shift, bool ctrl, bool alt);
	//virtual void DoKeyRelease(int key, bool shift, bool ctrl, bool alt);
	virtual void DoTick(float dt);
	virtual void DoDraw();*/
	virtual void OnMouseMove(int x, int y, int dx, int dy);
	virtual void OnMouseDown(int x, int y, unsigned button);
	virtual void OnMouseUp(int x, int y, unsigned button);
	//virtual void OnMouseWheel(int x, int y, int d) {}
	//virtual void OnKeyPress(int key, bool shift, bool ctrl, bool alt) {}
	//virtual void OnKeyRelease(int key, bool shift, bool ctrl, bool alt) {}
	virtual void OnTick(float dt);
	virtual void OnDraw();
};

#endif // GAMEVIEW_H
