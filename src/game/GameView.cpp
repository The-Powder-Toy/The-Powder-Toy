#include <sstream>

#include "Config.h"
#include "GameView.h"
#include "Graphics.h"
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Colour.h"
#include "interface/Keys.h"
#include "interface/Slider.h"
#include "search/Thumbnail.h"
#include "simulation/SaveRenderer.h"

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
	drawModeReset(false),
	selectMode(SelectNone),
	selectPoint1(0, 0),
	selectPoint2(0, 0),
	stampThumb(NULL),
	clipboardThumb(NULL)
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
	colourASlider = new ui::Slider(ui::Point(275, Size.Y-39), ui::Point(50, 14), 255);
	colourASlider->SetActionCallback(colC);
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
	RemoveComponent(colourASlider);
	colourASlider->SetParentWindow(NULL);
	if(sender->GetColourSelectorVisibility())
	{
		AddComponent(colourRSlider);
		AddComponent(colourGSlider);
		AddComponent(colourBSlider);
		AddComponent(colourASlider);
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
	colourASlider->SetValue(sender->GetColourSelectorColour().Alpha);
	colourASlider->SetColour(ui::Colour(0, 0, 0), ui::Colour(255, 255, 255));
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
		if(sender->GetSave()->GetID())
		{
			std::stringstream tagsStream;
			std::vector<string> tags = sender->GetSave()->GetTags();
			for(int i = 0; i < tags.size(); i++)
			{
				tagsStream << sender->GetSave()->GetTags()[i];
				if(i < tags.size()-1)
					tagsStream << " ";
			}
			tagSimulationButton->SetText(tagsStream.str());
		}
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
		tagSimulationButton->SetText("");
	}
}

void GameView::NotifyBrushChanged(GameModel * sender)
{
	activeBrush = sender->GetBrush();
}

void GameView::OnMouseMove(int x, int y, int dx, int dy)
{
	if(selectMode!=SelectNone)
	{
		if(selectMode==PlaceStamp || selectMode==PlaceClipboard)
			selectPoint1 = ui::Point(x, y);
		if(selectPoint1.X!=-1)
			selectPoint2 = ui::Point(x, y);
		return;
	}
	currentMouse = ui::Point(x, y);
	if(isMouseDown && drawMode == DrawPoints)
	{
		pointQueue.push(new ui::Point(x-dx, y-dy));
		pointQueue.push(new ui::Point(x, y));
	}
}

void GameView::OnMouseDown(int x, int y, unsigned button)
{
	if(selectMode!=SelectNone)
	{
		if(button==BUTTON_LEFT)
		{
			selectPoint1 = ui::Point(x, y);
			selectPoint2 = selectPoint1;
		}
		return;
	}
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
	if(selectMode!=SelectNone)
	{
		if(button==BUTTON_LEFT)
		{
			if(selectMode==PlaceStamp || selectMode==PlaceClipboard)
			{
				Thumbnail * tempThumb = selectMode==PlaceStamp?stampThumb:clipboardThumb;
				if(tempThumb)
				{
					int thumbX = selectPoint2.X - (tempThumb->Size.X/2);
					int thumbY = selectPoint2.Y - (tempThumb->Size.Y/2);

					if(thumbX<0)
						thumbX = 0;
					if(thumbX+(tempThumb->Size.X)>=XRES)
						thumbX = XRES-tempThumb->Size.X;

					if(thumbY<0)
						thumbY = 0;
					if(thumbY+(tempThumb->Size.Y)>=YRES)
						thumbY = YRES-tempThumb->Size.Y;

					if(selectMode==PlaceStamp)
						c->PlaceStamp(ui::Point(thumbX, thumbY));
					if(selectMode==PlaceClipboard)
						c->PlaceClipboard(ui::Point(thumbX, thumbY));
				}
			}
			else
			{
				int x2 = (selectPoint1.X>selectPoint2.X)?selectPoint1.X:selectPoint2.X;
				int y2 = (selectPoint1.Y>selectPoint2.Y)?selectPoint1.Y:selectPoint2.Y;
				int x1 = (selectPoint2.X<selectPoint1.X)?selectPoint2.X:selectPoint1.X;
				int y1 = (selectPoint2.Y<selectPoint1.Y)?selectPoint2.Y:selectPoint1.Y;
				if(x2-x1>0 && y2-y1>0)
				{
					if(selectMode==SelectCopy)
						c->CopyRegion(ui::Point(x1, y1), ui::Point(x2, y2));
					else if(selectMode==SelectStamp)
						c->StampRegion(ui::Point(x1, y1), ui::Point(x2, y2));
				}
			}
		}
		selectMode = SelectNone;
		return;
	}

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
	if(selectMode!=SelectNone)
	{
		return;
	}
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
	if(selectMode!=SelectNone)
	{
		return;
	}
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
	case 's':
		selectMode = SelectStamp;
		selectPoint1 = ui::Point(-1, -1);
		break;
	case 'c':
		if(ctrl)
		{
			selectMode = SelectCopy;
			selectPoint1 = ui::Point(-1, -1);
		}
		break;
	case 'v':
		if(ctrl && clipboardThumb)
		{
			selectMode = PlaceClipboard;
			selectPoint2 = ui::Point(-1, -1);
			selectPoint1 = selectPoint2;
		}
		break;
	case 'l':
		selectMode = PlaceStamp;
		selectPoint2 = ui::Point(-1, -1);
		selectPoint1 = selectPoint2;
		c->OpenStamps();
		break;
	}
}

void GameView::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(selectMode!=SelectNone)
	{
		return;
	}
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
	if(selectMode==PlaceStamp && !stampThumb)
			selectMode = SelectNone;
	if(selectMode==PlaceClipboard&& !clipboardThumb)
			selectMode = SelectNone;
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

void GameView::DoMouseMove(int x, int y, int dx, int dy)
{
	if(c->MouseMove(x, y, dx, dy))
		Window::DoMouseMove(x, y, dx, dy);
}

void GameView::DoMouseDown(int x, int y, unsigned button)
{
	if(c->MouseDown(x, y, button))
		Window::DoMouseDown(x, y, button);
}

void GameView::DoMouseUp(int x, int y, unsigned button)
{
	if(c->MouseUp(x, y, button))
		Window::DoMouseUp(x, y, button);
}

void GameView::DoMouseWheel(int x, int y, int d)
{
	if(c->MouseWheel(x, y, d))
		Window::DoMouseWheel(x, y, d);
}

void GameView::DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(c->KeyPress(key, character, shift, ctrl, alt))
		Window::DoKeyPress(key, character, shift, ctrl, alt);
}

void GameView::DoKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(c->KeyRelease(key, character, shift, ctrl, alt))
		Window::DoKeyRelease(key, character, shift, ctrl, alt);
}

void GameView::NotifyZoomChanged(GameModel * sender)
{
	zoomEnabled = sender->GetZoomEnabled();
}

void GameView::NotifyLogChanged(GameModel * sender, string entry)
{
	logEntries.push_front(entry);
	lastLogEntry = 100.0f;
	if(logEntries.size()>10)
		logEntries.pop_back();
}

void GameView::NotifyClipboardChanged(GameModel * sender)
{
	if(clipboardThumb)
		delete clipboardThumb;
	if(sender->GetClipboard())
	{
		clipboardThumb = SaveRenderer::Ref().Render(sender->GetClipboard()->GetData(), sender->GetClipboard()->GetDataLength());
	}
	else
		clipboardThumb = NULL;
}


void GameView::NotifyStampChanged(GameModel * sender)
{
	if(stampThumb)
		delete stampThumb;
	if(sender->GetStamp())
	{
		stampThumb = SaveRenderer::Ref().Render(sender->GetStamp()->GetData(), sender->GetStamp()->GetDataLength());
	}
	else
		stampThumb = NULL;
}

void GameView::changeColour()
{
	c->SetColour(ui::Colour(colourRSlider->GetValue(), colourGSlider->GetValue(), colourBSlider->GetValue(), colourASlider->GetValue()));
}

void GameView::OnDraw()
{
	if(ren)
	{
		Graphics * g = ui::Engine::Ref().g;
		ren->render_parts();
		ren->render_fire();
		ren->DrawWalls();
		if(activeBrush && currentMouse.X > 0 && currentMouse.X < XRES && currentMouse.Y > 0 && currentMouse.Y < YRES)
		{
			if(drawMode==DrawRect && isMouseDown)
			{
				activeBrush->RenderRect(g, c->PointTranslate(drawPoint1), c->PointTranslate(currentMouse));
			}
			else if(drawMode==DrawLine && isMouseDown)
			{
				activeBrush->RenderLine(g, c->PointTranslate(drawPoint1), c->PointTranslate(currentMouse));
			}
			else
			{
				activeBrush->RenderPoint(g, c->PointTranslate(currentMouse));
			}
		}
		ren->RenderZoom();
		ren->DrawSigns();

		if(selectMode!=SelectNone)
		{
			if(selectMode==PlaceStamp || selectMode==PlaceClipboard)
			{
				Thumbnail * tempThumb = selectMode==PlaceStamp?stampThumb:clipboardThumb;
				if(tempThumb && selectPoint2.X!=-1)
				{
					int thumbX = selectPoint2.X - (tempThumb->Size.X/2);
					int thumbY = selectPoint2.Y - (tempThumb->Size.Y/2);

					if(thumbX<0)
						thumbX = 0;
					if(thumbX+(tempThumb->Size.X)>=XRES)
						thumbX = XRES-tempThumb->Size.X;

					if(thumbY<0)
						thumbY = 0;
					if(thumbY+(tempThumb->Size.Y)>=YRES)
						thumbY = YRES-tempThumb->Size.Y;

					g->draw_image(tempThumb->Data, thumbX, thumbY, tempThumb->Size.X, tempThumb->Size.Y, 128);
				}
			}
			else
			{
				if(selectPoint1.X==-1)
				{
					g->fillrect(0, 0, XRES, YRES, 0, 0, 0, 100);
				}
				else
				{
					int x2 = (selectPoint1.X>selectPoint2.X)?selectPoint1.X:selectPoint2.X;
					int y2 = (selectPoint1.Y>selectPoint2.Y)?selectPoint1.Y:selectPoint2.Y;
					int x1 = (selectPoint2.X<selectPoint1.X)?selectPoint2.X:selectPoint1.X;
					int y1 = (selectPoint2.Y<selectPoint1.Y)?selectPoint2.Y:selectPoint1.Y;

					if(x2>XRES-1)
						x2 = XRES-1;
					if(y2>YRES-1)
						y2 = YRES-1;

					g->fillrect(0, 0, XRES, y1, 0, 0, 0, 100);
					g->fillrect(0, y2, XRES, YRES-y2, 0, 0, 0, 100);

					g->fillrect(0, y1-1, x1, (y2-y1)+2, 0, 0, 0, 100);
					g->fillrect(x2, y1-1, XRES-x2, (y2-y1)+2, 0, 0, 0, 100);

					g->xor_rect(x1, y1, (x2-x1)+1, (y2-y1)+1);
				}
			}
		}

		int startX = 20;
		int startY = YRES-20;
		if(lastLogEntry>0.1 && logEntries.size())
		{
			deque<string>::iterator iter;
			for(iter = logEntries.begin(); iter != logEntries.end(); iter++)
			{
				string message = (*iter);
				startY -= 14;
				g->fillrect(startX-3, startY-3, Graphics::textwidth((char*)message.c_str())+6, 14, 0, 0, 0, 100);
				g->drawtext(startX, startY, message.c_str(), 255, 255, 255, 255);
			}
		}
	}
}
