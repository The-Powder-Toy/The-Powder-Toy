#include <sstream>
#include <iomanip>

#include "Config.h"
#include "gui/Style.h"
#include "GameView.h"
#include "graphics/Graphics.h"
#include "gui/interface/Window.h"
#include "gui/interface/Button.h"
#include "gui/interface/Colour.h"
#include "gui/interface/Keys.h"
#include "gui/interface/Slider.h"
#include "gui/search/Thumbnail.h"
#include "simulation/SaveRenderer.h"
#include "simulation/SimulationData.h"
#include "gui/dialogues/ConfirmPrompt.h"
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
	void SetRightToolTip(std::string tooltip) { toolTip2 = tooltip; }
	bool GetShowSplit() { return showSplit; }
	void SetShowSplit(bool split) { showSplit = split; }
	SplitButtonAction * GetSplitActionCallback() { return splitActionCallback; }
	void SetSplitActionCallback(SplitButtonAction * newAction) { splitActionCallback = newAction; }
	void SetToolTip(int x, int y)
	{
		if(x >= splitPosition || !showSplit)
		{
			if(toolTip2.length()>0 && GetParentWindow())
			{
				GetParentWindow()->ToolTip(this, ui::Point(x, y), toolTip2);
			}
		}
		else if(x < splitPosition)
		{
			if(toolTip.length()>0 && GetParentWindow())
			{
				GetParentWindow()->ToolTip(this, ui::Point(x, y), toolTip);
			}
		}
	}
	virtual void OnMouseUnclick(int x, int y, unsigned int button)
	{
		if(isButtonDown)
		{
			if(leftDown)
				DoLeftAction();
			else if(rightDown)
				DoRightAction();
		}
		ui::Button::OnMouseUnclick(x, y, button);

	}
	virtual void OnMouseHover(int x, int y, int dx, int dy)
	{
		SetToolTip(x, y);
	}
	virtual void OnMouseHover(int x, int y)
	{
		SetToolTip(x, y);
	}
	virtual void OnMouseEnter(int x, int y)
	{
		isMouseInside = true;
		if(!Enabled)
			return;
		SetToolTip(x, y);
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
	pointQueue(queue<ui::Point>()),
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
	buttonTipShow(0),
	isToolTipFadingIn(false),
	isButtonTipFadingIn(false),
	toolTipPosition(-1, -1),
	shiftBehaviour(false),
	ctrlBehaviour(false),
	altBehaviour(false),
	showHud(true),
	showDebug(false),
	introText(2048),
	introTextMessage(introTextData),
	wallBrush(false),
	toolBrush(false),
	windTool(false),
	doScreenshot(false),
	recording(false),
	screenshotIndex(0),
	recordingIndex(0),
	toolTipPresence(0),
	currentSaveType(0),
	lastLogEntry(0.0f),
	lastMenu(-1)
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
	scrollBar->Appearance.BorderHover = ui::Colour(200, 200, 200);
	scrollBar->Appearance.BorderActive = ui::Colour(200, 200, 200);
	scrollBar->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	scrollBar->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(scrollBar);

	searchButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(17, 15), "", "Find & open a simulation");  //Open
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
		void AltActionCallback(ui::Button * sender)
		{
			v->c->OpenSavePreview();
		}
	};
	reloadButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(17, 15), "", "Reload the simulation");
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
				v->c->OpenLocalSaveWindow(false);
			else
				v->c->OpenSaveWindow();
		}
		void ActionCallbackLeft(ui::Button * sender)
		{
			if(v->CtrlBehaviour())
				v->c->OpenLocalSaveWindow(true);
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
	upVoteButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(15, 15), "", "Like this save");
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
	downVoteButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(15, 15), "", "Dislike this save");
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
	tagSimulationButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(251, 15), "[no tags set]", "Add simulation tags");
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
	clearSimButton = new ui::Button(ui::Point(Size.X-159, Size.Y-16), ui::Point(17, 15), "", "Erase everything");
	clearSimButton->SetIcon(IconNew);
	clearSimButton->Appearance.Margin.Left+=2;
	clearSimButton->SetActionCallback(new ClearSimAction(this));
	AddComponent(clearSimButton);

	class LoginAction : public SplitButtonAction
	{
		GameView * v;
	public:
		LoginAction(GameView * _v) { v = _v; }
		void ActionCallbackLeft(ui::Button * sender)
		{
			v->c->OpenLogin();
		}
		void ActionCallbackRight(ui::Button * sender)
		{
			v->c->OpenProfile();
		}
	};
	loginButton = new SplitButton(ui::Point(Size.X-141, Size.Y-16), ui::Point(92, 15), "[sign in]", "Sign into simulation server", "Edit Profile", 19);
	loginButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	loginButton->SetIcon(IconLogin);
	((SplitButton*)loginButton)->SetSplitActionCallback(new LoginAction(this));
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
	simulationOptionButton = new ui::Button(ui::Point(Size.X-48, Size.Y-16), ui::Point(15, 15), "", "Simulation options");
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
	displayModeButton = new ui::Button(ui::Point(Size.X-32, Size.Y-16), ui::Point(15, 15), "", "Renderer options");
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
	pauseButton = new ui::Button(ui::Point(Size.X-16, Size.Y-16), ui::Point(15, 15), "", "Pause/Resume the simulation");  //Pause
	pauseButton->SetIcon(IconPause);
	pauseButton->SetTogglable(true); 
	pauseButton->SetActionCallback(new PauseAction(this));
	AddComponent(pauseButton);

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
	ui::Button * tempButton = new ui::Button(ui::Point(XRES+BARSIZE-16, YRES+MENUSIZE-32), ui::Point(15, 15), "\xE5", "Search for elements");
	tempButton->Appearance.Margin = ui::Border(0, 2, 3, 2);
	tempButton->SetActionCallback(new ElementSearchAction(this));
	AddComponent(tempButton);

	class ColourPickerAction : public ui::ButtonAction
	{
		GameView * v;
	public:
		ColourPickerAction(GameView * _v) { v = _v; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->OpenColourPicker();
		}
	};
	colourPicker = new ui::Button(ui::Point((XRES/2)-8, YRES+1), ui::Point(16, 16), "", "Pick Colour");
	colourPicker->SetActionCallback(new ColourPickerAction(this));
}

GameView::~GameView()
{
	if(!colourPicker->GetParentWindow())
		delete colourPicker;

	for(std::vector<ToolButton*>::iterator iter = colourPresets.begin(), end = colourPresets.end(); iter != end; ++iter)
	{
		ToolButton * button = *iter;
		if(!button->GetParentWindow())
		{
			delete button;
		}

	}

	if(placeSaveThumb)
		delete placeSaveThumb;
}

class GameView::MenuAction: public ui::ButtonAction
{
	GameView * v;
public:
	int menuID;
	bool needsClick;
	MenuAction(GameView * _v, int menuID_)
	{ 
		v = _v;
		menuID = menuID_; 
		if (menuID == SC_DECO)
			needsClick = true;
		else
			needsClick = false;
	}
	void MouseEnterCallback(ui::Button * sender)
	{
		if(!needsClick && !ui::Engine::Ref().GetMouseButton())
			v->c->SetActiveMenu(menuID);
	}
	void ActionCallback(ui::Button * sender)
	{
		if (needsClick)
			v->c->SetActiveMenu(menuID);
		else
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
	for (int i = menuList.size()-1; i >= 0; i--)
	{
		std::string tempString = "";
		tempString += menuList[i]->GetIcon();
		ui::Button * tempButton = new ui::Button(ui::Point(XRES+BARSIZE-16, currentY), ui::Point(15, 15), tempString, menuList[i]->GetDescription());
		tempButton->Appearance.Margin = ui::Border(0, 2, 3, 2);
		tempButton->SetTogglable(true);
		tempButton->SetActionCallback(new MenuAction(this, i));
		currentY-=16;
		AddComponent(tempButton);
		menuButtons.push_back(tempButton);
	}
}

void GameView::SetSample(SimulationSample sample)
{
	this->sample = sample;
}

void GameView::SetHudEnable(bool hudState)
{
	showHud = hudState;
}

bool GameView::GetHudEnable()
{
	return showHud;
}

void GameView::SetDebugHUD(bool mode)
{
	showDebug = mode;
	if (ren)
		ren->debugLines = showDebug;
}

bool GameView::GetDebugHUD()
{
	return showDebug;
}

ui::Point GameView::GetMousePosition()
{
	return currentMouse;
}

void GameView::NotifyActiveToolsChanged(GameModel * sender)
{
	for(int i = 0; i < toolButtons.size(); i++)
	{
		Tool * tool = ((ToolAction*)toolButtons[i]->GetActionCallback())->tool;
		if(sender->GetActiveTool(0) == tool)
		{
			toolButtons[i]->SetSelectionState(0);	//Primary
			if (tool->GetIdentifier().find("DEFAULT_UI_WIND") != tool->GetIdentifier().npos)
				windTool = true;
			else
				windTool = false;
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
	//need to do this for all tools every time just in case it wasn't caught if you weren't in the menu a tool was changed to
	c->ActiveToolChanged(0, sender->GetActiveTool(0));
	c->ActiveToolChanged(1, sender->GetActiveTool(1));
	c->ActiveToolChanged(2, sender->GetActiveTool(2));
}

void GameView::NotifyLastToolChanged(GameModel * sender)
{
	if(sender->GetLastTool() && sender->GetLastTool()->GetResolution() == CELL)
		wallBrush = true;
	else
		wallBrush = false;

	if (sender->GetLastTool() && sender->GetLastTool()->GetIdentifier().find("DEFAULT_TOOL_") != sender->GetLastTool()->GetIdentifier().npos)
		toolBrush = true;
	else
		toolBrush = false;
}

void GameView::NotifyToolListChanged(GameModel * sender)
{
	//int currentY = YRES+MENUSIZE-36;
	lastOffset = 0;
	int currentX = XRES+BARSIZE-56;
	int totalColour;
	for(int i = 0; i < menuButtons.size(); i++)
	{
		if(((MenuAction*)menuButtons[i]->GetActionCallback())->menuID==sender->GetActiveMenu())
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
	if (sender->GetActiveMenu() != SC_DECO)
		lastMenu = sender->GetActiveMenu();
}

void GameView::NotifyColourSelectorVisibilityChanged(GameModel * sender)
{
	for(std::vector<ToolButton*>::iterator iter = colourPresets.begin(), end = colourPresets.end(); iter != end; ++iter)
	{
		ToolButton * button = *iter;
		RemoveComponent(button);
		button->SetParentWindow(NULL);
	}

	RemoveComponent(colourPicker);
	colourPicker->SetParentWindow(NULL);

	if(sender->GetColourSelectorVisibility())
	{
		for(std::vector<ToolButton*>::iterator iter = colourPresets.begin(), end = colourPresets.end(); iter != end; ++iter)
		{
			ToolButton * button = *iter;
			AddComponent(button);
		}
		AddComponent(colourPicker);
	}
}

void GameView::NotifyColourPresetsChanged(GameModel * sender)
{
	class ColourPresetAction: public ui::ButtonAction
	{
		GameView * v;
	public:
		int preset;
		ColourPresetAction(GameView * _v, int preset) : preset(preset) { v = _v; }
		void ActionCallback(ui::Button * sender_)
		{
			ToolButton *sender = (ToolButton*)sender_;
			if(sender->GetSelectionState() == 0)
			{
				v->c->SetActiveColourPreset(preset);
				v->c->SetColour(sender->Appearance.BackgroundInactive);
			}
			else
				sender->SetSelectionState(0);
		}
	};


	for(std::vector<ToolButton*>::iterator iter = colourPresets.begin(), end = colourPresets.end(); iter != end; ++iter)
	{
		ToolButton * button = *iter;
		RemoveComponent(button);
		delete button;
	}
	colourPresets.clear();

	int currentX = 5;
	std::vector<ui::Colour> colours = sender->GetColourPresets();
	int i = 0;
	for(std::vector<ui::Colour>::iterator iter = colours.begin(), end = colours.end(); iter != end; ++iter)
	{
		ToolButton * tempButton = new ToolButton(ui::Point(currentX, YRES+1), ui::Point(30, 18), "");
		tempButton->Appearance.BackgroundInactive = *iter;
		tempButton->SetActionCallback(new ColourPresetAction(this, i));

		currentX += 31;

		if(sender->GetColourSelectorVisibility())
			AddComponent(tempButton);
		colourPresets.push_back(tempButton);

		i++;
	}
	NotifyColourActivePresetChanged(sender);
}

void GameView::NotifyColourActivePresetChanged(GameModel * sender)
{
	for(int i = 0; i < colourPresets.size(); i++)
	{
		if(sender->GetActiveColourPreset() == i)
		{
			colourPresets[i]->SetSelectionState(0);	//Primary
		}
		else
		{
			colourPresets[i]->SetSelectionState(-1);
		}
	}
}

void GameView::NotifyColourSelectorColourChanged(GameModel * sender)
{
	colourPicker->Appearance.BackgroundInactive = sender->GetColourSelectorColour();
	colourPicker->Appearance.BackgroundHover = sender->GetColourSelectorColour();
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
		((SplitButton*)loginButton)->SetShowSplit(false);
		((SplitButton*)loginButton)->SetRightToolTip("Sign in to simulation server");
	}
	else
	{
		loginButton->SetText(sender->GetUser().Username);
		((SplitButton*)loginButton)->SetShowSplit(true);
		((SplitButton*)loginButton)->SetRightToolTip("Edit profile");
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
			upVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 108, 10, 255));
		else
			upVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));

		downVoteButton->Enabled = upVoteButton->Enabled;
		if(sender->GetSave()->GetID() && sender->GetUser().ID && sender->GetSave()->GetVote()==-1)
			downVoteButton->Appearance.BackgroundDisabled = (ui::Colour(108, 0, 10, 255));
		else
			downVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));

		if (sender->GetUser().ID)
		{
			upVoteButton->Appearance.BorderDisabled = upVoteButton->Appearance.BorderInactive;
			downVoteButton->Appearance.BorderDisabled = downVoteButton->Appearance.BorderInactive;
		}
		else
		{
			upVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100);
			downVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100);
		}

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
		currentSaveType = 1;
	}
	else if (sender->GetSaveFile())
	{
		if (ctrlBehaviour)
			((SplitButton*)saveSimulationButton)->SetShowSplit(true);
		else
			((SplitButton*)saveSimulationButton)->SetShowSplit(false);
		saveSimulationButton->SetText(sender->GetSaveFile()->GetDisplayName());
		reloadButton->Enabled = true;
		upVoteButton->Enabled = false;
		upVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));
		upVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100);
		downVoteButton->Enabled = false;
		upVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));
		downVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100);
		tagSimulationButton->Enabled = false;
		tagSimulationButton->SetText("[no tags set]");
		currentSaveType = 2;
	}
	else
	{
		((SplitButton*)saveSimulationButton)->SetShowSplit(false);
		saveSimulationButton->SetText("[untitled simulation]");
		reloadButton->Enabled = false;
		upVoteButton->Enabled = false;
		upVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));
		upVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100),
		downVoteButton->Enabled = false;
		downVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));
		downVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100),
		tagSimulationButton->Enabled = false;
		tagSimulationButton->SetText("[no tags set]");
		currentSaveType = 0;
	}
	c->HistorySnapshot();
}

void GameView::NotifyBrushChanged(GameModel * sender)
{
	activeBrush = sender->GetBrush();
}

void GameView::screenshot()
{
	doScreenshot = true;
}

void GameView::record()
{
	if(recording)
	{
		recording = false;
	}
	else
	{
		class RecordingConfirmation: public ConfirmDialogueCallback {
		public:
			GameView * v;
			RecordingConfirmation(GameView * v): v(v) {}
			virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
				if (result == ConfirmPrompt::ResultOkay)
				{
					v->recording = true;
				}
			}
			virtual ~RecordingConfirmation() { }
		};
		new ConfirmPrompt("Recording", "You're about to start recording all drawn frames. This may use a load of hard disk space.", new RecordingConfirmation(this));
	}
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
			selectPoint1 = c->PointTranslate(ui::Point(x, y));
		if(selectPoint1.X!=-1)
			selectPoint2 = c->PointTranslate(ui::Point(x, y));
		return;
	}
	currentMouse = ui::Point(x, y);
	if(isMouseDown && drawMode == DrawPoints)
	{
		pointQueue.push(ui::Point(c->PointTranslate(ui::Point(x-dx, y-dy))));
		pointQueue.push(ui::Point(c->PointTranslate(ui::Point(x, y))));
	}
}

void GameView::OnMouseDown(int x, int y, unsigned button)
{
	if(altBehaviour && !shiftBehaviour && !ctrlBehaviour)
		button = BUTTON_MIDDLE;
	if(selectMode!=SelectNone)
	{
		if(button==BUTTON_LEFT)
		{
			selectPoint1 = c->PointTranslate(ui::Point(x, y));
			selectPoint2 = selectPoint1;
		}
		return;
	}
	if(currentMouse.X >= 0 && currentMouse.X < XRES && currentMouse.Y >= 0 && currentMouse.Y < YRES && !(zoomEnabled && !zoomCursorFixed))
	{
		if(button == BUTTON_LEFT)
			toolIndex = 0;
		if(button == BUTTON_RIGHT)
			toolIndex = 1;
		if(button == BUTTON_MIDDLE)
			toolIndex = 2;
		isMouseDown = true;
		if(!pointQueue.size())
			c->HistorySnapshot();
		if(drawMode == DrawRect || drawMode == DrawLine)
		{
			drawPoint1 = c->PointTranslate(ui::Point(x, y));
		}
		if(drawMode == DrawPoints)
		{
			pointQueue.push(ui::Point(c->PointTranslate(ui::Point(x, y))));
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
				if(placeSaveThumb)
				{
					int thumbX = selectPoint2.X - (placeSaveThumb->Width/2);
					int thumbY = selectPoint2.Y - (placeSaveThumb->Height/2);

					if(thumbX<0)
						thumbX = 0;
					if(thumbX+(placeSaveThumb->Width)>=XRES)
						thumbX = XRES-placeSaveThumb->Width;

					if(thumbY<0)
						thumbY = 0;
					if(thumbY+(placeSaveThumb->Height)>=YRES)
						thumbY = YRES-placeSaveThumb->Height;

					c->PlaceSave(ui::Point(thumbX, thumbY));
				}
			}
			else
			{
				int x2 = (selectPoint1.X>selectPoint2.X)?selectPoint1.X:selectPoint2.X;
				int y2 = (selectPoint1.Y>selectPoint2.Y)?selectPoint1.Y:selectPoint2.Y;
				int x1 = (selectPoint2.X<selectPoint1.X)?selectPoint2.X:selectPoint1.X;
				int y1 = (selectPoint2.Y<selectPoint1.Y)?selectPoint2.Y:selectPoint1.Y;
				if(selectMode==SelectCopy)
					c->CopyRegion(ui::Point(x1, y1), ui::Point(x2, y2));
				else if(selectMode==SelectCut)
					c->CutRegion(ui::Point(x1, y1), ui::Point(x2, y2));
				else if(selectMode==SelectStamp)
					c->StampRegion(ui::Point(x1, y1), ui::Point(x2, y2));
			}
		}
		currentMouse = ui::Point(x, y);
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
				drawPoint2 = c->PointTranslate(ui::Point(x, y));
				finalDrawPoint2 = drawPoint2;

				if(drawSnap && drawMode == DrawLine)
				{
					finalDrawPoint2 = lineSnapCoords(c->PointTranslate(drawPoint1), drawPoint2);
				}

				if(drawSnap && drawMode == DrawRect)
				{
					finalDrawPoint2 = rectSnapCoords(c->PointTranslate(drawPoint1), drawPoint2);
				}

				if(drawMode == DrawRect)
				{
					c->DrawRect(toolIndex, c->PointTranslate(drawPoint1), finalDrawPoint2);
				}
				if(drawMode == DrawLine)
				{
					c->DrawLine(toolIndex, c->PointTranslate(drawPoint1), finalDrawPoint2);
				}
			}
			if(drawMode == DrawPoints)
			{
				c->ToolClick(toolIndex, c->PointTranslate(ui::Point(x, y)));
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
	if(sender->Position.Y > Size.Y-17)
	{
		if (selectMode == PlaceSave || selectMode == SelectNone)
		{
			buttonTip = toolTip;
			isButtonTipFadingIn = true;
		}
	}
	else if(sender->Position.X > Size.X-BARSIZE)// < Size.Y-(quickOptionButtons.size()+1)*16)
	{
		this->toolTip = toolTip;
		toolTipPosition = ui::Point(Size.X-27-Graphics::textwidth((char*)toolTip.c_str()), sender->Position.Y+3);
		if(toolTipPosition.Y+10 > Size.Y-MENUSIZE)
			toolTipPosition = ui::Point(Size.X-27-Graphics::textwidth((char*)toolTip.c_str()), Size.Y-MENUSIZE-10);
		isToolTipFadingIn = true;
	}
	else
	{
		this->toolTip = toolTip;
		toolTipPosition = ui::Point(Size.X-27-Graphics::textwidth((char*)toolTip.c_str()), Size.Y-MENUSIZE-10);
		isToolTipFadingIn = true;
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
		c->AdjustBrushSize(d, false, shiftBehaviour, ctrlBehaviour);
	}
}

void GameView::BeginStampSelection()
{
	selectMode = SelectStamp;
	selectPoint1 = ui::Point(-1, -1);
	buttonTip = "\x0F\xEF\xEF\020Click-and-drag to specify an area to create a stamp (right click = cancel)";
	buttonTipShow = 120;
}

void GameView::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
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
		if(key != ' ')
			return;
	}
	switch(key)
	{
	case KEY_ALT:
		drawSnap = true;
		enableAltBehaviour();
		break;
	case KEY_CTRL:
		if(!isMouseDown)
		{
			if(drawModeReset)
				drawModeReset = false;
			else
				drawPoint1 = currentMouse;
			if(shift)
			{
				if (!toolBrush)
					drawMode = DrawFill;
				else
					drawMode = DrawPoints;
			}
			else
				drawMode = DrawRect;
		}
		enableCtrlBehaviour();
		break;
	case KEY_SHIFT:
		if(!isMouseDown)
		{
			if(drawModeReset)
				drawModeReset = false;
			else
				drawPoint1 = currentMouse;
			if(ctrl)
			{
				if (!toolBrush)
					drawMode = DrawFill;
				else
					drawMode = DrawPoints;
			}
			else
				drawMode = DrawLine;
		}
		enableShiftBehaviour();
		break;
	case ' ': //Space
		c->SetPaused();
		break;
	case KEY_TAB: //Tab
		c->ChangeBrush();
		break;
	case 'z':
		if(ctrl)
		{
			c->HistoryRestore();
		}
		else
		{
			if (drawMode != DrawLine && !windTool)
				isMouseDown = false;
			zoomCursorFixed = false;
			c->SetZoomEnabled(true);
		}
		break;
	case '`':
		c->ShowConsole();
		break;
	case 'p':
		screenshot();
		break;
	case 'r':
		if (ctrl)
			c->ReloadSim();
		else
			record();
		break;
	case 'e':
		c->OpenElementSearch();
		break;
	case 'f':
		c->FrameStep();
		break;
	case 'g':
		if (ctrl)
			c->ShowGravityGrid();
		else if(shift)
			c->AdjustGridSize(-1);
		else
			c->AdjustGridSize(1);
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
		else
			if (colourPicker->GetParentWindow())
				c->SetActiveMenu(lastMenu);
			else
			{
				c->SetDecoration(true);
				c->SetPaused(true);
				c->SetActiveMenu(SC_DECO);
			}
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
			buttonTip = "\x0F\xEF\xEF\020Click-and-drag to specify an area to copy (right click = cancel)";
			buttonTipShow = 120;
		}
		break;
	case 'x':
		if(ctrl)
		{
			selectMode = SelectCut;
			selectPoint1 = ui::Point(-1, -1);
			buttonTip = "\x0F\xEF\xEF\020Click-and-drag to specify an area to copy then cut (right click = cancel)";
			buttonTipShow = 120;
		}
		break;
	case 'v':
		if(ctrl)
		{
			c->LoadClipboard();
			selectPoint2 = mousePosition;
			selectPoint1 = selectPoint2;
		}
		break;
	case 'l':
		c->LoadStamp();
		selectPoint2 = mousePosition;
		selectPoint1 = selectPoint2;
		isMouseDown = false;
		drawMode = DrawPoints;
		break;
	case 'k':
		selectPoint2 = ui::Point(-1, -1);
		selectPoint1 = selectPoint2;
		c->OpenStamps();
		break;
	case ']':
		if(zoomEnabled && !zoomCursorFixed)
			c->AdjustZoomSize(1, !alt);
		else
			c->AdjustBrushSize(1, !alt, shiftBehaviour, ctrlBehaviour);
		break;
	case '[':
		if(zoomEnabled && !zoomCursorFixed)
			c->AdjustZoomSize(-1, !alt);
		else
			c->AdjustBrushSize(-1, !alt, shiftBehaviour, ctrlBehaviour);
		break;
	case 'i':
		if(ctrl)
			c->Install();
		else
			c->InvertAirSim();
		break;
	}

	if (shift && showDebug && key == '1')
		c->LoadRenderPreset(10);
	else if(key >= '0' && key <= '9')
	{
		c->LoadRenderPreset(key-'0');
	}
}

void GameView::OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(ctrl && shift && drawMode != DrawPoints)
		drawMode = DrawFill;
	else if (ctrl && drawMode != DrawPoints)
		drawMode = DrawRect;
	else if (shift && drawMode != DrawPoints)
		drawMode = DrawLine;
	else if(!isMouseDown)
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
		if(!ctrl)
		{
			if(!zoomCursorFixed && !alt)
				c->SetZoomEnabled(false);
		}
		break;
	}
}

void GameView::OnBlur()
{
	disableAltBehaviour();
	disableCtrlBehaviour();
	disableShiftBehaviour();
	isMouseDown = false;
	drawMode = DrawPoints;
}

void GameView::OnTick(float dt)
{
	if(selectMode==PlaceSave && !placeSaveThumb)
		selectMode = SelectNone;
	if(zoomEnabled && !zoomCursorFixed)
		c->SetZoomPosition(currentMouse);
	if(drawMode == DrawPoints)
	{
		if(isMouseDown && pointQueue.empty())
		{
			pointQueue.push(ui::Point(c->PointTranslate(currentMouse)));
		}
		if(!pointQueue.empty())
		{
			c->DrawPoints(toolIndex, pointQueue);
		}
	}
	else if(drawMode == DrawFill && isMouseDown)
	{
		c->DrawFill(toolIndex, c->PointTranslate(currentMouse));
	}
	else if (windTool && isMouseDown && drawMode == DrawLine)
	{
		c->DrawLine(toolIndex, c->PointTranslate(drawPoint1), lineSnapCoords(c->PointTranslate(drawPoint1), currentMouse));
	}
	if(introText)
	{
		introText -= int(dt)>0?((int)dt < 5? dt:5):1;
		if(introText < 0)
			introText  = 0;
	}
	if(infoTipPresence>0)
	{
		infoTipPresence -= int(dt)>0?int(dt):1;
		if(infoTipPresence<0)
			infoTipPresence = 0;
	}
	if (isButtonTipFadingIn || (selectMode != PlaceSave && selectMode != SelectNone))
	{
		isButtonTipFadingIn = false;
		if(buttonTipShow < 120)
		{
			buttonTipShow += int(dt*2)>0?int(dt*2):1;
			if(buttonTipShow>120)
				buttonTipShow = 120;
		}
	}
	else if(buttonTipShow>0)
	{
		buttonTipShow -= int(dt)>0?int(dt):1;
		if(buttonTipShow<0)
			buttonTipShow = 0;
	}
	if (isToolTipFadingIn)
	{
		isToolTipFadingIn = false;
		if(toolTipPresence < 120)
		{
			toolTipPresence += int(dt*2)>0?int(dt*2):1;
			if(toolTipPresence>120)
				toolTipPresence = 120;
		}
	}
	else if(toolTipPresence>0)
	{
		toolTipPresence -= int(dt)>0?int(dt):1;
		if(toolTipPresence<0)
			toolTipPresence = 0;
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
		int scrollSize = (int)(((float)(XRES-BARSIZE))/((float)totalWidth) * ((float)XRES-BARSIZE));
		if (scrollSize>XRES-1)
			scrollSize = XRES-1;
		if(totalWidth > XRES-15)
		{
			int mouseX = x;
			if(mouseX > XRES)
				mouseX = XRES;
			if (mouseX < 15)
				mouseX = 15;

			scrollBar->Position.X = (int)(((float)mouseX/((float)XRES))*(float)(XRES-scrollSize));

			float overflow = totalWidth-(XRES-BARSIZE), mouseLocation = float(XRES)/float(mouseX-(XRES));
			setToolButtonOffset(overflow/mouseLocation);

			//Ensure that mouseLeave events are make their way to the buttons should they move from underneath the mouse pointer
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
			scrollBar->Position.X = 1;
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
		void AltActionCallback(ui::Button * sender)
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
		tempButton->Appearance.BorderHover = ui::Colour(255, 175, 0);
		tempButton->Appearance.TextHover = ui::Colour(255, 175, 0);
		AddComponent(tempButton);
		notificationComponents.push_back(tempButton);

		tempButton = new ui::Button(ui::Point(XRES-20, currentY), ui::Point(15, 15));
		tempButton->SetIcon(IconClose);
		tempButton->SetActionCallback(new CloseNotificationButtonAction(this, *iter));
		tempButton->Appearance.Margin.Left+=2;
		tempButton->Appearance.Margin.Top+=2;
		tempButton->Appearance.BorderInactive = style::Colour::WarningTitle;
		tempButton->Appearance.TextInactive = style::Colour::WarningTitle;
		tempButton->Appearance.BorderHover = ui::Colour(255, 175, 0);
		tempButton->Appearance.TextHover = ui::Colour(255, 175, 0);
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
		selectPoint2 = mousePosition;
	}
	else
	{
		placeSaveThumb = NULL;
		selectMode = SelectNone;
	}
}

void GameView::enableShiftBehaviour()
{
	if(!shiftBehaviour)
	{
		shiftBehaviour = true;
		if(isMouseDown || (toolBrush && drawMode == DrawPoints))
			c->SetToolStrength(10.0f);
	}
}

void GameView::disableShiftBehaviour()
{
	if(shiftBehaviour)
	{
		shiftBehaviour = false;
		if(!ctrlBehaviour)
			c->SetToolStrength(1.0f);
		else
			c->SetToolStrength(.1f);
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
		saveSimulationButton->Appearance.BackgroundInactive = saveSimulationButton->Appearance.BackgroundHover = ui::Colour(255, 255, 255);
		saveSimulationButton->Appearance.TextInactive = saveSimulationButton->Appearance.TextHover = ui::Colour(0, 0, 0);
		searchButton->Appearance.BackgroundInactive = searchButton->Appearance.BackgroundHover = ui::Colour(255, 255, 255);
		searchButton->Appearance.TextInactive = searchButton->Appearance.TextHover = ui::Colour(0, 0, 0);
		if (currentSaveType == 2)
			((SplitButton*)saveSimulationButton)->SetShowSplit(true);
		if(isMouseDown || (toolBrush && drawMode == DrawPoints))
		{
			if(!shiftBehaviour)
				c->SetToolStrength(.1f);
			else
				c->SetToolStrength(10.0f);
		}
	}
}

void GameView::disableCtrlBehaviour()
{
	if(ctrlBehaviour)
	{
		ctrlBehaviour = false;

		//Hide HDD save & load buttons
		saveSimulationButton->Appearance.BackgroundInactive = ui::Colour(0, 0, 0);
		saveSimulationButton->Appearance.BackgroundHover = ui::Colour(20, 20, 20);
		saveSimulationButton->Appearance.TextInactive = saveSimulationButton->Appearance.TextHover = ui::Colour(255, 255, 255);
		searchButton->Appearance.BackgroundInactive = ui::Colour(0, 0, 0);
		searchButton->Appearance.BackgroundHover = ui::Colour(20, 20, 20);
		searchButton->Appearance.TextInactive = searchButton->Appearance.TextHover = ui::Colour(255, 255, 255);
		if (currentSaveType == 2)
			((SplitButton*)saveSimulationButton)->SetShowSplit(false);
		if(!shiftBehaviour)
			c->SetToolStrength(1.0f);
		else
			c->SetToolStrength(10.0f);
	}
}

void GameView::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	if(ren)
	{
		ren->clearScreen(1.0f);
		ren->RenderBegin();
		ren->SetSample(c->PointTranslate(currentMouse).X, c->PointTranslate(currentMouse).Y);
		if(selectMode == SelectNone && (!zoomEnabled || zoomCursorFixed) && activeBrush && currentMouse.X >= 0 && currentMouse.X < XRES && currentMouse.Y >= 0 && currentMouse.Y < YRES)
		{
			ui::Point finalCurrentMouse = c->PointTranslate(currentMouse);
			ui::Point initialDrawPoint = drawPoint1;

			if(wallBrush)
			{
				finalCurrentMouse = c->NormaliseBlockCoord(finalCurrentMouse);
				initialDrawPoint = c->NormaliseBlockCoord(initialDrawPoint);
			}

			if(drawMode==DrawRect && isMouseDown)
			{
				if(drawSnap)
				{
					finalCurrentMouse = rectSnapCoords(c->PointTranslate(initialDrawPoint), finalCurrentMouse);
				}
				activeBrush->RenderRect(ren, c->PointTranslate(initialDrawPoint), finalCurrentMouse);
			}
			else if(drawMode==DrawLine && isMouseDown)
			{
				if(drawSnap)
				{
					finalCurrentMouse = lineSnapCoords(c->PointTranslate(initialDrawPoint), finalCurrentMouse);
				}
				activeBrush->RenderLine(ren, c->PointTranslate(initialDrawPoint), finalCurrentMouse);
			}
			else if(drawMode==DrawFill)// || altBehaviour)
			{
				activeBrush->RenderFill(ren, finalCurrentMouse);
			}
			if(drawMode == DrawPoints || drawMode==DrawLine || (drawMode == DrawRect && !isMouseDown))
			{
				if(wallBrush)
				{
					ui::Point finalBrushRadius = c->NormaliseBlockCoord(activeBrush->GetRadius());
					ren->xor_line(finalCurrentMouse.X-finalBrushRadius.X, finalCurrentMouse.Y-finalBrushRadius.Y, finalCurrentMouse.X+finalBrushRadius.X+CELL-1, finalCurrentMouse.Y-finalBrushRadius.Y);
					ren->xor_line(finalCurrentMouse.X-finalBrushRadius.X, finalCurrentMouse.Y+finalBrushRadius.Y+CELL-1, finalCurrentMouse.X+finalBrushRadius.X+CELL-1, finalCurrentMouse.Y+finalBrushRadius.Y+CELL-1);

					ren->xor_line(finalCurrentMouse.X-finalBrushRadius.X, finalCurrentMouse.Y-finalBrushRadius.Y+1, finalCurrentMouse.X-finalBrushRadius.X, finalCurrentMouse.Y+finalBrushRadius.Y+CELL-2);
					ren->xor_line(finalCurrentMouse.X+finalBrushRadius.X+CELL-1, finalCurrentMouse.Y-finalBrushRadius.Y+1, finalCurrentMouse.X+finalBrushRadius.X+CELL-1, finalCurrentMouse.Y+finalBrushRadius.Y+CELL-2);
				}
				else
				{
					activeBrush->RenderPoint(ren, finalCurrentMouse);
				}
			}
		}

		if(selectMode!=SelectNone)
		{
			if(selectMode==PlaceSave)
			{
				if(placeSaveThumb && selectPoint2.X!=-1)
				{
					int thumbX = selectPoint2.X - (placeSaveThumb->Width/2);
					int thumbY = selectPoint2.Y - (placeSaveThumb->Height/2);

					ui::Point thumbPos = c->NormaliseBlockCoord(ui::Point(thumbX, thumbY));

					if(thumbPos.X<0)
						thumbPos.X = 0;
					if(thumbPos.X+(placeSaveThumb->Width)>=XRES)
						thumbPos.X = XRES-placeSaveThumb->Width;

					if(thumbPos.Y<0)
						thumbPos.Y = 0;
					if(thumbPos.Y+(placeSaveThumb->Height)>=YRES)
						thumbPos.Y = YRES-placeSaveThumb->Height;

					ren->draw_image(placeSaveThumb, thumbPos.X, thumbPos.Y, 128);

					ren->xor_rect(thumbPos.X, thumbPos.Y, placeSaveThumb->Width, placeSaveThumb->Height);
				}
			}
			else
			{
				if(selectPoint1.X==-1)
				{
					ren->fillrect(0, 0, XRES, YRES, 0, 0, 0, 100);
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

					ren->fillrect(0, 0, XRES, y1, 0, 0, 0, 100);
					ren->fillrect(0, y2+1, XRES, YRES-y2-1, 0, 0, 0, 100);

					ren->fillrect(0, y1, x1, (y2-y1)+1, 0, 0, 0, 100);
					ren->fillrect(x2+1, y1, XRES-x2-1, (y2-y1)+1, 0, 0, 0, 100);

					ren->xor_rect(x1, y1, (x2-x1)+1, (y2-y1)+1);
				}
			}
		}

		ren->RenderEnd();

		if(doScreenshot)
		{
			VideoBuffer screenshot(ren->DumpFrame());
			std::vector<char> data = format::VideoBufferToPNG(screenshot);

			std::stringstream filename;
			filename << "screenshot_";
			filename << std::setfill('0') << std::setw(6) << (screenshotIndex++);
			filename << ".png";

			Client::Ref().WriteFile(data, filename.str());
			doScreenshot = false;
		}

		if(recording)
		{
			VideoBuffer screenshot(ren->DumpFrame());
			std::vector<char> data = format::VideoBufferToPPM(screenshot);

			std::stringstream filename;
			filename << "frame_";
			filename << std::setfill('0') << std::setw(6) << (recordingIndex++);
			filename << ".ppm";

			Client::Ref().WriteFile(data, filename.str());
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
				startY -= 14;
				g->fillrect(startX-3, startY-3, Graphics::textwidth((char*)message.c_str())+6, 14, 0, 0, 0, 100);
				g->drawtext(startX, startY, message.c_str(), 255, 255, 255, startAlpha);
				startAlpha-=14;
			}
		}
	}

	if(recording)
	{
		std::stringstream sampleInfo;
		sampleInfo << recordingIndex;
		sampleInfo << ". \x8E REC";

		int textWidth = Graphics::textwidth((char*)sampleInfo.str().c_str());
		g->fillrect(XRES-20-textWidth, 12, textWidth+8, 15, 0, 0, 0, 255*0.5);
		g->drawtext(XRES-16-textWidth, 16, (const char*)sampleInfo.str().c_str(), 255, 50, 20, 255);
	}
	else if(showHud)
	{
		//Draw info about simulation under cursor
		int wavelengthGfx = 0;
		std::stringstream sampleInfo;
		sampleInfo.precision(2);
		if(sample.particle.type)
		{
			int ctype = sample.particle.ctype;
			if (sample.particle.type == PT_PIPE || sample.particle.type == PT_PPIP)
				ctype = sample.particle.tmp&0xFF;
			if(showDebug)
			{
				if (sample.particle.type == PT_LAVA && ctype > 0 && ctype < PT_NUM)
					sampleInfo << "Molten " << c->ElementResolve(ctype, -1);
				else if ((sample.particle.type == PT_PIPE || sample.particle.type == PT_PPIP) && ctype > 0 && ctype < PT_NUM)
					sampleInfo << c->ElementResolve(sample.particle.type, -1) << " with " << c->ElementResolve(ctype, (int)sample.particle.pavg[1]);
				else if (sample.particle.type == PT_LIFE)
					sampleInfo << c->ElementResolve(sample.particle.type, sample.particle.ctype);
				else
				{
					sampleInfo << c->ElementResolve(sample.particle.type, sample.particle.ctype);
					if(ctype > 0 && ctype < PT_NUM)
						sampleInfo << " (" << c->ElementResolve(ctype, -1) << ")";
					else
						sampleInfo << " ()";
				}
				sampleInfo << ", Temp: " << std::fixed << sample.particle.temp -273.15f;
				sampleInfo << ", Life: " << sample.particle.life;
				sampleInfo << ", Tmp: " << sample.particle.tmp;
				sampleInfo << ", Pressure: " << std::fixed << sample.AirPressure;
			}
			else
			{
				if (sample.particle.type == PT_LAVA && sample.particle.ctype > 0 && sample.particle.ctype < PT_NUM)
					sampleInfo << "Molten " << c->ElementResolve(sample.particle.ctype, -1);
				else if ((sample.particle.type == PT_PIPE || sample.particle.type == PT_PPIP) && ctype > 0 && ctype < PT_NUM)
					sampleInfo << c->ElementResolve(sample.particle.type, -1) << " with " << c->ElementResolve(ctype, (int)sample.particle.pavg[1]);
				else if (sample.particle.type == PT_LIFE)
					sampleInfo << c->ElementResolve(sample.particle.type, sample.particle.ctype);
				else
					sampleInfo << c->ElementResolve(sample.particle.type, sample.particle.ctype);
				sampleInfo << ", Temp: " << std::fixed << sample.particle.temp -273.15f;
				sampleInfo << ", Pressure: " << std::fixed << sample.AirPressure;
			}
			if(sample.particle.type == PT_PHOT)
				wavelengthGfx = sample.particle.ctype;
		}
		else if (sample.WallType)
		{
			sampleInfo << c->WallName(sample.WallType);
			sampleInfo << ", Pressure: " << std::fixed << sample.AirPressure;
		}
		else if (sample.isMouseInSim)
		{
			sampleInfo << "Empty, Pressure: " << std::fixed << sample.AirPressure;
		}
		else
		{
			sampleInfo << "Empty";
		}

		int textWidth = Graphics::textwidth((char*)sampleInfo.str().c_str());
		g->fillrect(XRES-20-textWidth, 12, textWidth+8, 15, 0, 0, 0, 255*0.5);
		g->drawtext(XRES-16-textWidth, 16, (const char*)sampleInfo.str().c_str(), 255, 255, 255, 255*0.75);

#ifndef OGLI
		if(wavelengthGfx)
		{
			int i, cr, cg, cb, j, h = 3, x = XRES-19-textWidth, y = 10;
			int tmp;
			g->fillrect(x, y, 30, h, 64, 64, 64, 255); // coords -1 size +1 to work around bug in fillrect - TODO: fix fillrect
			for (i = 0; i < 30; i++)
			{
				if ((wavelengthGfx >> i)&1)
				{
					// Need a spread of wavelengths to get a smooth spectrum, 5 bits seems to work reasonably well
					if (i>2) tmp = 0x1F << (i-2);
					else tmp = 0x1F >> (2-i);

					cg = 0;
					cb = 0;
					cr = 0;

					for (j=0; j<12; j++)
					{
						cr += (tmp >> (j+18)) & 1;
						cb += (tmp >> j) & 1;
					}
					for (j=0; j<13; j++)
						cg += (tmp >> (j+9)) & 1;

					tmp = 624/(cr+cg+cb+1);
					cr *= tmp;
					cg *= tmp;
					cb *= tmp;
					for (j=0; j<h; j++)
						g->blendpixel(x+29-i,y+j,cr>255?255:cr,cg>255?255:cg,cb>255?255:cb,255);
				}
			}
		}
#endif

		if(showDebug)
		{
			sampleInfo.str(std::string());

			if(sample.particle.type)
			{
				sampleInfo << "#" << sample.ParticleID << ", ";
			}
			sampleInfo << "X:" << sample.PositionX << " Y:" << sample.PositionY;
			if (sample.Gravity)
				sampleInfo << " GX: " << sample.GravityVelocityX << " GY: " << sample.GravityVelocityY;

			textWidth = Graphics::textwidth((char*)sampleInfo.str().c_str());
			g->fillrect(XRES-20-textWidth, 26, textWidth+8, 15, 0, 0, 0, 255*0.5);
			g->drawtext(XRES-16-textWidth, 30, (const char*)sampleInfo.str().c_str(), 255, 255, 255, 255*0.75);
		}
	}

	if(showHud && introText < 51)
	{
		//FPS and some version info
#ifndef DEBUG //In debug mode, the Engine will draw FPS and other info instead
		std::stringstream fpsInfo;
		fpsInfo.precision(2);
#ifdef SNAPSHOT
		fpsInfo << "Snapshot " << SNAPSHOT_ID << ", ";
#elif defined(BETA)
		fpsInfo << "Beta " << SAVE_VERSION << "." << MINOR_VERSION << "." << BUILD_NUM << ", ";
#endif
		fpsInfo << "FPS: " << std::fixed << ui::Engine::Ref().GetFps();

		if (showDebug)
			fpsInfo << " Parts: " << sample.NumParts;
		if (ren->GetGridSize())
			fpsInfo << " [GRID: " << ren->GetGridSize() << "]";

		int textWidth = Graphics::textwidth((char*)fpsInfo.str().c_str());
		int alpha = 255-introText*5;
		g->fillrect(12, 12, textWidth+8, 15, 0, 0, 0, alpha*0.5);
		g->drawtext(16, 16, (const char*)fpsInfo.str().c_str(), 32, 216, 255, alpha*0.75);
#endif
	}

	//Tooltips
	if(infoTipPresence)
	{
		int infoTipAlpha = (infoTipPresence>50?50:infoTipPresence)*5;
		g->drawtext_outline((XRES-Graphics::textwidth((char*)infoTip.c_str()))/2, (YRES/2)-2, (char*)infoTip.c_str(), 255, 255, 255, infoTipAlpha);
	}

	if(toolTipPresence && toolTipPosition.X!=-1 && toolTipPosition.Y!=-1 && toolTip.length())
	{
		g->drawtext(toolTipPosition.X, toolTipPosition.Y, (char*)toolTip.c_str(), 255, 255, 255, toolTipPresence>51?255:toolTipPresence*5);
	}

	if(buttonTipShow > 0)
	{
		g->drawtext(16, Size.Y-MENUSIZE-24, (char*)buttonTip.c_str(), 255, 255, 255, buttonTipShow>51?255:buttonTipShow*5);
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
	float snapAngle = floor(atan2((float)point2.Y-point1.Y, point2.X-point1.X)/(M_PI*0.25)+0.5)*M_PI*0.25;
	float lineMag = sqrtf(pow((float)(point2.X-point1.X),2)+pow((float)(point2.Y-point1.Y),2));
	newPoint.X = (int)(lineMag*cos(snapAngle)+point1.X+0.5f);
	newPoint.Y = (int)(lineMag*sin(snapAngle)+point1.Y+0.5f);
	return newPoint;
}

ui::Point GameView::rectSnapCoords(ui::Point point1, ui::Point point2)
{
	ui::Point newPoint(0, 0);
	float snapAngle = floor((atan2((float)point2.Y-point1.Y, point2.X-point1.X)+M_PI*0.25)/(M_PI*0.5)+0.5)*M_PI*0.5 - M_PI*0.25;
	float lineMag = sqrtf(pow((float)(point2.X-point1.X),2)+pow((float)(point2.Y-point1.Y),2));
	newPoint.X = (int)(lineMag*cos(snapAngle)+point1.X+0.5f);
	newPoint.Y = (int)(lineMag*sin(snapAngle)+point1.Y+0.5f);
	return newPoint;
}
