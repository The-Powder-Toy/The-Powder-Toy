#include "Config.h"
#include "GameView.h"
#include "interface/Window.h"
#include "interface/Button.h"

GameView::GameView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES+BARSIZE, YRES+MENUSIZE)),
	pointQueue(queue<ui::Point*>()),
	isMouseDown(false),
	ren(NULL),
	activeBrush(NULL)
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
            v->c->ReloadSim();
        }
    };
    reloadButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16), "\x91");
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
            v->c->OpenSaveWindow();
        }
    };
    saveSimulationButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(Size.X/5, 16), "\x82");
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
        	v->c->Vote(1);
        }
    };
    upVoteButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16), "\xCB");
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
        	v->c->Vote(-1);
        }
    };
    downVoteButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16), "\xCA");
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
            v->c->OpenTags();
        }
    };
    tagSimulationButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(Size.X-(currentX+176), 16), "\x83");
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
            v->c->ClearSim();
        }
    };
    clearSimButton = new ui::Button(ui::Point(Size.X-174, Size.Y-18), ui::Point(16, 16), "C");
    clearSimButton->SetActionCallback(new ClearSimAction(this));
    AddComponent(clearSimButton);

    class LoginAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        LoginAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->OpenLogin();
        }
    };
    loginButton = new ui::Button(ui::Point(Size.X-156, Size.Y-18), ui::Point(100, 16), "\xDA Login");
    loginButton->SetActionCallback(new LoginAction(this));
    AddComponent(loginButton);

    class SimulationOptionAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        SimulationOptionAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->OpenDisplayOptions();
        }
    };
    simulationOptionButton = new ui::Button(ui::Point(Size.X-54, Size.Y-18), ui::Point(16, 16), "\xDA");
    simulationOptionButton->SetActionCallback(new SimulationOptionAction(this));
    AddComponent(simulationOptionButton);

    class DisplayModeAction : public ui::ButtonAction
    {
        GameView * v;
    public:
        DisplayModeAction(GameView * _v) { v = _v; }
        void ActionCallback(ui::Button * sender)
        {
            v->c->OpenRenderOptions();
        }
    };
    displayModeButton = new ui::Button(ui::Point(Size.X-36, Size.Y-18), ui::Point(16, 16), "\xDA");
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

void GameView::NotifySaveChanged(GameModel * sender)
{
	if(sender->GetSave())
	{
		reloadButton->Enabled = true;
		if(sender->GetSave()->GetID())	//Online saves have an ID, local saves have an ID of 0 and a filename
		{
			upVoteButton->Enabled = true;
			downVoteButton->Enabled = true;
			tagSimulationButton->Enabled = true;
		}
		else
		{
			upVoteButton->Enabled = false;
			downVoteButton->Enabled = false;
			tagSimulationButton->Enabled = false;
		}
	}
	else
	{
		reloadButton->Enabled = false;
		upVoteButton->Enabled = false;
		downVoteButton->Enabled = false;
		tagSimulationButton->Enabled = false;
	}
}

void GameView::NotifyBrushChanged(GameModel * sender)
{
	activeBrush = sender->GetBrush();
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

void GameView::OnMouseWheel(int x, int y, int d)
{
	if(!d)
		return;
	c->AdjustBrushSize(d);
	if(isMouseDown)
	{
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
	if(activeBrush)
	{
		activeBrush->Render(ui::Engine::Ref().g, ui::Point(ui::Engine::Ref().GetMouseX(),ui::Engine::Ref().GetMouseY()));
	}
}
