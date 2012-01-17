#include "Config.h"
#include "GameView.h"
#include "interface/Window.h"
#include "interface/Button.h"

GameView::GameView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES+BARSIZE, YRES+MENUSIZE)),
	pointQueue(queue<ui::Point*>()),
	isMouseDown(false),
	ren(NULL)
{
	//Set up UI
	class PauseAction : public ui::ButtonAction
	{
		GameView * v;
	public:
		PauseAction(GameView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->SetPaused(sender->GetToggleState());
		}
	};
	pauseButton = new ui::Button(ui::Point(Size.X-18, Size.Y-18), ui::Point(16, 16), "\x90");  //Pause
	pauseButton->SetTogglable(true);
	pauseButton->SetActionCallback(new PauseAction(this));
	AddComponent(pauseButton);
}

void GameView::NotifyRendererChanged(GameModel * sender)
{
	ren = sender->GetRenderer();
}

void GameView::NotifySimulationChanged(GameModel * sender)
{

}

void GameView::NotifyPausedChanged(GameModel * sender)
{
	pauseButton->SetToggleState(sender->GetPaused());
}

void GameView::OnMouseMove(int x, int y, int dx, int dy)
{
	if(isMouseDown)
	{
		pointQueue.push(new ui::Point(x-dx, y-dy));
		pointQueue.push(new ui::Point(x, y));
	}
}

void GameView::OnMouseDown(int x, int y, unsigned button)
{
	isMouseDown = true;
	pointQueue.push(new ui::Point(x, y));
}

void GameView::OnMouseUp(int x, int y, unsigned button)
{
	if(isMouseDown)
	{
		isMouseDown = false;
		pointQueue.push(new ui::Point(x, y));
	}
}

void GameView::OnTick(float dt)
{
	if(!pointQueue.empty())
	{
		c->DrawPoints(pointQueue);
	}
	c->Tick();
}

void GameView::OnDraw()
{
	if(ren)
	{
		ren->render_parts();
	}
}
