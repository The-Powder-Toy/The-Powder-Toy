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
	int currentX = 1;
	//Set up UI
	class SearchAction : public ui::ButtonAction
	{
		GameView * v;
	public:
		SearchAction(GameView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->OpenSearch();
		}
	};
	searchButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16), "\x81");  //Open
	currentX+=18;
	searchButton->SetTogglable(false);
	searchButton->SetActionCallback(new SearchAction(this));
	AddComponent(searchButton);

    class ReloadAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        ReloadAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->OpenSearch(); // TODO call proper function
        }
    };
    reloadButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16), "\x91"); // TODO Position?
    currentX+=18;
    reloadButton->SetActionCallback(new ReloadAction(this));
    AddComponent(reloadButton);

    class SaveSimulationAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        SaveSimulationAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->OpenSearch(); // TODO call proper function
        }
    };
    saveSimulationButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(Size.X/5, 16), "\x82"); // TODO All arguments
    currentX+=(Size.X/5)+2;
    saveSimulationButton->SetActionCallback(new SaveSimulationAction(this));
    AddComponent(saveSimulationButton);

    class UpVoteAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        UpVoteAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->OpenSearch(); // TODO call proper function
        }
    };
    upVoteButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16), "\xCB"); // TODO All arguments
    currentX+=16;
    upVoteButton->SetActionCallback(new UpVoteAction(this));
    AddComponent(upVoteButton);

    class DownVoteAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        DownVoteAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->OpenSearch(); // TODO call proper function
        }
    };
    downVoteButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16), "\xCA"); // TODO All arguments
    currentX+=18;
    downVoteButton->SetActionCallback(new DownVoteAction(this));
    AddComponent(downVoteButton);

    class TagSimulationAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        TagSimulationAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->OpenSearch(); // TODO call proper function
        }
    };
    tagSimulationButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(Size.X-(currentX+176), 16), "\x83"); // TODO All arguments
    currentX+=Size.X-(currentX+176);
    tagSimulationButton->SetActionCallback(new TagSimulationAction(this));
    AddComponent(tagSimulationButton);

    class ClearSimAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        ClearSimAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->SetPaused(sender->GetToggleState()); // TODO call proper function
        }
    };
    clearSimButton = new ui::Button(ui::Point(Size.X-174, Size.Y-18), ui::Point(16, 16), "C");  // TODO All arguments
    clearSimButton->SetActionCallback(new ClearSimAction(this));
    AddComponent(clearSimButton);

    class LoginAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        LoginAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->SetPaused(sender->GetToggleState()); // TODO call proper function
        }
    };
    loginButton = new ui::Button(ui::Point(Size.X-156, Size.Y-18), ui::Point(100, 16), "\xDA Login");  // TODO All arguments
    loginButton->SetActionCallback(new LoginAction(this));
    AddComponent(loginButton);

    class SimulationOptionAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        SimulationOptionAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->SetPaused(sender->GetToggleState()); // TODO call proper function
        }
    };
    simulationOptionButton = new ui::Button(ui::Point(Size.X-54, Size.Y-18), ui::Point(16, 16), "\xDA");  // TODO All arguments
    simulationOptionButton->SetActionCallback(new SimulationOptionAction(this));
    AddComponent(simulationOptionButton);

    class DisplayModeAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        DisplayModeAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->SetPaused(sender->GetToggleState()); // TODO call proper function
        }
    };
    displayModeButton = new ui::Button(ui::Point(Size.X-36, Size.Y-18), ui::Point(16, 16), "\xDA");  // TODO All arguments
    displayModeButton->SetActionCallback(new DisplayModeAction(this));
    AddComponent(displayModeButton);

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
