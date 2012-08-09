#include <sstream>
#include <iomanip>

#include "Config.h"
#include "Style.h"
#include "GameView.h"
#include "graphics/Graphics.h"
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Colour.h"
#include "interface/Keys.h"
#include "interface/Slider.h"
#include "search/Thumbnail.h"
#include "simulation/SaveRenderer.h"
#include "dialogues/ConfirmPrompt.h"
#include "Format.h"
#include "QuickOption.h"
#include "IntroText.h"


class SplitButton;
class SplitButtonAction
{
public:
	virtual void ActionCallbackLeft(ui::Button * sender) {}
	virtual void ActionCallbackRight(ui::Button * sender) {}
	virtual ~SplitButtonAction() {}
};
class SplitButton : public ui::Button
{
private:
	bool rightDown;
	bool leftDown;
	bool showSplit;
	int splitPosition;
	std::string toolTip2;
	SplitButtonAction * splitActionCallback;
public:
	SplitButton(ui::Point position, ui::Point size, std::string buttonText, std::string toolTip, std::string toolTip2, int split) :
		Button(position, size, buttonText, toolTip),
		toolTip2(toolTip2),
		splitPosition(split),
		splitActionCallback(NULL),
		showSplit(true)
	{

	}
	bool GetShowSplit() { return showSplit; }
	void SetShowSplit(bool split) { showSplit = split; }
	SplitButtonAction * GetSplitActionCallback() { return splitActionCallback; }
	void SetSplitActionCallback(SplitButtonAction * newAction) { splitActionCallback = newAction; }
	virtual void OnMouseUp(int x, int y, unsigned int button)
	{
	    if(isButtonDown)
	    {
	    	if(leftDown)
				DoLeftAction();
			else if(rightDown)
				DoRightAction();
	    }
	    ui::Button::OnMouseUp(x, y, button);

	}
	virtual void OnMouseMovedInside(int x, int y, int dx, int dy)
	{
		if(x >= splitPosition)
		{
			if(toolTip.length()>0 && GetParentWindow())
			{
				GetParentWindow()->ToolTip(this, ui::Point(x, y), toolTip);
			}
		}
		else if(x < splitPosition)
		{
			if(toolTip2.length()>0 && GetParentWindow())
			{
				GetParentWindow()->ToolTip(this, ui::Point(x, y), toolTip2);
			}
		}
	}
	virtual void OnMouseEnter(int x, int y)
	{
	    isMouseInside = true;
		if(!Enabled)
			return;
		if(x >= splitPosition)
		{
			if(toolTip.length()>0 && GetParentWindow())
			{
				GetParentWindow()->ToolTip(this, ui::Point(x, y), toolTip);
			}
		}
		else if(x < splitPosition)
		{
			if(toolTip2.length()>0 && GetParentWindow())
			{
				GetParentWindow()->ToolTip(this, ui::Point(x, y), toolTip2);
			}
		}
	}
	virtual void TextPosition()
	{
		ui::Button::TextPosition();
		textPosition.X += 3;
	}
	virtual void OnMouseClick(int x, int y, unsigned int button)
	{
		ui::Button::OnMouseClick(x, y, button);
		rightDown = false;
		leftDown = false;
		if(x >= splitPosition)
			rightDown = true;
		else if(x < splitPosition)
			leftDown = true;
	}
	void DoRightAction()
	{
		if(!Enabled)
			return;
		if(splitActionCallback)
			splitActionCallback->ActionCallbackRight(this);
	}
	void DoLeftAction()
	{
		if(!Enabled)
			return;
		if(splitActionCallback)
			splitActionCallback->ActionCallbackLeft(this);
	}
	void Draw(const ui::Point& screenPos)
	{
		ui::Button::Draw(screenPos);
		Graphics * g = ui::Engine::Ref().g;
		drawn = true;

		if(showSplit)
			g->draw_line(splitPosition+screenPos.X, screenPos.Y+1, splitPosition+screenPos.X, screenPos.Y+Size.Y-2, 180, 180, 180, 255);
	}
	virtual ~SplitButton()
	{
		if(splitActionCallback)
			delete splitActionCallback;
	}
};


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
	placeSaveThumb(NULL),
	mousePosition(0, 0),
	lastOffset(0),
	drawSnap(false),
	toolTip(""),
	infoTip(""),
	infoTipPresence(0),
	toolTipPosition(-1, -1),
	shiftBehaviour(false),
	ctrlBehaviour(false),
	showHud(true),
	showDebug(false),
	introText(2048),
	introTextMessage(introTextData)
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
			if(v->CtrlBehaviour())
				v->c->OpenLocalBrowse();
			else
				v->c->OpenSearch();
		}
	};
	
	scrollBar = new ui::Button(ui::Point(0,YRES+21), ui::Point(XRES, 2), "");
	scrollBar->Appearance.BackgroundInactive = ui::Colour(255, 255, 255);
	scrollBar->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	scrollBar->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(scrollBar);
	
	searchButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(17, 15));  //Open
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
    reloadButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(17, 15));
    reloadButton->SetIcon(IconReload);
    reloadButton->Appearance.Margin.Left+=2;
    currentX+=18;
    reloadButton->SetActionCallback(new ReloadAction(this));
    AddComponent(reloadButton);

    class SaveSimulationAction : public SplitButtonAction
    {
        GameView * v;
    public:
        SaveSimulationAction(GameView * _v) { v = _v; }
        void ActionCallbackRight(ui::Button * sender)
        {
        	if(v->CtrlBehaviour())
        		v->c->OpenLocalSaveWindow();
        	else
	            v->c->OpenSaveWindow();
        }
        void ActionCallbackLeft(ui::Button * sender)
        {
        	if(v->CtrlBehaviour())
        		v->c->OpenLocalSaveWindow();
        	else
	            v->c->SaveAsCurrent();
        }
    };
    saveSimulationButton = new SplitButton(ui::Point(currentX, Size.Y-16), ui::Point(150, 15), "[untitled simulation]", "Save game as current name", "Save game as new name", 19);
	saveSimulationButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
    saveSimulationButton->SetIcon(IconSave);
    currentX+=151;
    ((SplitButton*)saveSimulationButton)->SetSplitActionCallback(new SaveSimulationAction(this));
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
    upVoteButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(15, 15));
    upVoteButton->SetIcon(IconVoteUp);
    upVoteButton->Appearance.Margin.Top+=2;
    upVoteButton->Appearance.Margin.Left+=2;
    currentX+=14;
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
    downVoteButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(15, 15));
    downVoteButton->SetIcon(IconVoteDown);
    downVoteButton->Appearance.Margin.Bottom+=2;
    downVoteButton->Appearance.Margin.Left+=2;
    currentX+=16;
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
    tagSimulationButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(251, 15), "[no tags set]");
	tagSimulationButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
    tagSimulationButton->SetIcon(IconTag);
    currentX+=252;
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
    clearSimButton = new ui::Button(ui::Point(Size.X-159, Size.Y-16), ui::Point(17, 15));
    clearSimButton->SetIcon(IconNew);
    clearSimButton->Appearance.Margin.Left+=2;
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
    loginButton = new ui::Button(ui::Point(Size.X-141, Size.Y-16), ui::Point(92, 15), "[sign in]");
	loginButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
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
            v->c->OpenOptions();
        }
    };
    simulationOptionButton = new ui::Button(ui::Point(Size.X-48, Size.Y-16), ui::Point(15, 15));
    simulationOptionButton->SetIcon(IconSimulationSettings);
    simulationOptionButton->Appearance.Margin.Left+=2;
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
    displayModeButton = new ui::Button(ui::Point(Size.X-32, Size.Y-16), ui::Point(15, 15));
    displayModeButton->SetIcon(IconRenderSettings);
    displayModeButton->Appearance.Margin.Left+=2;
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
	pauseButton = new ui::Button(ui::Point(Size.X-16, Size.Y-16), ui::Point(15, 15));  //Pause
	pauseButton->SetIcon(IconPause);
	pauseButton->SetTogglable(true);
	pauseButton->SetActionCallback(new PauseAction(this));
	AddComponent(pauseButton);

	class ColourChange : public ui::SliderAction, public ui::TextboxAction
	{
		GameView * v;
	public:
		ColourChange(GameView * _v) { v = _v; }
		void ValueChangedCallback(ui::Slider * sender)
		{
			v->changeColourSlider();
		}

		void TextChangedCallback(ui::Textbox * sender)
		{
			v->changeColourText();
		}
	};
	colourRSlider = new ui::Slider(ui::Point(5, Size.Y-39), ui::Point(50, 14), 255);
	colourRSlider->SetActionCallback(new ColourChange(this));
	colourRValue = new ui::Textbox(ui::Point(60, Size.Y-41), ui::Point(25, 17), "255");
	colourRValue->SetActionCallback(new ColourChange(this));
	colourRValue->SetLimit(3);
	colourRValue->SetInputType(ui::Textbox::Number);

	colourGSlider = new ui::Slider(ui::Point(95, Size.Y-39), ui::Point(50, 14), 255);
	colourGSlider->SetActionCallback(new ColourChange(this));
	colourGValue = new ui::Textbox(ui::Point(150, Size.Y-41), ui::Point(25, 17), "255");
	colourGValue->SetActionCallback(new ColourChange(this));
	colourGValue->SetLimit(3);
	colourGValue->SetInputType(ui::Textbox::Number);

	colourBSlider = new ui::Slider(ui::Point(185, Size.Y-39), ui::Point(50, 14), 255);
	colourBSlider->SetActionCallback(new ColourChange(this));
	colourBValue = new ui::Textbox(ui::Point(240, Size.Y-41), ui::Point(25, 17), "255");
	colourBValue->SetActionCallback(new ColourChange(this));
	colourBValue->SetLimit(3);
	colourBValue->SetInputType(ui::Textbox::Number);

	colourASlider = new ui::Slider(ui::Point(275, Size.Y-39), ui::Point(50, 14), 255);
	colourASlider->SetActionCallback(new ColourChange(this));
	colourAValue = new ui::Textbox(ui::Point(330, Size.Y-41), ui::Point(25, 17), "255");
	colourAValue->SetActionCallback(new ColourChange(this));
	colourAValue->SetLimit(3);
	colourAValue->SetInputType(ui::Textbox::Number);

	class ElementSearchAction : public ui::ButtonAction
	{
		GameView * v;
	public:
		ElementSearchAction(GameView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->OpenElementSearch();
		}
	};
	ui::Button * tempButton = new ui::Button(ui::Point(XRES+BARSIZE-16, YRES+MENUSIZE-32), ui::Point(15, 15), "\xE5");
	tempButton->Appearance.Margin = ui::Border(0, 2, 3, 2);
	tempButton->SetActionCallback(new ElementSearchAction(this));
	AddComponent(tempButton);

	//Render mode presets. Possibly load from config in future?
	renderModePresets = new RenderPreset[10];

	renderModePresets[0].Name = "Alternative Velocity Display";
	renderModePresets[0].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[0].RenderModes.push_back(RENDER_BASC);
	renderModePresets[0].DisplayModes.push_back(DISPLAY_AIRC);

	renderModePresets[1].Name = "Velocity Display";
	renderModePresets[1].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[1].RenderModes.push_back(RENDER_BASC);
	renderModePresets[1].DisplayModes.push_back(DISPLAY_AIRV);

	renderModePresets[2].Name = "Pressure Display";
	renderModePresets[2].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[2].RenderModes.push_back(RENDER_BASC);
	renderModePresets[2].DisplayModes.push_back(DISPLAY_AIRP);

	renderModePresets[3].Name = "Persistent Display";
	renderModePresets[3].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[3].RenderModes.push_back(RENDER_BASC);
	renderModePresets[3].DisplayModes.push_back(DISPLAY_PERS);

	renderModePresets[4].Name = "Fire Display";
	renderModePresets[4].RenderModes.push_back(RENDER_FIRE);
	renderModePresets[4].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[4].RenderModes.push_back(RENDER_BASC);

	renderModePresets[5].Name = "Blob Display";
	renderModePresets[5].RenderModes.push_back(RENDER_FIRE);
	renderModePresets[5].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[5].RenderModes.push_back(RENDER_BLOB);

	renderModePresets[6].Name = "Heat Display";
	renderModePresets[6].RenderModes.push_back(RENDER_BASC);
	renderModePresets[6].DisplayModes.push_back(DISPLAY_AIRH);
	renderModePresets[6].ColourMode = COLOUR_HEAT;

	renderModePresets[7].Name = "Fancy Display";
	renderModePresets[7].RenderModes.push_back(RENDER_FIRE);
	renderModePresets[7].RenderModes.push_back(RENDER_GLOW);
	renderModePresets[7].RenderModes.push_back(RENDER_BLUR);
	renderModePresets[7].RenderModes.push_back(RENDER_EFFE);
	renderModePresets[7].RenderModes.push_back(RENDER_BASC);
	renderModePresets[7].DisplayModes.push_back(DISPLAY_WARP);

	renderModePresets[8].Name = "Nothing Display";
	renderModePresets[8].RenderModes.push_back(RENDER_BASC);

	renderModePresets[9].Name = "Heat Gradient Display";
	renderModePresets[9].RenderModes.push_back(RENDER_BASC);
	renderModePresets[9].ColourMode = COLOUR_GRAD;
}

class GameView::MenuAction: public ui::ButtonAction
{
	GameView * v;
public:
	Menu * menu;
	MenuAction(GameView * _v, Menu * menu_) { v = _v; menu = menu_; }
	void MouseEnterCallback(ui::Button * sender)
	{
		v->c->SetActiveMenu(menu);
	}
	void ActionCallback(ui::Button * sender)
	{
		MouseEnterCallback(sender);
	}
};

class GameView::OptionAction: public ui::ButtonAction
{
	QuickOption * option;
public:
	OptionAction(QuickOption * _option) { option = _option; }
	void ActionCallback(ui::Button * sender)
	{
		option->Perform();
	}
};

class GameView::OptionListener: public QuickOptionListener
{
	ui::Button * button;
public:
	OptionListener(ui::Button * _button) { button = _button; }
	virtual void OnValueChanged(QuickOption * option)
	{
		switch(option->GetType())
		{
		case QuickOption::Toggle:
			button->SetTogglable(true);
			button->SetToggleState(option->GetToggle());
		}
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

void GameView::NotifyQuickOptionsChanged(GameModel * sender)
{
	for(int i = 0; i < quickOptionButtons.size(); i++)
	{
		RemoveComponent(quickOptionButtons[i]);
		delete quickOptionButtons[i];
	}

	int currentY = 1;
	vector<QuickOption*> optionList = sender->GetQuickOptions();
	for(vector<QuickOption*>::iterator iter = optionList.begin(), end = optionList.end(); iter != end; ++iter)
	{
		QuickOption * option = *iter;
		ui::Button * tempButton = new ui::Button(ui::Point(XRES+BARSIZE-16, currentY), ui::Point(15, 15), option->GetIcon(), option->GetDescription());
		//tempButton->Appearance.Margin = ui::Border(0, 2, 3, 2);
		tempButton->SetTogglable(true);
		tempButton->SetActionCallback(new OptionAction(option));
		option->AddListener(new OptionListener(tempButton));
		AddComponent(tempButton);

		quickOptionButtons.push_back(tempButton);
		currentY += 16;
	}
}

void GameView::NotifyMenuListChanged(GameModel * sender)
{
	int currentY = YRES+MENUSIZE-48;//-(sender->GetMenuList().size()*16);
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
	for(vector<Menu*>::reverse_iterator iter = menuList.rbegin(), end = menuList.rend(); iter != end; ++iter)
	{
		std::string tempString = "";
		Menu * item = *iter;
		tempString += item->GetIcon();
		ui::Button * tempButton = new ui::Button(ui::Point(XRES+BARSIZE-16, currentY), ui::Point(15, 15), tempString, item->GetDescription());
		tempButton->Appearance.Margin = ui::Border(0, 2, 3, 2);
		tempButton->SetTogglable(true);
		tempButton->SetActionCallback(new MenuAction(this, item));
		currentY-=16;
		AddComponent(tempButton);
		menuButtons.push_back(tempButton);
	}
}

void GameView::SetSample(SimulationSample sample)
{
	this->sample = sample;
}

ui::Point GameView::GetMousePosition()
{
	return mousePosition;
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
	lastOffset = 0;
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
		VideoBuffer * tempTexture = toolList[i]->GetTexture(26, 14);
		ToolButton * tempButton;

		if(tempTexture)
			tempButton = new ToolButton(ui::Point(currentX, YRES+1), ui::Point(30, 18), "", toolList[i]->GetDescription());
		else
			tempButton = new ToolButton(ui::Point(currentX, YRES+1), ui::Point(30, 18), toolList[i]->GetName(), toolList[i]->GetDescription());

		//currentY -= 17;
		currentX -= 31;
		tempButton->SetActionCallback(new ToolAction(this, toolList[i]));

		tempButton->Appearance.SetTexture(tempTexture);
		if(tempTexture)
			delete tempTexture;

		tempButton->Appearance.BackgroundInactive = ui::Colour(toolList[i]->colRed, toolList[i]->colGreen, toolList[i]->colBlue);

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

		tempButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
		tempButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
		AddComponent(tempButton);
		toolButtons.push_back(tempButton);
	}

}

void GameView::NotifyColourSelectorVisibilityChanged(GameModel * sender)
{
	RemoveComponent(colourRSlider);
	colourRSlider->SetParentWindow(NULL);
	RemoveComponent(colourRValue);
	colourRValue->SetParentWindow(NULL);

	RemoveComponent(colourGSlider);
	colourGSlider->SetParentWindow(NULL);
	RemoveComponent(colourGValue);
	colourGValue->SetParentWindow(NULL);

	RemoveComponent(colourBSlider);
	colourBSlider->SetParentWindow(NULL);
	RemoveComponent(colourBValue);
	colourBValue->SetParentWindow(NULL);

	RemoveComponent(colourASlider);
	colourASlider->SetParentWindow(NULL);
	RemoveComponent(colourAValue);
	colourAValue->SetParentWindow(NULL);

	if(sender->GetColourSelectorVisibility())
	{
		AddComponent(colourRSlider);
		AddComponent(colourRValue);

		AddComponent(colourGSlider);
		AddComponent(colourGValue);

		AddComponent(colourBSlider);
		AddComponent(colourBValue);

		AddComponent(colourASlider);
		AddComponent(colourAValue);
	}

}

void GameView::NotifyColourSelectorColourChanged(GameModel * sender)
{
	std::string intR, intG, intB, intA;

	intR = format::NumberToString<int>(sender->GetColourSelectorColour().Red);
	intG = format::NumberToString<int>(sender->GetColourSelectorColour().Green);
	intB = format::NumberToString<int>(sender->GetColourSelectorColour().Blue);
	intA = format::NumberToString<int>(sender->GetColourSelectorColour().Alpha);

	colourRSlider->SetValue(sender->GetColourSelectorColour().Red);
	colourRSlider->SetColour(ui::Colour(0, 0, 0), ui::Colour(255, 0, 0));
	if(!colourRValue->IsFocused())
		colourRValue->SetText(intR);

	colourGSlider->SetValue(sender->GetColourSelectorColour().Green);
	colourGSlider->SetColour(ui::Colour(0, 0, 0), ui::Colour(0, 255, 0));
	if(!colourGValue->IsFocused())
		colourGValue->SetText(intG);

	colourBSlider->SetValue(sender->GetColourSelectorColour().Blue);
	colourBSlider->SetColour(ui::Colour(0, 0, 0), ui::Colour(0, 0, 255));
	if(!colourBValue->IsFocused())
		colourBValue->SetText(intB);

	colourASlider->SetValue(sender->GetColourSelectorColour().Alpha);
	colourASlider->SetColour(ui::Colour(0, 0, 0), ui::Colour(sender->GetColourSelectorColour().Red, sender->GetColourSelectorColour().Green, sender->GetColourSelectorColour().Blue));
	if(!colourAValue->IsFocused())
		colourAValue->SetText(intA);
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
		loginButton->SetText("[sign in]");
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

void GameView::NotifyToolTipChanged(GameModel * sender)
{
	toolTip = sender->GetToolTip();
}

void GameView::NotifyInfoTipChanged(GameModel * sender)
{
	infoTip = sender->GetInfoTip();
	infoTipPresence = 120;
}

void GameView::NotifySaveChanged(GameModel * sender)
{
	if(sender->GetSave())
	{
		if(introText > 50)
			introText = 50;

		saveSimulationButton->SetText(sender->GetSave()->GetName());
		if(sender->GetSave()->GetUserName() == sender->GetUser().Username)
			((SplitButton*)saveSimulationButton)->SetShowSplit(true);
		else
			((SplitButton*)saveSimulationButton)->SetShowSplit(false);
		reloadButton->Enabled = true;
		upVoteButton->Enabled = (sender->GetSave()->GetID() && sender->GetUser().ID && sender->GetSave()->GetVote()==0);
		if(sender->GetSave()->GetID() && sender->GetUser().ID && sender->GetSave()->GetVote()==1)
			upVoteButton->Appearance.BackgroundInactive = (ui::Colour(0, 200, 40));
		else
			upVoteButton->Appearance.BackgroundInactive = (ui::Colour(0, 0, 0));
		downVoteButton->Enabled = upVoteButton->Enabled;
		if(sender->GetSave()->GetID() && sender->GetUser().ID && sender->GetSave()->GetVote()==-1)
			downVoteButton->Appearance.BackgroundInactive = (ui::Colour(200, 40, 40));
		else
			downVoteButton->Appearance.BackgroundInactive = (ui::Colour(0, 0, 0));
		tagSimulationButton->Enabled = (sender->GetSave()->GetID() && sender->GetUser().ID);
		if(sender->GetSave()->GetID())
		{
			std::stringstream tagsStream;
			std::vector<string> tags = sender->GetSave()->GetTags();
			if(tags.size())
			{
				for(int i = 0; i < tags.size(); i++)
				{
					tagsStream << sender->GetSave()->GetTags()[i];
					if(i < tags.size()-1)
						tagsStream << " ";
				}
				tagSimulationButton->SetText(tagsStream.str());
			}
			else
			{
				tagSimulationButton->SetText("[no tags set]");
			}
		}
		else
		{
			tagSimulationButton->SetText("[no tags set]");
		}
	}
	else
	{
		((SplitButton*)saveSimulationButton)->SetShowSplit(false);
		saveSimulationButton->SetText("[untitled simulation]");
		reloadButton->Enabled = false;
		upVoteButton->Enabled = false;
		upVoteButton->Appearance.BackgroundInactive = (ui::Colour(0, 0, 0));
		downVoteButton->Enabled = false;
		upVoteButton->Appearance.BackgroundInactive = (ui::Colour(0, 0, 0));
		tagSimulationButton->Enabled = false;
		tagSimulationButton->SetText("[no tags set]");
	}
}

void GameView::NotifyBrushChanged(GameModel * sender)
{
	activeBrush = sender->GetBrush();
}

void GameView::setToolButtonOffset(int offset)
{
	int offset_ = offset;
	offset = offset-lastOffset;
	lastOffset = offset_;

	for(vector<ToolButton*>::iterator iter = toolButtons.begin(), end = toolButtons.end(); iter!=end; ++iter)
	{
		ToolButton * button = *iter;
		button->Position.X -= offset;
		if(button->Position.X <= 0 || (button->Position.X+button->Size.X) > XRES-2) {
			button->Visible = false;
		} else {
			button->Visible = true;
		}
	}
}

void GameView::OnMouseMove(int x, int y, int dx, int dy)
{
	mousePosition = c->PointTranslate(ui::Point(x, y));
	if(selectMode!=SelectNone)
	{
		if(selectMode==PlaceSave)
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
			if(selectMode==PlaceSave)
			{
				Thumbnail * tempThumb = placeSaveThumb;
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

					c->PlaceSave(ui::Point(thumbX, thumbY));
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
				ui::Point finalDrawPoint2(0, 0);
				drawPoint2 = ui::Point(x, y);
				finalDrawPoint2 = drawPoint2;

				if(drawSnap && drawMode == DrawLine)
				{
					finalDrawPoint2 = lineSnapCoords(drawPoint1, drawPoint2);
				}

				if(drawSnap && drawMode == DrawRect)
				{
					finalDrawPoint2 = rectSnapCoords(drawPoint1, drawPoint2);
				}

				if(drawMode == DrawRect)
				{
					c->DrawRect(toolIndex, drawPoint1, finalDrawPoint2);
				}
				if(drawMode == DrawLine)
				{
					c->DrawLine(toolIndex, drawPoint1, finalDrawPoint2);
				}
			}
			if(drawMode == DrawPoints)
			{
				c->ToolClick(toolIndex, ui::Point(x, y));
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

void GameView::ExitPrompt()
{
	class ExitConfirmation: public ConfirmDialogueCallback {
	public:
		ExitConfirmation() {}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
			{
				ui::Engine::Ref().Exit();
			}
		}
		virtual ~ExitConfirmation() { }
	};
	new ConfirmPrompt("You are about to quit", "Are you sure you want to exit the game?", new ExitConfirmation());
}

void GameView::ToolTip(ui::Component * sender, ui::Point mousePosition, std::string toolTip)
{
	this->toolTip = toolTip;
	toolTipPosition = ui::Point(Size.X-27-Graphics::textwidth((char*)toolTip.c_str()), Size.Y-MENUSIZE-10);
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
		c->AdjustBrushSize(d, false, shiftBehaviour, ctrlBehaviour);
		if(isMouseDown)
		{
			pointQueue.push(new ui::Point(x, y));
		}
	}
}

void GameView::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(colourRValue->IsFocused() || colourGValue->IsFocused() || colourBValue->IsFocused() || colourAValue->IsFocused())
		return;

	if(introText > 50)
	{
		introText = 50;
	}

	if(selectMode!=SelectNone)
	{
		if(selectMode==PlaceSave)
		{
			switch(key)
			{
			case KEY_RIGHT:
			case 'd':
				c->TranslateSave(ui::Point(1, 0));
				break;
			case KEY_LEFT:
			case 'a':
				c->TranslateSave(ui::Point(-1, 0));
				break;
			case KEY_UP:
			case 'w':
				c->TranslateSave(ui::Point(0, -1));
				break;
			case KEY_DOWN:
			case 's':
				c->TranslateSave(ui::Point(0, 1));
				break;
			case 'r':
				if(shift)
				{
					//Flip
					c->TransformSave(m2d_new(-1,0,0,1));
				}
				else
				{
					//Rotate 90deg
					c->TransformSave(m2d_new(0,1,-1,0));
				}
				break;
			}
		}
		return;
	}
	switch(key)
	{
	case KEY_ALT:
		drawSnap = true;
		enableAltBehaviour();
		break;
	case KEY_CTRL:
		if(drawModeReset)
			drawModeReset = false;
		else
			drawPoint1 = currentMouse;
		if(shift)
			drawMode = DrawFill;
		else
			drawMode = DrawRect;
		enableCtrlBehaviour();
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
		enableShiftBehaviour();
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
	case 'e':
		c->OpenElementSearch();
		break;
	case 'f':
		c->FrameStep();
		break;
	case 'd':
		showDebug = !showDebug;
		break;
	case KEY_F1:
		if(!introText)
			introText = 8047;
		else
			introText = 0;
		break;
	case 'h':
		if(ctrl)
		{
			if(!introText)
				introText = 8047;
			else
				introText = 0;
		}
		else
			showHud = !showHud;
		break;
	case 'b':
		if(ctrl)
			c->SetDecoration();
		break;
	case 's':
		selectMode = SelectStamp;
		selectPoint1 = ui::Point(-1, -1);
		break;
	case 'w':
		c->SwitchGravity();
		break;
	case 'y':
		c->SwitchAir();
		break;
	case KEY_ESCAPE:
	case 'q':
		ExitPrompt();
		break;
	case 'u':
		c->ToggleAHeat();
		break;
	case '=':
		if(ctrl)
			c->ResetSpark();
		else
			c->ResetAir();
		break;
	case 'c':
		if(ctrl)
		{
			selectMode = SelectCopy;
			selectPoint1 = ui::Point(-1, -1);
		}
		break;
	case 'v':
		if(ctrl)
		{
			c->LoadClipboard();
			selectPoint2 = ui::Point(-1, -1);
			selectPoint1 = selectPoint2;
		}
		break;
	case 'l':
		c->LoadStamp();
		selectPoint2 = ui::Point(-1, -1);
		selectPoint1 = selectPoint2;
		break;
	case 'k':
		selectPoint2 = ui::Point(-1, -1);
		selectPoint1 = selectPoint2;
		c->OpenStamps();
		break;
	case ']':
		c->AdjustBrushSize(1, !alt, shiftBehaviour, ctrlBehaviour);
		break;
	case '[':
		c->AdjustBrushSize(-1, !alt, shiftBehaviour, ctrlBehaviour);
		break;
	}

	if(key >= '0' && key <= '9')
	{
		c->LoadRenderPreset(renderModePresets[key-'0']);
	}
}

void GameView::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(colourRValue->IsFocused() || colourGValue->IsFocused() || colourBValue->IsFocused() || colourAValue->IsFocused())
		return;

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
	case KEY_ALT:
		drawSnap = false;
		disableAltBehaviour();
		break;
	case KEY_CTRL:
		disableCtrlBehaviour();
		break;
	case KEY_SHIFT:
		disableShiftBehaviour();
		break;
	case 'z':
		if(!zoomCursorFixed)
			c->SetZoomEnabled(false);
		break;
	}
}

void GameView::OnBlur()
{
	disableAltBehaviour();
	disableCtrlBehaviour();
	disableShiftBehaviour();
}

void GameView::OnTick(float dt)
{
	if(selectMode==PlaceSave && !placeSaveThumb)
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
	if(introText)
	{
		introText -= int(dt)>0?std::min(int(dt), 5):1;
		if(introText < 0)
			introText  = 0;
	}
	if(infoTipPresence>0)
	{
		infoTipPresence -= int(dt)>0?int(dt):1;
		if(infoTipPresence<0)
			infoTipPresence = 0;
	}
	c->Update();
	if(lastLogEntry > -0.1f)
		lastLogEntry -= 0.16*dt;
}


void GameView::DoMouseMove(int x, int y, int dx, int dy)
{
	if(c->MouseMove(x, y, dx, dy))
		Window::DoMouseMove(x, y, dx, dy);

	if(toolButtons.size())
	{
		int totalWidth = (toolButtons[0]->Size.X+1)*toolButtons.size();
		int scrollSize = (int)(((float)(XRES-15))/((float)totalWidth) * ((float)XRES-15));
		if (scrollSize>XRES)
			scrollSize = XRES;
		if(totalWidth > XRES-15)
		{
			int mouseX = x;
			if(mouseX > XRES)
				mouseX = XRES;
				
			scrollBar->Position.X = (int)(((float)mouseX/((float)XRES-15))*(float)(XRES-scrollSize));
					
			float overflow = totalWidth-(XRES-15), mouseLocation = float(XRES)/float(mouseX-(XRES));
			setToolButtonOffset(overflow/mouseLocation);
			
			//Ensure that mouseLeave events are make their way to the buttons should they move from underneith the mouse pointer
			if(toolButtons[0]->Position.Y < y && toolButtons[0]->Position.Y+toolButtons[0]->Size.Y > y)
			{
				for(vector<ToolButton*>::iterator iter = toolButtons.begin(), end = toolButtons.end(); iter!=end; ++iter)
				{
					ToolButton * button = *iter;
					if(button->Position.X < x && button->Position.X+button->Size.X > x)
						button->OnMouseEnter(x, y);
					else
						button->OnMouseLeave(x, y);
				}
			}
		}
		else
		{
			scrollBar->Position.X = 0;
		}
		scrollBar->Size.X=scrollSize;
	}
}

void GameView::DoMouseDown(int x, int y, unsigned button)
{
	if(introText > 50)
		introText = 50;
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

void GameView::DoDraw()
{
	Window::DoDraw();
	c->Tick();
}

void GameView::NotifyNotificationsChanged(GameModel * sender)
{
    class NotificationButtonAction : public ui::ButtonAction
    {
        GameView * v;
        Notification * notification;
    public:
        NotificationButtonAction(GameView * v, Notification * notification) : v(v), notification(notification) { }
        void ActionCallback(ui::Button * sender)
        {
        	notification->Action();
            //v->c->RemoveNotification(notification);
        }
    };
    class CloseNotificationButtonAction : public ui::ButtonAction
    {
        GameView * v;
        Notification * notification;
    public:
        CloseNotificationButtonAction(GameView * v, Notification * notification) : v(v), notification(notification) { }
        void ActionCallback(ui::Button * sender)
        {
            v->c->RemoveNotification(notification);
        }
    };

	for(std::vector<ui::Component*>::const_iterator iter = notificationComponents.begin(), end = notificationComponents.end(); iter != end; ++iter) {
		ui::Component * cNotification = *iter;
		RemoveComponent(cNotification);
		delete cNotification;
	}
	notificationComponents.clear();


	std::vector<Notification*> notifications = sender->GetNotifications();

	int currentY = YRES-23;
	for(std::vector<Notification*>::iterator iter = notifications.begin(), end = notifications.end(); iter != end; ++iter)
	{
		int width = (Graphics::textwidth((*iter)->Message.c_str()))+8;
		ui::Button * tempButton = new ui::Button(ui::Point(XRES-width-22, currentY), ui::Point(width, 15), (*iter)->Message);
		tempButton->SetActionCallback(new NotificationButtonAction(this, *iter));
		tempButton->Appearance.BorderInactive = style::Colour::WarningTitle;
		tempButton->Appearance.TextInactive = style::Colour::WarningTitle;
		AddComponent(tempButton);
		notificationComponents.push_back(tempButton);

		tempButton = new ui::Button(ui::Point(XRES-20, currentY), ui::Point(15, 15));
		tempButton->SetIcon(IconClose);
		tempButton->SetActionCallback(new CloseNotificationButtonAction(this, *iter));
		tempButton->Appearance.Margin.Left+=2;
		tempButton->Appearance.Margin.Top+=2;
		tempButton->Appearance.BorderInactive = style::Colour::WarningTitle;
		tempButton->Appearance.TextInactive = style::Colour::WarningTitle;
		AddComponent(tempButton);
		notificationComponents.push_back(tempButton);

		currentY -= 17;
	}
}

void GameView::NotifyZoomChanged(GameModel * sender)
{
	zoomEnabled = sender->GetZoomEnabled();
}

void GameView::NotifyLogChanged(GameModel * sender, string entry)
{
	logEntries.push_front(entry);
	lastLogEntry = 100.0f;
	if(logEntries.size()>20)
		logEntries.pop_back();
}

void GameView::NotifyPlaceSaveChanged(GameModel * sender)
{
	if(placeSaveThumb)
		delete placeSaveThumb;
	if(sender->GetPlaceSave())
	{
		placeSaveThumb = SaveRenderer::Ref().Render(sender->GetPlaceSave());
		selectMode = PlaceSave;
	}
	else
	{
		placeSaveThumb = NULL;
		selectMode = SelectNone;
	}
}

void GameView::changeColourSlider()
{
	c->SetColour(ui::Colour(colourRSlider->GetValue(), colourGSlider->GetValue(), colourBSlider->GetValue(), colourASlider->GetValue()));
}

void GameView::changeColourText()
{
	c->SetColour(ui::Colour(
		std::min(255U, format::StringToNumber<unsigned int>(colourRValue->GetText())),
		std::min(255U, format::StringToNumber<unsigned int>(colourGValue->GetText())),
		std::min(255U, format::StringToNumber<unsigned int>(colourBValue->GetText())),
		std::min(255U, format::StringToNumber<unsigned int>(colourAValue->GetText())))
	);
}

void GameView::enableShiftBehaviour()
{
	if(!shiftBehaviour)
	{
		shiftBehaviour = true;
	}
}

void GameView::disableShiftBehaviour()
{
	if(shiftBehaviour)
	{
		shiftBehaviour = false;
	}
}

void GameView::enableAltBehaviour()
{
	if(!altBehaviour)
	{
		altBehaviour = true;
	}
}

void GameView::disableAltBehaviour()
{
	if(altBehaviour)
	{
		altBehaviour = false;
	}
}

void GameView::enableCtrlBehaviour()
{
	if(!ctrlBehaviour)
	{
		ctrlBehaviour = true;

		//Show HDD save & load buttons
		saveSimulationButton->Appearance.BackgroundInactive = ui::Colour(255, 255, 255);
		saveSimulationButton->Appearance.TextInactive = ui::Colour(0, 0, 0);
		searchButton->Appearance.BackgroundInactive = ui::Colour(255, 255, 255);
		searchButton->Appearance.TextInactive = ui::Colour(0, 0, 0);
	}
}

void GameView::disableCtrlBehaviour()
{
	if(ctrlBehaviour)
	{
		ctrlBehaviour = false;

		//Hide HDD save & load buttons
		saveSimulationButton->Appearance.BackgroundInactive = ui::Colour(0, 0, 0);
		saveSimulationButton->Appearance.TextInactive = ui::Colour(255, 255, 255);
		searchButton->Appearance.BackgroundInactive = ui::Colour(0, 0, 0);
		searchButton->Appearance.TextInactive = ui::Colour(255, 255, 255);
	}
}

void GameView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	if(ren)
	{
		ren->clearScreen(1.0f);
		ren->RenderBegin();
		if(selectMode == SelectNone && activeBrush && currentMouse.X > 0 && currentMouse.X < XRES && currentMouse.Y > 0 && currentMouse.Y < YRES)
		{
			ui::Point finalCurrentMouse = c->PointTranslate(currentMouse);

			if(drawMode==DrawRect && isMouseDown)
			{
				if(drawSnap)
				{
					finalCurrentMouse = rectSnapCoords(c->PointTranslate(drawPoint1), finalCurrentMouse);
				}
				activeBrush->RenderRect(ren, c->PointTranslate(drawPoint1), finalCurrentMouse);
			}
			else if(drawMode==DrawLine && isMouseDown)
			{
				if(drawSnap)
				{
					finalCurrentMouse = lineSnapCoords(c->PointTranslate(drawPoint1), finalCurrentMouse);
				}
				activeBrush->RenderLine(ren, c->PointTranslate(drawPoint1), finalCurrentMouse);
			}
			else if(drawMode==DrawFill)
			{
				activeBrush->RenderFill(ren, finalCurrentMouse);
			}
			else
			{
				activeBrush->RenderPoint(ren, finalCurrentMouse);
			}
		}
		ren->RenderEnd();

		if(selectMode!=SelectNone)
		{
			if(selectMode==PlaceSave)
			{
				Thumbnail * tempThumb = placeSaveThumb;
				if(tempThumb && selectPoint2.X!=-1)
				{
					int thumbX = selectPoint2.X - (tempThumb->Size.X/2);
					int thumbY = selectPoint2.Y - (tempThumb->Size.Y/2);

					ui::Point thumbPos = c->NormaliseBlockCoord(ui::Point(thumbX, thumbY));

					if(thumbPos.X<0)
						thumbPos.X = 0;
					if(thumbPos.X+(tempThumb->Size.X)>=XRES)
						thumbPos.X = XRES-tempThumb->Size.X;

					if(thumbPos.Y<0)
						thumbPos.Y = 0;
					if(thumbPos.Y+(tempThumb->Size.Y)>=YRES)
						thumbPos.Y = YRES-tempThumb->Size.Y;

					g->draw_image(tempThumb->Data, thumbPos.X, thumbPos.Y, tempThumb->Size.X, tempThumb->Size.Y, 128);
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

					g->fillrect(0, y1, x1, (y2-y1), 0, 0, 0, 100);
					g->fillrect(x2, y1, XRES-x2, (y2-y1), 0, 0, 0, 100);

					g->xor_rect(x1, y1, (x2-x1)+1, (y2-y1)+1);
				}
			}
		}

		int startX = 20;
		int startY = YRES-20;
		int startAlpha;
		if(lastLogEntry>0.1f && logEntries.size())
		{
			startAlpha = 2.55f*lastLogEntry;
			deque<string>::iterator iter;
			for(iter = logEntries.begin(); iter != logEntries.end() && startAlpha>0; iter++)
			{
				string message = (*iter);
				startY -= 13;
				g->fillrect(startX-3, startY-3, Graphics::textwidth((char*)message.c_str())+6                                                                                                                                        , 14, 0, 0, 0, 100);
				g->drawtext(startX, startY, message.c_str(), 255, 255, 255, startAlpha);
				startAlpha-=14;
			}
		}
	}

	if(showHud && !introText)
	{
		//Draw info about simulation under cursor
		std::stringstream sampleInfo;
		sampleInfo.precision(2);
		if(sample.particle.type)
		{	
			if(showDebug)
			{
				sampleInfo << c->ElementResolve(sample.particle.type);
				if(sample.particle.ctype > 0 && sample.particle.ctype < PT_NUM)
				{
					sampleInfo << " (" << c->ElementResolve(sample.particle.ctype) << ")";
				}
				sampleInfo << ", Pressure: " << std::fixed << sample.AirPressure;
				sampleInfo << ", Temp: " << std::fixed << sample.particle.temp -273.15f;
				sampleInfo << ", Life: " << sample.particle.life;
				sampleInfo << ", Temp: " << sample.particle.tmp;
			}
			else
			{
				sampleInfo << c->ElementResolve(sample.particle.type) << ", Pressure: " << std::fixed << sample.AirPressure;
				sampleInfo << ", Temp: " << std::fixed << sample.particle.temp -273.15f;
			}
		}
		else
		{
			sampleInfo << "Empty, Pressure: " << std::fixed << sample.AirPressure;
		}

		int textWidth = Graphics::textwidth((char*)sampleInfo.str().c_str());
		g->fillrect(XRES-20-textWidth, 12, textWidth+8, 15, 0, 0, 0, 255*0.5);
		g->drawtext(XRES-16-textWidth, 16, (const char*)sampleInfo.str().c_str(), 255, 255, 255, 255*0.75);

		if(showDebug)
		{
			sampleInfo.str(std::string());

			if(sample.particle.type)
			{
				sampleInfo << "#" << sample.ParticleID << ", ";
			}
			sampleInfo << "X:" << sample.PositionX << " Y:" << sample.PositionY;

			textWidth = Graphics::textwidth((char*)sampleInfo.str().c_str());
			g->fillrect(XRES-20-textWidth, 26, textWidth+8, 15, 0, 0, 0, 255*0.5);
			g->drawtext(XRES-16-textWidth, 30, (const char*)sampleInfo.str().c_str(), 255, 255, 255, 255*0.75);
		}


		//FPS and some version info
#ifndef DEBUG //In debug mode, the Engine will draw FPS and other info instead
		std::stringstream fpsInfo;
		fpsInfo.precision(2);
#ifdef SNAPSHOT
		fpsInfo << "Snapshot " << SNAPSHOT_ID << ". ";
#endif
		fpsInfo << "FPS: " << std::fixed << ui::Engine::Ref().GetFps();

		textWidth = Graphics::textwidth((char*)fpsInfo.str().c_str());
		g->fillrect(12, 12, textWidth+8, 15, 0, 0, 0, 255*0.5);
		g->drawtext(16, 16, (const char*)fpsInfo.str().c_str(), 32, 216, 255, 255*0.75);
#endif
	}

	//Tooltips
	if(infoTipPresence)
	{
		int infoTipAlpha = (infoTipPresence>50?50:infoTipPresence)*5;
		g->drawtext((XRES-Graphics::textwidth((char*)infoTip.c_str()))/2, (YRES/2)-2, (char*)infoTip.c_str(), 255, 255, 255, infoTipAlpha);
	}

	if(toolTipPosition.X!=-1 && toolTipPosition.Y!=-1 && toolTip.length())
	{
		g->drawtext(toolTipPosition.X, toolTipPosition.Y, (char*)toolTip.c_str(), 255, 255, 255, 255);
	}

	//Introduction text
	if(introText)
	{
		g->fillrect(0, 0, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, introText>51?102:introText*2);
		g->drawtext(16, 20, (char*)introTextMessage.c_str(), 255, 255, 255, introText>51?255:introText*5);
	}
}

ui::Point GameView::lineSnapCoords(ui::Point point1, ui::Point point2)
{
	ui::Point newPoint(0, 0);
	float snapAngle = floor(atan2(point2.Y-point1.Y, point2.X-point1.X)/(M_PI*0.25)+0.5)*M_PI*0.25;
	float lineMag = sqrtf(pow((float)(point2.X-point1.X),2)+pow((float)(point2.Y-point1.Y),2));
	newPoint.X = (int)(lineMag*cos(snapAngle)+point1.X+0.5f);
	newPoint.Y = (int)(lineMag*sin(snapAngle)+point1.Y+0.5f);
	return newPoint;
}

ui::Point GameView::rectSnapCoords(ui::Point point1, ui::Point point2)
{
	ui::Point newPoint(0, 0);
	float snapAngle = floor((atan2(point2.Y-point1.Y, point2.X-point1.X)+M_PI*0.25)/(M_PI*0.5)+0.5)*M_PI*0.5 - M_PI*0.25;
	float lineMag = sqrtf(pow((float)(point2.X-point1.X),2)+pow((float)(point2.Y-point1.Y),2));
	newPoint.X = (int)(lineMag*cos(snapAngle)+point1.X+0.5f);
	newPoint.Y = (int)(lineMag*sin(snapAngle)+point1.Y+0.5f);
	return newPoint;
}
