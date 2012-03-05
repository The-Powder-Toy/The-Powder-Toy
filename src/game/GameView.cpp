#include "Config.h"
#include "GameView.h"
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Colour.h"
#include "interface/Keys.h"
#include "interface/Slider.h"

GameView::GameView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES+BARSIZE, YRES+MENUSIZE)),
	pointQueue(queue<ui::Point*>()),
	isMouseDown(false),
	ren(NULL),
	activeBrush(NULL),
	currentMouse(0, 0),
	toolIndex(0),
	zoomEnabled(false),
	zoomCursorFixed(false),
	drawPoint1(0, 0),
	drawPoint2(0, 0),
	drawMode(DrawPoints),
	drawModeReset(false)
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
	searchButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16));  //Open
	searchButton->SetIcon(IconOpen);
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
    reloadButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16));
    reloadButton->SetIcon(IconReload);
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
    saveSimulationButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(Size.X/5, 16));
    saveSimulationButton->SetIcon(IconSave);
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
    upVoteButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16));
    upVoteButton->SetIcon(IconVoteUp);
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
    downVoteButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(16, 16));
    downVoteButton->SetIcon(IconVoteDown);
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
    tagSimulationButton = new ui::Button(ui::Point(currentX, Size.Y-18), ui::Point(Size.X-(currentX+176), 16));
    tagSimulationButton->SetIcon(IconTag);
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
    clearSimButton = new ui::Button(ui::Point(Size.X-174, Size.Y-18), ui::Point(16, 16));
    clearSimButton->SetIcon(IconNew);
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
    loginButton = new ui::Button(ui::Point(Size.X-156, Size.Y-18), ui::Point(100, 16), "Login");
    loginButton->SetIcon(IconLogin);
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
    simulationOptionButton = new ui::Button(ui::Point(Size.X-54, Size.Y-18), ui::Point(16, 16));
    simulationOptionButton->SetIcon(IconSimulationSettings);
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
    displayModeButton = new ui::Button(ui::Point(Size.X-36, Size.Y-18), ui::Point(16, 16));
    displayModeButton->SetIcon(IconRenderSettings);
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
	pauseButton = new ui::Button(ui::Point(Size.X-18, Size.Y-18), ui::Point(16, 16));  //Pause
	pauseButton->SetIcon(IconPause);
	pauseButton->SetTogglable(true);
	pauseButton->SetActionCallback(new PauseAction(this));
	AddComponent(pauseButton);

	class ColourChange : public ui::SliderAction
	{
		GameView * v;
	public:
		ColourChange(GameView * _v) { v = _v; }
		void ValueChangedCallback(ui::Slider * sender)
		{
			v->changeColour();
		}
	};
	ColourChange * colC = new ColourChange(this);
	colourRSlider = new ui::Slider(ui::Point(5, Size.Y-39), ui::Point(80, 14), 255);
	colourRSlider->SetActionCallback(colC);
	colourGSlider = new ui::Slider(ui::Point(95, Size.Y-39), ui::Point(80, 14), 255);
	colourGSlider->SetActionCallback(colC);
	colourBSlider = new ui::Slider(ui::Point(185, Size.Y-39), ui::Point(80, 14), 255);
	colourBSlider->SetActionCallback(colC);
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
	void ActionCallback(ui::Button * sender_)
	{
		ToolButton *sender = (ToolButton*)sender_;
		if(sender->GetSelectionState() >= 0 && sender->GetSelectionState() <= 2)
			v->c->SetActiveTool(sender->GetSelectionState(), tool);
	}
};

void GameView::NotifyMenuListChanged(GameModel * sender)
{
	int currentY = YRES+MENUSIZE-18-(sender->GetMenuList().size()*18);
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
		currentY+=18;
		AddComponent(tempButton);
		menuButtons.push_back(tempButton);
	}
}

void GameView::NotifyActiveToolsChanged(GameModel * sender)
{
	for(int i = 0; i < toolButtons.size(); i++)
	{
		Tool * tool = ((ToolAction*)toolButtons[i]->GetActionCallback())->tool;
		if(sender->GetActiveTool(0) == tool)
		{
			toolButtons[i]->SetSelectionState(0);	//Primary
		}
		else if(sender->GetActiveTool(1) == tool)
		{
			toolButtons[i]->SetSelectionState(1);	//Secondary
		}
		else if(sender->GetActiveTool(2) == tool)
		{
			toolButtons[i]->SetSelectionState(2);	//Tertiary
		}
		else
		{
			toolButtons[i]->SetSelectionState(-1);
		}
	}
}

void GameView::NotifyToolListChanged(GameModel * sender)
{
	//int currentY = YRES+MENUSIZE-36;
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
		//ToolButton * tempButton = new ToolButton(ui::Point(XRES+1, currentY), ui::Point(28, 15), toolList[i]->GetName());
		ToolButton * tempButton = new ToolButton(ui::Point(currentX, YRES+1), ui::Point(28, 15), toolList[i]->GetName());
		//currentY -= 17;
		currentX -= 32;
		tempButton->SetActionCallback(new ToolAction(this, toolList[i]));

		tempButton->SetBackgroundColour(ui::Colour(toolList[i]->colRed, toolList[i]->colGreen, toolList[i]->colBlue));

		if(sender->GetActiveTool(0) == toolList[i])
		{
			tempButton->SetSelectionState(0);	//Primary
		}
		else if(sender->GetActiveTool(1) == toolList[i])
		{
			tempButton->SetSelectionState(1);	//Secondary
		}
		else if(sender->GetActiveTool(2) == toolList[i])
		{
			tempButton->SetSelectionState(2);	//Tertiary
		}

		tempButton->SetAlignment(AlignCentre, AlignBottom);
		AddComponent(tempButton);
		toolButtons.push_back(tempButton);
	}

}

void GameView::NotifyColourSelectorVisibilityChanged(GameModel * sender)
{
	RemoveComponent(colourRSlider);
	colourRSlider->SetParentWindow(NULL);
	RemoveComponent(colourGSlider);
	colourGSlider->SetParentWindow(NULL);
	RemoveComponent(colourBSlider);
	colourBSlider->SetParentWindow(NULL);
	if(sender->GetColourSelectorVisibility())
	{
		AddComponent(colourRSlider);
		AddComponent(colourGSlider);
		AddComponent(colourBSlider);
	}

}

void GameView::NotifyColourSelectorColourChanged(GameModel * sender)
{
	colourRSlider->SetValue(sender->GetColourSelectorColour().Red);
	colourRSlider->SetColour(ui::Colour(0, sender->GetColourSelectorColour().Green, sender->GetColourSelectorColour().Blue), ui::Colour(255, sender->GetColourSelectorColour().Green, sender->GetColourSelectorColour().Blue));
	colourGSlider->SetValue(sender->GetColourSelectorColour().Green);
	colourGSlider->SetColour(ui::Colour(sender->GetColourSelectorColour().Red, 0, sender->GetColourSelectorColour().Blue), ui::Colour(sender->GetColourSelectorColour().Red, 255, sender->GetColourSelectorColour().Blue));
	colourBSlider->SetValue(sender->GetColourSelectorColour().Blue);
	colourBSlider->SetColour(ui::Colour(sender->GetColourSelectorColour().Red, sender->GetColourSelectorColour().Green, 0), ui::Colour(sender->GetColourSelectorColour().Red, sender->GetColourSelectorColour().Green, 255));

	vector<Tool*> tools = sender->GetMenuList()[SC_DECO]->GetToolList();
	for(int i = 0; i < tools.size(); i++)
	{
		tools[i]->colRed = sender->GetColourSelectorColour().Red;
		tools[i]->colGreen = sender->GetColourSelectorColour().Green;
		tools[i]->colBlue = sender->GetColourSelectorColour().Blue;
	}
	NotifyToolListChanged(sender);
}

void GameView::NotifyRendererChanged(GameModel * sender)
{
	ren = sender->GetRenderer();
}

void GameView::NotifySimulationChanged(GameModel * sender)
{

}
void GameView::NotifyUserChanged(GameModel * sender)
{
	if(!sender->GetUser().ID)
	{
		loginButton->SetText("Login");
	}
	else
	{
		loginButton->SetText(sender->GetUser().Username);
	}
	NotifySaveChanged(sender);
}


void GameView::NotifyPausedChanged(GameModel * sender)
{
	pauseButton->SetToggleState(sender->GetPaused());
}

void GameView::NotifySaveChanged(GameModel * sender)
{
	if(sender->GetSave())
	{
		saveSimulationButton->SetText(sender->GetSave()->GetName());
		reloadButton->Enabled = true;
		upVoteButton->Enabled = (sender->GetSave()->GetID() && sender->GetUser().ID && sender->GetSave()->GetVote()==0);
		if(sender->GetSave()->GetID() && sender->GetUser().ID && sender->GetSave()->GetVote()==1)
			upVoteButton->SetBackgroundColour(ui::Colour(0, 200, 40));
		else
			upVoteButton->SetBackgroundColour(ui::Colour(0, 0, 0));
		downVoteButton->Enabled = upVoteButton->Enabled;
		if(sender->GetSave()->GetID() && sender->GetUser().ID && sender->GetSave()->GetVote()==-1)
			downVoteButton->SetBackgroundColour(ui::Colour(200, 40, 40));
		else
			downVoteButton->SetBackgroundColour(ui::Colour(0, 0, 0));
		tagSimulationButton->Enabled = (sender->GetSave()->GetID() && sender->GetUser().ID);
	}
	else
	{
		saveSimulationButton->SetText("");
		reloadButton->Enabled = false;
		upVoteButton->Enabled = false;
		upVoteButton->SetBackgroundColour(ui::Colour(0, 0, 0));
		downVoteButton->Enabled = false;
		upVoteButton->SetBackgroundColour(ui::Colour(0, 0, 0));
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
	if(isMouseDown && drawMode == DrawPoints)
	{
		pointQueue.push(new ui::Point(x-dx, y-dy));
		pointQueue.push(new ui::Point(x, y));
	}
}

void GameView::OnMouseDown(int x, int y, unsigned button)
{
	if(currentMouse.X > 0 && currentMouse.X < XRES && currentMouse.Y > 0 && currentMouse.Y < YRES && !(zoomEnabled && !zoomCursorFixed))
	{
		if(button == BUTTON_LEFT)
			toolIndex = 0;
		if(button == BUTTON_RIGHT)
			toolIndex = 1;
		if(button == BUTTON_MIDDLE)
			toolIndex = 2;
		isMouseDown = true;
		if(drawMode == DrawRect || drawMode == DrawLine)
		{
			drawPoint1 = ui::Point(x, y);
		}
		if(drawMode == DrawPoints)
		{
			pointQueue.push(new ui::Point(x, y));
		}
	}
}

void GameView::OnMouseUp(int x, int y, unsigned button)
{
	if(zoomEnabled && !zoomCursorFixed)
		zoomCursorFixed = true;
	else
	{
		if(isMouseDown)
		{
			isMouseDown = false;
			if(drawMode == DrawRect || drawMode == DrawLine)
			{
				drawPoint2 = ui::Point(x, y);
				if(drawMode == DrawRect)
				{
					c->DrawRect(toolIndex, drawPoint1, drawPoint2);
				}
				if(drawMode == DrawLine)
				{
					c->DrawLine(toolIndex, drawPoint1, drawPoint2);
				}
			}
			if(drawMode == DrawPoints)
			{
				pointQueue.push(new ui::Point(x, y));
			}
			if(drawModeReset)
			{
				drawModeReset = false;
				drawMode = DrawPoints;
			}
		}
	}
}

void GameView::OnMouseWheel(int x, int y, int d)
{
	if(!d)
		return;
	if(zoomEnabled && !zoomCursorFixed)
	{
		c->AdjustZoomSize(d);
	}
	else
	{
		c->AdjustBrushSize(d);
		if(isMouseDown)
		{
			pointQueue.push(new ui::Point(x, y));
		}
	}
}

void GameView::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	switch(key)
	{
	case KEY_CTRL:
		if(drawModeReset)
			drawModeReset = false;
		else
			drawPoint1 = currentMouse;
		if(shift)
			drawMode = DrawFill;
		else
			drawMode = DrawRect;
		break;
	case KEY_SHIFT:
		if(drawModeReset)
			drawModeReset = false;
		else
			drawPoint1 = currentMouse;
		if(ctrl)
			drawMode = DrawFill;
		else
			drawMode = DrawLine;
		break;
	case ' ': //Space
		c->SetPaused();
		break;
	case KEY_TAB: //Tab
		c->ChangeBrush();
		break;
	case 'z':
		isMouseDown = false;
		zoomCursorFixed = false;
		c->SetZoomEnabled(true);
		break;
	case '`':
		c->ShowConsole();
		break;
	case 'f':
		c->FrameStep();
		break;
	case 'b':
		if(ctrl)
			c->SetDecoration();
		break;
	}
}

void GameView::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(!isMouseDown)
		drawMode = DrawPoints;
	else
		drawModeReset = true;
	switch(key)
	{
	case 'z':
		if(!zoomCursorFixed)
			c->SetZoomEnabled(false);
		break;
	}
}

void GameView::OnTick(float dt)
{
	if(zoomEnabled && !zoomCursorFixed)
		c->SetZoomPosition(currentMouse);
	if(drawMode == DrawPoints)
	{
		if(isMouseDown)
		{
			pointQueue.push(new ui::Point(currentMouse));
		}
		if(!pointQueue.empty())
		{
			c->DrawPoints(toolIndex, pointQueue);
		}
	}
	if(drawMode == DrawFill && isMouseDown)
	{
		c->DrawFill(toolIndex, currentMouse);
	}
	c->Update();
}

void GameView::NotifyZoomChanged(GameModel * sender)
{
	zoomEnabled = sender->GetZoomEnabled();
}

void GameView::changeColour()
{
	c->SetColour(ui::Colour(colourRSlider->GetValue(), colourGSlider->GetValue(), colourBSlider->GetValue()));
}

void GameView::OnDraw()
{
	if(ren)
	{
		ren->render_parts();
		ren->render_fire();
		ren->DrawWalls();
		if(activeBrush && currentMouse.X > 0 && currentMouse.X < XRES && currentMouse.Y > 0 && currentMouse.Y < YRES)
		{
			if(drawMode==DrawRect && isMouseDown)
			{
				activeBrush->RenderRect(ui::Engine::Ref().g, c->PointTranslate(drawPoint1), c->PointTranslate(currentMouse));
			}
			else if(drawMode==DrawLine && isMouseDown)
			{
				activeBrush->RenderLine(ui::Engine::Ref().g, c->PointTranslate(drawPoint1), c->PointTranslate(currentMouse));
			}
			else
			{
				activeBrush->RenderPoint(ui::Engine::Ref().g, c->PointTranslate(currentMouse));
			}
		}
		ren->RenderZoom();
		ren->DrawSigns();
	}
}
