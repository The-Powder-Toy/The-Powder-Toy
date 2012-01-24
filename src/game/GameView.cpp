#include "Config.h"
#include "GameView.h"
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Colour.h"

GameView::GameView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES+BARSIZE, YRES+MENUSIZE)),
	pointQueue(queue<ui::Point*>()),
	isMouseDown(false),
	ren(NULL),
	activeBrush(NULL),
	currentMouse(0, 0)
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

class GameView::MenuAction: public ui::ButtonAction
{
	GameView * v;
public:
	Menu * menu;
	MenuAction(GameView * _v, Menu * menu_) { v = _v; menu = menu_; }
	void ActionCallback(ui::Button * sender)
	{
		v->c->SetActiveMenu(menu);
	}
};

class GameView::ToolAction: public ui::ButtonAction
{
	GameView * v;
public:
	Tool * tool;
	ToolAction(GameView * _v, Tool * tool_) { v = _v; tool = tool_; }
	void ActionCallback(ui::Button * sender)
	{
		v->c->SetActiveTool(tool);
	}
};

void GameView::NotifyMenuListChanged(GameModel * sender)
{
	int currentY = YRES+MENUSIZE-36;
	for(int i = 0; i < menuButtons.size(); i++)
	{
		RemoveComponent(menuButtons[i]);
		delete menuButtons[i];
	}
	menuButtons.clear();
	for(int i = 0; i < toolButtons.size(); i++)
	{
		RemoveComponent(toolButtons[i]);
		delete toolButtons[i];
	}
	toolButtons.clear();
	vector<Menu*> menuList = sender->GetMenuList();
	for(int i = 0; i < menuList.size(); i++)
	{
		std::string tempString = "";
		tempString += menuList[i]->GetIcon();
		ui::Button * tempButton = new ui::Button(ui::Point(XRES+BARSIZE-18, currentY), ui::Point(16, 16), tempString);
		tempButton->SetTogglable(true);
		tempButton->SetActionCallback(new MenuAction(this, menuList[i]));
		currentY-=18;
		AddComponent(tempButton);
		menuButtons.push_back(tempButton);
	}
}

void GameView::NotifyActiveToolChanged(GameModel * sender)
{
	for(int i = 0; i < toolButtons.size(); i++)
	{
		if(((ToolAction*)toolButtons[i]->GetActionCallback())->tool==sender->GetActiveTool())
		{
			toolButtons[i]->SetToggleState(true);
		}
		else
		{
			toolButtons[i]->SetToggleState(false);
		}
	}
}

void GameView::NotifyToolListChanged(GameModel * sender)
{
	int currentX = XRES+BARSIZE-56;
	int totalColour;
	for(int i = 0; i < menuButtons.size(); i++)
	{
		if(((MenuAction*)menuButtons[i]->GetActionCallback())->menu==sender->GetActiveMenu())
		{
			menuButtons[i]->SetToggleState(true);
		}
		else
		{
			menuButtons[i]->SetToggleState(false);
		}
	}
	for(int i = 0; i < toolButtons.size(); i++)
	{
		RemoveComponent(toolButtons[i]);
		delete toolButtons[i];
	}
	toolButtons.clear();
	vector<Tool*> toolList = sender->GetToolList();
	for(int i = 0; i < toolList.size(); i++)
	{
		ui::Button * tempButton = new ui::Button(ui::Point(currentX, YRES), ui::Point(32, 16), toolList[i]->GetName());
		currentX -= 36;
		tempButton->SetTogglable(true);
		tempButton->SetActionCallback(new ToolAction(this, toolList[i]));

		totalColour = toolList[i]->colRed + 3*toolList[i]->colGreen + 2*toolList[i]->colBlue;

		tempButton->SetBackgroundColour(ui::Colour(toolList[i]->colRed, toolList[i]->colGreen, toolList[i]->colBlue));
		if (totalColour<544)
		{
			tempButton->SetTextColour(ui::Colour(255, 255, 255));
		}
		else
		{
			tempButton->SetTextColour(ui::Colour(0, 0, 0));
		}
		tempButton->SetBorderColour(ui::Colour(0, 0, 0));
		tempButton->SetActiveBackgroundColour(ui::Colour(toolList[i]->colRed, toolList[i]->colGreen, toolList[i]->colBlue));
		tempButton->SetActiveBorderColour(ui::Colour(0, 0, 255));

		tempButton->SetAlignment(AlignCentre, AlignBottom);
		AddComponent(tempButton);
		toolButtons.push_back(tempButton);
	}

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
	currentMouse = ui::Point(x, y);
	if(isMouseDown)
	{
		pointQueue.push(new ui::Point(x-dx, y-dy));
		pointQueue.push(new ui::Point(x, y));
	}
}

void GameView::OnMouseDown(int x, int y, unsigned button)
{
	if(currentMouse.X > 0 && currentMouse.X < XRES && currentMouse.Y > 0 && currentMouse.Y < YRES)
	{
		isMouseDown = true;
		pointQueue.push(new ui::Point(x, y));
	}
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

void GameView::OnKeyPress(int key, bool shift, bool ctrl, bool alt)
{
	switch(key)
	{
	case ' ':
		c->SetPaused();
		break;
	}
}

void GameView::OnTick(float dt)
{
	if(isMouseDown)
	{
		pointQueue.push(new ui::Point(currentMouse));
	}
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
		ren->render_fire();
		ren->render_signs();
		ren->draw_walls();
	}
	if(activeBrush && currentMouse.X > 0 && currentMouse.X < XRES && currentMouse.Y > 0 && currentMouse.Y < YRES)
	{
		activeBrush->Render(ui::Engine::Ref().g, currentMouse);
	}
}
