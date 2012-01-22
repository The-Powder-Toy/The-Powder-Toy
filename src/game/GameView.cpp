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
	searchButton = new ui::Button(ui::Point(1, Size.Y-18), ui::Point(16, 16), "\x81");  //Open
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
    reloadButton = new ui::Button(ui::Point(16, Size.Y-18), ui::Point(16, 16), "\x91"); // TODO Position?
    reloadButton->SetTogglable(false);
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
    saveSimulationButton = new ui::Button(ui::Point(32, Size.Y-18), ui::Point(152, 16), "\x82"); // TODO All arguments
    saveSimulationButton->SetTogglable(false);
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
    upVoteButton = new ui::Button(ui::Point(184, Size.Y-18), ui::Point(16, 16), "\xCB"); // TODO All arguments
    upVoteButton->SetTogglable(false);
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
    downVoteButton = new ui::Button(ui::Point(200, Size.Y-18), ui::Point(16, 16), "\xCA"); // TODO All arguments
    downVoteButton->SetTogglable(false);
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
    tagSimulationButton = new ui::Button(ui::Point(216, Size.Y-18), ui::Point(152, 16), "\x83"); // TODO All arguments
    tagSimulationButton->SetTogglable(false);
    tagSimulationButton->SetActionCallback(new TagSimulationAction(this));
    AddComponent(tagSimulationButton);

    //simul option
    //erase all
    // login

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
    displayModeButton = new ui::Button(ui::Point(Size.X-34, Size.Y-18), ui::Point(16, 16), "\xDA");  // TODO All arguments
    displayModeButton->SetTogglable(true);
    displayModeButton->SetActionCallback(new DisplayModeAction(this));
    AddComponent(displayModeButton);
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
