#include "GameView.h"

#include "Brush.h"
#include "Config.h"
#include "DecorationTool.h"
#include "Favorite.h"
#include "Format.h"
#include "GameController.h"
#include "GameModel.h"
#include "IntroText.h"
#include "Menu.h"
#include "MenuButton.h"
#include "Misc.h"
#include "Notification.h"
#include "ToolButton.h"
#include "QuickOptions.h"

#include "client/SaveInfo.h"
#include "client/SaveFile.h"
#include "client/Client.h"
#include "common/Platform.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "gui/Style.h"
#include "simulation/ElementClasses.h"
#include "simulation/ElementDefs.h"
#include "simulation/SaveRenderer.h"
#include "simulation/SimulationData.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/Colour.h"
#include "gui/interface/Keys.h"
#include "gui/interface/Engine.h"

#include <cstring>
#include <iostream>

#ifdef GetUserName
# undef GetUserName // dammit windows
#endif

class SplitButton : public ui::Button
{
	bool rightDown;
	bool leftDown;
	bool showSplit;
	int splitPosition;
	String toolTip2;

	struct SplitButtonAction
	{
		std::function<void ()> left, right;
	};
	SplitButtonAction actionCallback;

public:
	SplitButton(ui::Point position, ui::Point size, String buttonText, String toolTip, String toolTip2, int split) :
		Button(position, size, buttonText, toolTip),
		showSplit(true),
		splitPosition(split),
		toolTip2(toolTip2)
	{

	}
	virtual ~SplitButton() = default;

	void SetRightToolTip(String tooltip) { toolTip2 = tooltip; }
	bool GetShowSplit() { return showSplit; }
	void SetShowSplit(bool split) { showSplit = split; }
	inline SplitButtonAction const &GetSplitActionCallback() { return actionCallback; }
	inline void SetSplitActionCallback(SplitButtonAction const &action) { actionCallback = action; }
	void SetToolTip(int x, int y)
	{
		if(x >= splitPosition || !showSplit)
		{
			if(toolTip2.length()>0 && GetParentWindow())
			{
				GetParentWindow()->ToolTip(Position, toolTip2);
			}
		}
		else if(x < splitPosition)
		{
			if(toolTip.length()>0 && GetParentWindow())
			{
				GetParentWindow()->ToolTip(Position, toolTip);
			}
		}
	}
	void OnMouseUnclick(int x, int y, unsigned int button) override
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
	void OnMouseHover(int x, int y) override
	{
		SetToolTip(x, y);
	}
	void OnMouseEnter(int x, int y) override
	{
		isMouseInside = true;
		if(!Enabled)
			return;
		SetToolTip(x, y);
	}
	void TextPosition(String ButtonText) override
	{
		ui::Button::TextPosition(ButtonText);
		textPosition.X += 3;
	}
	void SetToolTips(String newToolTip1, String newToolTip2)
	{
		toolTip = newToolTip1;
		toolTip2 = newToolTip2;
	}
	void OnMouseClick(int x, int y, unsigned int button) override
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
		if (actionCallback.right)
			actionCallback.right();
	}
	void DoLeftAction()
	{
		if(!Enabled)
			return;
		if (actionCallback.left)
			actionCallback.left();
	}
	void Draw(const ui::Point& screenPos) override
	{
		ui::Button::Draw(screenPos);
		Graphics * g = GetGraphics();
		drawn = true;

		if(showSplit)
			g->draw_line(splitPosition+screenPos.X, screenPos.Y+1, splitPosition+screenPos.X, screenPos.Y+Size.Y-2, 180, 180, 180, 255);
	}
};


GameView::GameView():
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, WINDOWH)),
	isMouseDown(false),
	skipDraw(false),
	zoomEnabled(false),
	zoomCursorFixed(false),
	mouseInZoom(false),
	drawSnap(false),
	shiftBehaviour(false),
	ctrlBehaviour(false),
	altBehaviour(false),
	showHud(true),
	showBrush(true),
	showDebug(false),
	delayedActiveMenu(-1),
	wallBrush(false),
	toolBrush(false),
	decoBrush(false),
	windTool(false),
	toolIndex(0),
	currentSaveType(0),
	lastMenu(-1),

	toolTipPresence(0),
	toolTip(""),
	isToolTipFadingIn(false),
	toolTipPosition(-1, -1),
	infoTipPresence(0),
	infoTip(""),
	buttonTipShow(0),
	buttonTip(""),
	isButtonTipFadingIn(false),
	introText(2048),
	introTextMessage(ByteString(introTextData).FromUtf8()),

	doScreenshot(false),
	screenshotIndex(1),
	lastScreenshotTime(0),
	recordingIndex(0),
	recording(false),
	recordingFolder(0),
	currentPoint(ui::Point(0, 0)),
	lastPoint(ui::Point(0, 0)),
	ren(NULL),
	activeBrush(NULL),
	saveSimulationButtonEnabled(false),
	saveReuploadAllowed(true),
	drawMode(DrawPoints),
	drawPoint1(0, 0),
	drawPoint2(0, 0),
	selectMode(SelectNone),
	selectPoint1(0, 0),
	selectPoint2(0, 0),
	currentMouse(0, 0),
	mousePosition(0, 0),
	placeSaveThumb(NULL),
	placeSaveOffset(0, 0)
{

	int currentX = 1;
	//Set up UI

	scrollBar = new ui::Button(ui::Point(0,YRES+21), ui::Point(XRES, 2), "");
	scrollBar->Appearance.BorderHover = ui::Colour(200, 200, 200);
	scrollBar->Appearance.BorderActive = ui::Colour(200, 200, 200);
	scrollBar->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	scrollBar->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(scrollBar);

	searchButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(17, 15), "", "Find & open a simulation. Hold Ctrl to load offline saves.");  //Open
	searchButton->SetIcon(IconOpen);
	currentX+=18;
	searchButton->SetTogglable(false);
	searchButton->SetActionCallback({ [this] {
		if (CtrlBehaviour())
			c->OpenLocalBrowse();
		else
			c->OpenSearch("");
	} });
	AddComponent(searchButton);

	reloadButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(17, 15), "", "Reload the simulation");
	reloadButton->SetIcon(IconReload);
	reloadButton->Appearance.Margin.Left+=2;
	currentX+=18;
	reloadButton->SetActionCallback({ [this] { c->ReloadSim(); }, [this] { c->OpenSavePreview(); } });
	AddComponent(reloadButton);

	saveSimulationButton = new SplitButton(ui::Point(currentX, Size.Y-16), ui::Point(150, 15), "[untitled simulation]", "", "", 19);
	saveSimulationButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	saveSimulationButton->SetIcon(IconSave);
	currentX+=151;
	saveSimulationButton->SetSplitActionCallback({
		[this] {
			if (CtrlBehaviour() || !Client::Ref().GetAuthUser().UserID)
				c->OpenLocalSaveWindow(true);
			else
				c->SaveAsCurrent();
		},
		[this] {
			if (CtrlBehaviour() || !Client::Ref().GetAuthUser().UserID)
				c->OpenLocalSaveWindow(false);
			else
				c->OpenSaveWindow();
		}
	});
	SetSaveButtonTooltips();
	AddComponent(saveSimulationButton);

	upVoteButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(39, 15), "", "Like this save");
	upVoteButton->SetIcon(IconVoteUp);
	upVoteButton->Appearance.Margin.Top+=2;
	upVoteButton->Appearance.Margin.Left+=2;
	currentX+=38;
	upVoteButton->SetActionCallback({ [this] { c->Vote(1); } });
	AddComponent(upVoteButton);

	downVoteButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(15, 15), "", "Dislike this save");
	downVoteButton->SetIcon(IconVoteDown);
	downVoteButton->Appearance.Margin.Bottom+=2;
	downVoteButton->Appearance.Margin.Left+=2;
	currentX+=16;
	downVoteButton->SetActionCallback({ [this] { c->Vote(-1); } });
	AddComponent(downVoteButton);

	tagSimulationButton = new ui::Button(ui::Point(currentX, Size.Y-16), ui::Point(227, 15), "[no tags set]", "Add simulation tags");
	tagSimulationButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tagSimulationButton->SetIcon(IconTag);
	//currentX+=252;
	tagSimulationButton->SetActionCallback({ [this] { c->OpenTags(); } });
	AddComponent(tagSimulationButton);

	clearSimButton = new ui::Button(ui::Point(Size.X-159, Size.Y-16), ui::Point(17, 15), "", "Erase everything");
	clearSimButton->SetIcon(IconNew);
	clearSimButton->Appearance.Margin.Left+=2;
	clearSimButton->SetActionCallback({ [this] { c->ClearSim(); } });
	AddComponent(clearSimButton);

	loginButton = new SplitButton(ui::Point(Size.X-141, Size.Y-16), ui::Point(92, 15), "[sign in]", "Sign into simulation server", "Edit Profile", 19);
	loginButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	loginButton->SetIcon(IconLogin);
	loginButton->SetSplitActionCallback({
		[this] { c->OpenLogin(); },
		[this] { c->OpenProfile(); }
	});
	AddComponent(loginButton);

	simulationOptionButton = new ui::Button(ui::Point(Size.X-48, Size.Y-16), ui::Point(15, 15), "", "Simulation options");
	simulationOptionButton->SetIcon(IconSimulationSettings);
	simulationOptionButton->Appearance.Margin.Left+=2;
	simulationOptionButton->SetActionCallback({ [this] { c->OpenOptions(); } });
	AddComponent(simulationOptionButton);

	displayModeButton = new ui::Button(ui::Point(Size.X-32, Size.Y-16), ui::Point(15, 15), "", "Renderer options");
	displayModeButton->SetIcon(IconRenderSettings);
	displayModeButton->Appearance.Margin.Left+=2;
	displayModeButton->SetActionCallback({ [this] { c->OpenRenderOptions(); } });
	AddComponent(displayModeButton);

	pauseButton = new ui::Button(ui::Point(Size.X-16, Size.Y-16), ui::Point(15, 15), "", "Pause/Resume the simulation");  //Pause
	pauseButton->SetIcon(IconPause);
	pauseButton->SetTogglable(true);
	pauseButton->SetActionCallback({ [this] { c->SetPaused(pauseButton->GetToggleState()); } });
	AddComponent(pauseButton);

	ui::Button * tempButton = new ui::Button(ui::Point(WINDOWW-16, WINDOWH-32), ui::Point(15, 15), 0xE065, "Search for elements");
	tempButton->Appearance.Margin = ui::Border(0, 2, 3, 2);
	tempButton->SetActionCallback({ [this] { c->OpenElementSearch(); } });
	AddComponent(tempButton);

	colourPicker = new ui::Button(ui::Point((XRES/2)-8, YRES+1), ui::Point(16, 16), "", "Pick Colour");
	colourPicker->SetActionCallback({ [this] { c->OpenColourPicker(); } });
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

	delete placeSaveThumb;
}

class GameView::OptionListener: public QuickOptionListener
{
	ui::Button * button;
public:
	OptionListener(ui::Button * _button) { button = _button; }
	void OnValueChanged(QuickOption * option) override
	{
		switch(option->GetType())
		{
		case QuickOption::Toggle:
			button->SetTogglable(true);
			button->SetToggleState(option->GetToggle());
			break;
		default:
			break;
		}
	}
};

void GameView::NotifyQuickOptionsChanged(GameModel * sender)
{
	for (size_t i = 0; i < quickOptionButtons.size(); i++)
	{
		RemoveComponent(quickOptionButtons[i]);
		delete quickOptionButtons[i];
	}

	int currentY = 1;
	std::vector<QuickOption*> optionList = sender->GetQuickOptions();
	for(auto *option : optionList)
	{
		ui::Button * tempButton = new ui::Button(ui::Point(WINDOWW-16, currentY), ui::Point(15, 15), option->GetIcon(), option->GetDescription());
		//tempButton->Appearance.Margin = ui::Border(0, 2, 3, 2);
		tempButton->SetTogglable(true);
		tempButton->SetActionCallback({ [option] {
			option->Perform();
		} });
		option->AddListener(new OptionListener(tempButton));
		AddComponent(tempButton);

		quickOptionButtons.push_back(tempButton);
		currentY += 16;
	}
}

void GameView::NotifyMenuListChanged(GameModel * sender)
{
	int currentY = WINDOWH-48;//-(sender->GetMenuList().size()*16);
	for (size_t i = 0; i < menuButtons.size(); i++)
	{
		RemoveComponent(menuButtons[i]);
		delete menuButtons[i];
	}
	menuButtons.clear();
	for (size_t i = 0; i < toolButtons.size(); i++)
	{
		RemoveComponent(toolButtons[i]);
		delete toolButtons[i];
	}
	toolButtons.clear();
	std::vector<Menu*> menuList = sender->GetMenuList();
	for (int i = (int)menuList.size()-1; i >= 0; i--)
	{
		if (menuList[i]->GetVisible())
		{
			String tempString = "";
			tempString += menuList[i]->GetIcon();
			String description = menuList[i]->GetDescription();
			if (i == SC_FAVORITES && !Favorite::Ref().AnyFavorites())
				description += " (Use ctrl+shift+click to toggle the favorite status of an element)";
			auto *tempButton = new MenuButton(ui::Point(WINDOWW-16, currentY), ui::Point(15, 15), tempString, description);
			tempButton->Appearance.Margin = ui::Border(0, 2, 3, 2);
			tempButton->menuID = i;
			tempButton->needsClick = i == SC_DECO;
			tempButton->SetTogglable(true);
			auto mouseEnterCallback = [this, tempButton] {
				// don't immediately change the active menu, the actual set is done inside GameView::OnMouseMove
				// if we change it here it causes components to be removed, which causes the window to stop sending events
				// and then the previous menusection button never gets sent the OnMouseLeave event and is never unhighlighted
				if(!(tempButton->needsClick || c->GetMouseClickRequired()) && !GetMouseDown())
					SetActiveMenuDelayed(tempButton->menuID);
			};
			auto actionCallback = [this, tempButton, mouseEnterCallback] {
				if (tempButton->needsClick || c->GetMouseClickRequired())
					c->SetActiveMenu(tempButton->menuID);
				else
					mouseEnterCallback();
			};
			tempButton->SetActionCallback({ actionCallback, nullptr, mouseEnterCallback });
			currentY-=16;
			AddComponent(tempButton);
			menuButtons.push_back(tempButton);
		}
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

void GameView::SetBrushEnable(bool brushState)
{
	showBrush = brushState;
}

bool GameView::GetBrushEnable()
{
	return showBrush;
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

bool GameView::GetPlacingSave()
{
	return selectMode != SelectNone;
}

bool GameView::GetPlacingZoom()
{
	return zoomEnabled && !zoomCursorFixed;
}

void GameView::NotifyActiveToolsChanged(GameModel * sender)
{
	for (size_t i = 0; i < toolButtons.size(); i++)
	{
		auto *tool = toolButtons[i]->tool;
		// Primary
		if (sender->GetActiveTool(0) == tool)
			toolButtons[i]->SetSelectionState(0);
		// Secondary
		else if (sender->GetActiveTool(1) == tool)
			toolButtons[i]->SetSelectionState(1);
		// Tertiary
		else if (sender->GetActiveTool(2) == tool)
			toolButtons[i]->SetSelectionState(2);
		// Replace Mode
		else if (sender->GetActiveTool(3) == tool)
			toolButtons[i]->SetSelectionState(3);
		// Not selected at all
		else
			toolButtons[i]->SetSelectionState(-1);
	}

	decoBrush = sender->GetActiveTool(0)->GetIdentifier().BeginsWith("DEFAULT_DECOR_");

	if (sender->GetRenderer()->findingElement)
	{
		Tool *active = sender->GetActiveTool(0);
		if (!active->GetIdentifier().Contains("_PT_"))
			ren->findingElement = 0;
		else
			ren->findingElement = sender->GetActiveTool(0)->GetToolID();
	}
}

void GameView::NotifyLastToolChanged(GameModel * sender)
{
	if (sender->GetLastTool())
	{
		wallBrush = sender->GetLastTool()->GetBlocky();
		toolBrush = sender->GetLastTool()->GetIdentifier().BeginsWith("DEFAULT_TOOL_");
	}
}

void GameView::NotifyToolListChanged(GameModel * sender)
{
	for (size_t i = 0; i < menuButtons.size(); i++)
	{
		if (menuButtons[i]->menuID==sender->GetActiveMenu())
		{
			menuButtons[i]->SetToggleState(true);
		}
		else
		{
			menuButtons[i]->SetToggleState(false);
		}
	}
	for (size_t i = 0; i < toolButtons.size(); i++)
	{
		RemoveComponent(toolButtons[i]);
		delete toolButtons[i];
	}
	toolButtons.clear();
	std::vector<Tool*> toolList = sender->GetToolList();
	int currentX = 0;
	for (size_t i = 0; i < toolList.size(); i++)
	{
		auto *tool = toolList[i];
		VideoBuffer * tempTexture = tool->GetTexture(26, 14);
		ToolButton * tempButton;

		//get decotool texture manually, since it changes depending on it's own color
		if (sender->GetActiveMenu() == SC_DECO)
			tempTexture = ((DecorationTool*)tool)->GetIcon(tool->GetToolID(), 26, 14);

		if(tempTexture)
			tempButton = new ToolButton(ui::Point(currentX, YRES+1), ui::Point(30, 18), "", tool->GetIdentifier(), tool->GetDescription());
		else
			tempButton = new ToolButton(ui::Point(currentX, YRES+1), ui::Point(30, 18), tool->GetName(), tool->GetIdentifier(), tool->GetDescription());

		tempButton->clipRectX = 1;
		tempButton->clipRectY = YRES + 1;
		tempButton->clipRectW = XRES - 1;
		tempButton->clipRectH = 18;

		//currentY -= 17;
		currentX -= 31;
		tempButton->tool = tool;
		tempButton->SetActionCallback({ [this, tempButton] {
			auto *tool = tempButton->tool;
			if (ShiftBehaviour() && CtrlBehaviour() && !AltBehaviour())
			{
				if (tempButton->GetSelectionState() == 0)
				{
					if (Favorite::Ref().IsFavorite(tool->GetIdentifier()))
					{
						Favorite::Ref().RemoveFavorite(tool->GetIdentifier());
					}
					else
					{
						Favorite::Ref().AddFavorite(tool->GetIdentifier());
					}
					c->RebuildFavoritesMenu();
				}
				else if (tempButton->GetSelectionState() == 1)
				{
					auto identifier = tool->GetIdentifier();
					if (Favorite::Ref().IsFavorite(identifier))
					{
						Favorite::Ref().RemoveFavorite(identifier);
						c->RebuildFavoritesMenu();
					}
					else if (identifier.BeginsWith("DEFAULT_PT_LIFECUST_"))
					{
						if (ConfirmPrompt::Blocking("Remove custom GOL type", "Are you sure you want to remove " + identifier.Substr(20).FromUtf8() + "?"))
						{
							c->RemoveCustomGOLType(identifier);
						}
					}
				}
			}
			else
			{
				if (CtrlBehaviour() && AltBehaviour() && !ShiftBehaviour())
				{
					if (tool->GetIdentifier().Contains("_PT_"))
					{
						tempButton->SetSelectionState(3);
					}
				}

				if (tempButton->GetSelectionState() >= 0 && tempButton->GetSelectionState() <= 3)
					c->SetActiveTool(tempButton->GetSelectionState(), tool);
			}
		} });

		tempButton->Appearance.SetTexture(tempTexture);
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
		else if(sender->GetActiveTool(3) == toolList[i])
		{
			tempButton->SetSelectionState(3);	//Replace mode
		}

		tempButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
		tempButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
		AddComponent(tempButton);
		toolButtons.push_back(tempButton);
	}
	if (sender->GetActiveMenu() != SC_DECO)
		lastMenu = sender->GetActiveMenu();

	updateToolButtonScroll();
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
		c->SetActiveColourPreset(-1);
	}
}

void GameView::NotifyColourPresetsChanged(GameModel * sender)
{
	for (auto *button : colourPresets)
	{
		RemoveComponent(button);
		delete button;
	}
	colourPresets.clear();

	int currentX = 5;
	std::vector<ui::Colour> colours = sender->GetColourPresets();
	int i = 0;
	for(std::vector<ui::Colour>::iterator iter = colours.begin(), end = colours.end(); iter != end; ++iter)
	{
		ToolButton * tempButton = new ToolButton(ui::Point(currentX, YRES+1), ui::Point(30, 18), "", "", "Decoration Presets.");
		tempButton->Appearance.BackgroundInactive = *iter;
		tempButton->SetActionCallback({ [this, i, tempButton] {
			c->SetActiveColourPreset(i);
			c->SetColour(tempButton->Appearance.BackgroundInactive);
		} });

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
	for (size_t i = 0; i < colourPresets.size(); i++)
	{
		if (sender->GetActiveColourPreset() == i)
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
	if(!sender->GetUser().UserID)
	{
		loginButton->SetText("[sign in]");
		loginButton->SetShowSplit(false);
		loginButton->SetRightToolTip("Sign in to simulation server");
	}
	else
	{
		loginButton->SetText(sender->GetUser().Username.FromUtf8());
		loginButton->SetShowSplit(true);
		loginButton->SetRightToolTip("Edit profile");
	}
	// saveSimulationButtonEnabled = sender->GetUser().ID;
	saveSimulationButtonEnabled = true;
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
	saveReuploadAllowed = true;
	if (sender->GetSave())
	{
		if (introText > 50)
			introText = 50;

		saveSimulationButton->SetText(sender->GetSave()->GetName());
		if (sender->GetSave()->GetUserName() == sender->GetUser().Username)
			saveSimulationButton->SetShowSplit(true);
		else
			saveSimulationButton->SetShowSplit(false);
		reloadButton->Enabled = true;
		upVoteButton->Enabled = (sender->GetSave()->GetID() && sender->GetUser().UserID && sender->GetSave()->GetVote()==0);
		if(sender->GetSave()->GetID() && sender->GetUser().UserID && sender->GetSave()->GetVote()==1)
			upVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 108, 10, 255));
		else
			upVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));

		downVoteButton->Enabled = upVoteButton->Enabled;
		if (sender->GetSave()->GetID() && sender->GetUser().UserID && sender->GetSave()->GetVote()==-1)
			downVoteButton->Appearance.BackgroundDisabled = (ui::Colour(108, 0, 10, 255));
		else
			downVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));

		if (sender->GetUser().UserID)
		{
			upVoteButton->Appearance.BorderDisabled = upVoteButton->Appearance.BorderInactive;
			downVoteButton->Appearance.BorderDisabled = downVoteButton->Appearance.BorderInactive;
		}
		else
		{
			upVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100);
			downVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100);
		}

		tagSimulationButton->Enabled = sender->GetSave()->GetID();
		if (sender->GetSave()->GetID())
		{
			StringBuilder tagsStream;
			std::list<ByteString> tags = sender->GetSave()->GetTags();
			if (tags.size())
			{
				for (std::list<ByteString>::const_iterator iter = tags.begin(), begin = tags.begin(), end = tags.end(); iter != end; iter++)
				{
					if (iter != begin)
						tagsStream << " ";
					tagsStream << iter->FromUtf8();
				}
				tagSimulationButton->SetText(tagsStream.Build());
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
		int saveID = sender->GetSave()->GetID();
		if (saveID == 404 || saveID == 2157797)
			saveReuploadAllowed = false;
	}
	else if (sender->GetSaveFile())
	{
		if (ctrlBehaviour)
			saveSimulationButton->SetShowSplit(true);
		else
			saveSimulationButton->SetShowSplit(false);
		saveSimulationButton->SetText(sender->GetSaveFile()->GetDisplayName());
		reloadButton->Enabled = true;
		upVoteButton->Enabled = false;
		upVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));
		upVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100);
		downVoteButton->Enabled = false;
		downVoteButton->Appearance.BackgroundDisabled = (ui::Colour(0, 0, 0));
		downVoteButton->Appearance.BorderDisabled = ui::Colour(100, 100, 100);
		tagSimulationButton->Enabled = false;
		tagSimulationButton->SetText("[no tags set]");
		currentSaveType = 2;
	}
	else
	{
		saveSimulationButton->SetShowSplit(false);
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
	saveSimulationButton->Enabled = (saveSimulationButtonEnabled && saveReuploadAllowed) || ctrlBehaviour;
	SetSaveButtonTooltips();
}

void GameView::NotifyBrushChanged(GameModel * sender)
{
	activeBrush = sender->GetBrush();
}

ByteString GameView::TakeScreenshot(int captureUI, int fileType)
{
	std::unique_ptr<VideoBuffer> screenshot;
	if (captureUI)
	{
		screenshot = std::make_unique<VideoBuffer>(ren->DumpFrame());
	}
	else
	{
		screenshot = std::make_unique<VideoBuffer>(ui::Engine::Ref().g->DumpFrame());
	}

	ByteString filename;
	{
		// Optional suffix to distinguish screenshots taken at the exact same time
		ByteString suffix = "";
		time_t screenshotTime = time(nullptr);
		if (screenshotTime == lastScreenshotTime)
		{
			screenshotIndex++;
			suffix = ByteString::Build(" (", screenshotIndex, ")");
		}
		else
		{
			screenshotIndex = 1;
		}
		lastScreenshotTime = screenshotTime;
		std::string date = format::UnixtimeToDate(screenshotTime, "%Y-%m-%d %H.%M.%S");
		filename = ByteString::Build("screenshot ", date, suffix);
	}

	if (fileType == 1)
	{
		filename += ".bmp";
		// We should be able to simply use SDL_PIXELFORMAT_XRGB8888 here with a bit depth of 32 to convert RGBA data to RGB data,
		// and save the resulting surface directly. However, ubuntu-18.04 ships SDL2 so old that it doesn't have
		// SDL_PIXELFORMAT_XRGB8888, so we first create an RGBA surface and then convert it.
		auto *rgbaSurface = SDL_CreateRGBSurfaceWithFormatFrom(screenshot->Buffer, screenshot->Width, screenshot->Height, 32, screenshot->Width * sizeof(pixel), SDL_PIXELFORMAT_ARGB8888);
		auto *rgbSurface = SDL_ConvertSurfaceFormat(rgbaSurface, SDL_PIXELFORMAT_RGB888, 0);
		if (!rgbSurface || SDL_SaveBMP(rgbSurface, filename.c_str()))
		{
			std::cerr << "SDL_SaveBMP failed: " << SDL_GetError() << std::endl;
			filename = "";
		}
		SDL_FreeSurface(rgbSurface);
		SDL_FreeSurface(rgbaSurface);
	}
	else if (fileType == 2)
	{
		filename += ".ppm";
		if (!Platform::WriteFile(format::VideoBufferToPPM(*screenshot), filename))
		{
			filename = "";
		}
	}
	else
	{
		filename += ".png";
		if (!screenshot->WritePNG(filename))
		{
			filename = "";
		}
	}

	return filename;
}

int GameView::Record(bool record)
{
	if (!record)
	{
		recording = false;
		recordingFolder = 0;
	}
	else if (!recording)
	{
		// block so that the return value is correct
		bool record = ConfirmPrompt::Blocking("Recording", "You're about to start recording all drawn frames. This will use a load of disk space.");
		if (record)
		{
			time_t startTime = time(NULL);
			recordingFolder = startTime;
			Platform::MakeDirectory("recordings");
			Platform::MakeDirectory(ByteString::Build("recordings", PATH_SEP, recordingFolder).c_str());
			recording = true;
			recordingIndex = 0;
		}
	}
	return recordingFolder;
}

void GameView::updateToolButtonScroll()
{
	if (toolButtons.size())
	{
		int x = currentMouse.X;
		int y = currentMouse.Y;

		int offsetDelta = 0;

		int newInitialX = WINDOWW - 56;
		int totalWidth = (toolButtons[0]->Size.X + 1) * toolButtons.size();
		int scrollSize = (int)(((float)(XRES - BARSIZE))/((float)totalWidth) * ((float)XRES - BARSIZE));

		if (scrollSize > XRES - 1)
			scrollSize = XRES - 1;
		
		if (totalWidth > XRES - 15)
		{			
			int mouseX = x;

			float overflow = 0;
			float mouseLocation = 0;

			if (mouseX > XRES)
				mouseX = XRES;

			// if (mouseX < 15) // makes scrolling a little nicer at edges but apparently if you put hundreds of elements in a menu it makes the end not show ...
			// 	mouseX = 15;

			scrollBar->Visible = true;

			scrollBar->Position.X = (int)(((float)mouseX / (float)XRES) * (float)(XRES - scrollSize)) + 1;

			overflow = (float)(totalWidth - (XRES - BARSIZE));
			mouseLocation = (float)(XRES - 3)/(float)((XRES - 2) - mouseX); // mouseLocation adjusted slightly in case you have 200 elements in one menu

			newInitialX += (int)(overflow/mouseLocation);
		}
		else
		{
			scrollBar->Visible = false;
		}

		scrollBar->Size.X = scrollSize - 1;

		offsetDelta = toolButtons[0]->Position.X - newInitialX;

		for (auto *button : toolButtons)
		{
			button->Position.X -= offsetDelta;
		}

		// Ensure that mouseLeave events are make their way to the buttons should they move from underneath the mouse pointer
		if (toolButtons[0]->Position.Y < y && toolButtons[0]->Position.Y + toolButtons[0]->Size.Y > y)
		{
			for (auto *button : toolButtons)
			{
				if (button->Position.X < x && button->Position.X + button->Size.X > x)
					button->OnMouseEnter(x, y);
				else
					button->OnMouseLeave(x, y);
			}
		}
	}
}

void GameView::OnMouseMove(int x, int y, int dx, int dy)
{
	bool newMouseInZoom = c->MouseInZoom(ui::Point(x, y));
	mousePosition = c->PointTranslate(ui::Point(x, y));
	currentMouse = ui::Point(x, y);
	if (selectMode != SelectNone)
	{
		if (selectMode == PlaceSave)
			selectPoint1 = c->PointTranslate(ui::Point(x, y));
		if (selectPoint1.X != -1)
			selectPoint2 = c->PointTranslate(ui::Point(x, y));
	}
	else if (isMouseDown)
	{
		if (newMouseInZoom == mouseInZoom)
		{
			if (drawMode == DrawPoints)
			{
				currentPoint = mousePosition;
				c->DrawPoints(toolIndex, lastPoint, currentPoint, true);
				lastPoint = currentPoint;
				skipDraw = true;
			}
			else if (drawMode == DrawFill)
			{
				c->DrawFill(toolIndex, mousePosition);
				skipDraw = true;
			}
		}
		else if (drawMode == DrawPoints || drawMode == DrawFill)
		{
			isMouseDown = false;
			drawMode = DrawPoints;
			c->MouseUp(x, y, 0, GameController::mouseUpDrawEnd);
		}
	}
	mouseInZoom = newMouseInZoom;

	// set active menu (delayed)
	if (delayedActiveMenu != -1)
	{
		c->SetActiveMenu(delayedActiveMenu);
		delayedActiveMenu = -1;
	}

	updateToolButtonScroll();
}

void GameView::OnMouseDown(int x, int y, unsigned button)
{
	currentMouse = ui::Point(x, y);
	if (altBehaviour && !shiftBehaviour && !ctrlBehaviour)
		button = SDL_BUTTON_MIDDLE;
	if  (!(zoomEnabled && !zoomCursorFixed))
	{
		if (selectMode != SelectNone)
		{
			isMouseDown = true;
			if (button == SDL_BUTTON_LEFT && selectPoint1.X == -1)
			{
				selectPoint1 = c->PointTranslate(currentMouse);
				selectPoint2 = selectPoint1;
			}
			return;
		}
		if (currentMouse.X >= 0 && currentMouse.X < XRES && currentMouse.Y >= 0 && currentMouse.Y < YRES)
		{
			// update tool index, set new "last" tool so GameView can detect certain tools properly
			if (button == SDL_BUTTON_LEFT)
				toolIndex = 0;
			else if (button == SDL_BUTTON_RIGHT)
				toolIndex = 1;
			else if (button == SDL_BUTTON_MIDDLE)
				toolIndex = 2;
			else
				return;
			Tool *lastTool = c->GetActiveTool(toolIndex);
			c->SetLastTool(lastTool);
			windTool = lastTool->GetIdentifier() == "DEFAULT_UI_WIND";
			decoBrush = lastTool->GetIdentifier().BeginsWith("DEFAULT_DECOR_");

			UpdateDrawMode();

			isMouseDown = true;
			c->HistorySnapshot();
			if (drawMode == DrawRect || drawMode == DrawLine)
			{
				drawPoint1 = c->PointTranslate(currentMouse);
			}
			else if (drawMode == DrawPoints)
			{
				lastPoint = currentPoint = c->PointTranslate(currentMouse);
				c->DrawPoints(toolIndex, lastPoint, currentPoint, false);
			}
			else if (drawMode == DrawFill)
			{
				c->DrawFill(toolIndex, c->PointTranslate(currentMouse));
			}
		}
	}
}

void GameView::OnMouseUp(int x, int y, unsigned button)
{
	currentMouse = ui::Point(x, y);
	if (zoomEnabled && !zoomCursorFixed)
	{
		zoomCursorFixed = true;
		drawMode = DrawPoints;
		isMouseDown = false;
	}
	else if (isMouseDown)
	{
		isMouseDown = false;
		if (selectMode != SelectNone)
		{
			if (button == SDL_BUTTON_LEFT && selectPoint1.X != -1 && selectPoint1.Y != -1 && selectPoint2.X != -1 && selectPoint2.Y != -1)
			{
				if (selectMode == PlaceSave)
				{
					if (placeSaveThumb && y <= WINDOWH-BARSIZE)
					{
						int thumbX = selectPoint2.X - ((placeSaveThumb->Width-placeSaveOffset.X)/2);
						int thumbY = selectPoint2.Y - ((placeSaveThumb->Height-placeSaveOffset.Y)/2);

						if (thumbX < 0)
							thumbX = 0;
						if (thumbX+(placeSaveThumb->Width) >= XRES)
							thumbX = XRES-placeSaveThumb->Width;

						if (thumbY < 0)
							thumbY = 0;
						if (thumbY+(placeSaveThumb->Height) >= YRES)
							thumbY = YRES-placeSaveThumb->Height;

						c->PlaceSave(ui::Point(thumbX, thumbY));
					}
				}
				else
				{
					int x2 = (selectPoint1.X>selectPoint2.X) ? selectPoint1.X : selectPoint2.X;
					int y2 = (selectPoint1.Y>selectPoint2.Y) ? selectPoint1.Y : selectPoint2.Y;
					int x1 = (selectPoint2.X<selectPoint1.X) ? selectPoint2.X : selectPoint1.X;
					int y1 = (selectPoint2.Y<selectPoint1.Y) ? selectPoint2.Y : selectPoint1.Y;
					if (selectMode ==SelectCopy)
						c->CopyRegion(ui::Point(x1, y1), ui::Point(x2, y2));
					else if (selectMode == SelectCut)
						c->CutRegion(ui::Point(x1, y1), ui::Point(x2, y2));
					else if (selectMode == SelectStamp)
						c->StampRegion(ui::Point(x1, y1), ui::Point(x2, y2));
				}
			}
			selectMode = SelectNone;
			return;
		}

		ui::Point finalDrawPoint2 = c->PointTranslate(currentMouse);
		if (drawMode == DrawRect || drawMode == DrawLine)
		{
			drawPoint2 = finalDrawPoint2;
			if (drawSnap && drawMode == DrawLine)
			{
				finalDrawPoint2 = lineSnapCoords(c->PointTranslate(drawPoint1), drawPoint2);
			}
			if (drawSnap && drawMode == DrawRect)
			{
				finalDrawPoint2 = rectSnapCoords(c->PointTranslate(drawPoint1), drawPoint2);
			}

			if (drawMode == DrawRect)
			{
				c->DrawRect(toolIndex, c->PointTranslate(drawPoint1), finalDrawPoint2);
			}
			if (drawMode == DrawLine)
			{
				c->DrawLine(toolIndex, c->PointTranslate(drawPoint1), finalDrawPoint2);
			}
		}
		else if (drawMode == DrawPoints)
		{
			// draw final line
			c->DrawPoints(toolIndex, lastPoint, finalDrawPoint2, true);
			// plop tool stuff (like STKM)
			c->ToolClick(toolIndex, finalDrawPoint2);
		}
		else if (drawMode == DrawFill)
		{
			c->DrawFill(toolIndex, finalDrawPoint2);
		}
	}
	// this shouldn't happen, but do this just in case
	else if (selectMode != SelectNone && button != SDL_BUTTON_LEFT)
		selectMode = SelectNone;

	// update the drawing mode for the next line
	// since ctrl/shift state may have changed since we started drawing
	UpdateDrawMode();
}

void GameView::ToolTip(ui::Point senderPosition, String toolTip)
{
	// buttom button tooltips
	if (senderPosition.Y > Size.Y-17)
	{
		if (selectMode == PlaceSave || selectMode == SelectNone)
		{
			buttonTip = toolTip;
			isButtonTipFadingIn = true;
		}
	}
	// quickoption and menu tooltips
	else if(senderPosition.X > Size.X-BARSIZE)// < Size.Y-(quickOptionButtons.size()+1)*16)
	{
		this->toolTip = toolTip;
		toolTipPosition = ui::Point(Size.X-27-Graphics::textwidth(toolTip), senderPosition.Y+3);
		if(toolTipPosition.Y+10 > Size.Y-MENUSIZE)
			toolTipPosition = ui::Point(Size.X-27-Graphics::textwidth(toolTip), Size.Y-MENUSIZE-10);
		isToolTipFadingIn = true;
	}
	// element tooltips
	else
	{
		this->toolTip = toolTip;
		toolTipPosition = ui::Point(Size.X-27-Graphics::textwidth(toolTip), Size.Y-MENUSIZE-10);
		isToolTipFadingIn = true;
	}
}

void GameView::OnMouseWheel(int x, int y, int d)
{
	if (!d)
		return;
	if (selectMode != SelectNone)
	{
		return;
	}
	if (zoomEnabled && !zoomCursorFixed)
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
	selectPoint1 = selectPoint2 = ui::Point(-1, -1);
	isMouseDown = false;
	buttonTip = "\x0F\xEF\xEF\020Click-and-drag to specify an area to create a stamp (right click = cancel)";
	buttonTipShow = 120;
}

void GameView::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (introText > 50)
	{
		introText = 50;
	}

	if (selectMode != SelectNone)
	{
		if (selectMode == PlaceSave)
		{
			switch (key)
			{
			case SDLK_RIGHT:
				c->TranslateSave(ui::Point(1, 0));
				return;
			case SDLK_LEFT:
				c->TranslateSave(ui::Point(-1, 0));
				return;
			case SDLK_UP:
				c->TranslateSave(ui::Point(0, -1));
				return;
			case SDLK_DOWN:
				c->TranslateSave(ui::Point(0, 1));
				return;
			}
			if (scan == SDL_SCANCODE_R && !repeat)
			{
				if (ctrl && shift)
				{
					//Vertical flip
					c->TransformSave(m2d_new(1,0,0,-1));
				}
				else if (!ctrl && shift)
				{
					//Horizontal flip
					c->TransformSave(m2d_new(-1,0,0,1));
				}
				else
				{
					//Rotate 90deg
					c->TransformSave(m2d_new(0,1,-1,0));
				}
				return;
			}
		}
	}

	if (repeat)
		return;
	bool didKeyShortcut = true;
	switch(scan)
	{
	case SDL_SCANCODE_GRAVE:
		c->ShowConsole();
		break;
	case SDL_SCANCODE_SPACE: //Space
		c->SetPaused();
		break;
	case SDL_SCANCODE_Z:
		if (selectMode != SelectNone && isMouseDown)
			break;
		if (ctrl && !isMouseDown)
		{
			if (shift)
				c->HistoryForward();
			else
				c->HistoryRestore();
		}
		else
		{
			isMouseDown = false;
			zoomCursorFixed = false;
			c->SetZoomEnabled(true);
		}
		break;
	case SDL_SCANCODE_P:
	case SDL_SCANCODE_F2:
		if (ctrl)
		{
			if (shift)
				c->SetActiveTool(1, "DEFAULT_UI_PROPERTY");
			else
				c->SetActiveTool(0, "DEFAULT_UI_PROPERTY");
		}
		else
			doScreenshot = true;
		break;
	case SDL_SCANCODE_F3:
		SetDebugHUD(!GetDebugHUD());
		break;
	case SDL_SCANCODE_F5:
		c->ReloadSim();
		break;
	case SDL_SCANCODE_A:
		if ((Client::Ref().GetAuthUser().UserElevation == User::ElevationModerator
		     || Client::Ref().GetAuthUser().UserElevation == User::ElevationAdmin) && ctrl)
		{
			ByteString authorString = Client::Ref().GetAuthorInfo().toStyledString();
			new InformationMessage("Save authorship info", authorString.FromUtf8(), true);
		}
		break;
	case SDL_SCANCODE_R:
		if (ctrl)
			c->ReloadSim();
		break;
	case SDL_SCANCODE_E:
		c->OpenElementSearch();
		break;
	case SDL_SCANCODE_F:
		if (ctrl)
		{
			Tool *active = c->GetActiveTool(0);
			if (!active->GetIdentifier().Contains("_PT_") || (ren->findingElement == active->GetToolID()))
				ren->findingElement = 0;
			else
				ren->findingElement = active->GetToolID();
		}
		else
			c->FrameStep();
		break;
	case SDL_SCANCODE_G:
		if (ctrl)
			c->ShowGravityGrid();
		else if(shift)
			c->AdjustGridSize(-1);
		else
			c->AdjustGridSize(1);
		break;
	case SDL_SCANCODE_F1:
		if(!introText)
			introText = 8047;
		else
			introText = 0;
		break;
	case SDL_SCANCODE_H:
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
	case SDL_SCANCODE_B:
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
	case SDL_SCANCODE_Y:
		if (ctrl)
		{
			c->HistoryForward();
		}
		else
		{
			c->SwitchAir();
		}
		break;
	case SDL_SCANCODE_ESCAPE:
	case SDL_SCANCODE_Q:
		ui::Engine::Ref().ConfirmExit();
		break;
	case SDL_SCANCODE_U:
		if (ctrl)
			c->ResetAHeat();
		else
			c->ToggleAHeat();
		break;
	case SDL_SCANCODE_N:
		c->ToggleNewtonianGravity();
		break;
	case SDL_SCANCODE_EQUALS:
		if(ctrl)
			c->ResetSpark();
		else
			c->ResetAir();
		break;
	case SDL_SCANCODE_C:
		if(ctrl)
		{
			selectMode = SelectCopy;
			selectPoint1 = selectPoint2 = ui::Point(-1, -1);
			isMouseDown = false;
			buttonTip = "\x0F\xEF\xEF\020Click-and-drag to specify an area to copy (right click = cancel)";
			buttonTipShow = 120;
		}
		break;
	case SDL_SCANCODE_X:
		if(ctrl)
		{
			selectMode = SelectCut;
			selectPoint1 = selectPoint2 = ui::Point(-1, -1);
			isMouseDown = false;
			buttonTip = "\x0F\xEF\xEF\020Click-and-drag to specify an area to copy then cut (right click = cancel)";
			buttonTipShow = 120;
		}
		break;
	case SDL_SCANCODE_V:
		if (ctrl)
		{
			if (c->LoadClipboard())
			{
				selectPoint1 = selectPoint2 = mousePosition;
				isMouseDown = false;
			}
		}
		break;
	case SDL_SCANCODE_L:
	{
		std::vector<ByteString> stampList = Client::Ref().GetStamps(0, 1);
		if (stampList.size())
		{
			SaveFile *saveFile = Client::Ref().GetStamp(stampList[0]);
			if (!saveFile || !saveFile->GetGameSave())
				break;
			c->LoadStamp(saveFile->GetGameSave());
			delete saveFile;
			selectPoint1 = selectPoint2 = mousePosition;
			isMouseDown = false;
			break;
		}
	}
	case SDL_SCANCODE_K:
		selectMode = SelectNone;
		selectPoint1 = selectPoint2 = ui::Point(-1, -1);
		c->OpenStamps();
		break;
	case SDL_SCANCODE_RIGHTBRACKET:
		if(zoomEnabled && !zoomCursorFixed)
			c->AdjustZoomSize(1, !alt);
		else
			c->AdjustBrushSize(1, !alt, shiftBehaviour, ctrlBehaviour);
		break;
	case SDL_SCANCODE_LEFTBRACKET:
		if(zoomEnabled && !zoomCursorFixed)
			c->AdjustZoomSize(-1, !alt);
		else
			c->AdjustBrushSize(-1, !alt, shiftBehaviour, ctrlBehaviour);
		break;
	case SDL_SCANCODE_I:
		if(ctrl)
			c->Install();
		else
			c->InvertAirSim();
		break;
	case SDL_SCANCODE_SEMICOLON:
		if (ctrl)
			c->SetReplaceModeFlags(c->GetReplaceModeFlags()^SPECIFIC_DELETE);
		else
			c->SetReplaceModeFlags(c->GetReplaceModeFlags()^REPLACE_MODE);
		break;
	default:
		didKeyShortcut = false;
	}
	if (!didKeyShortcut)
	{
		switch (key)
		{
		case SDLK_TAB: //Tab
			c->ChangeBrush();
			break;
		case SDLK_INSERT:
			if (ctrl)
				c->SetReplaceModeFlags(c->GetReplaceModeFlags()^SPECIFIC_DELETE);
			else
				c->SetReplaceModeFlags(c->GetReplaceModeFlags()^REPLACE_MODE);
			break;
		case SDLK_DELETE:
			c->SetReplaceModeFlags(c->GetReplaceModeFlags()^SPECIFIC_DELETE);
			break;
		}
	}

	if (shift && showDebug && key == '1')
		c->LoadRenderPreset(10);
	else if (key >= '0' && key <= '9')
	{
		c->LoadRenderPreset(key-'0');
	}
}

void GameView::OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	if (scan == SDL_SCANCODE_Z)
	{
		if (!zoomCursorFixed && !alt)
			c->SetZoomEnabled(false);
		return;
	}
}

void GameView::OnBlur()
{
	disableAltBehaviour();
	disableCtrlBehaviour();
	disableShiftBehaviour();
	isMouseDown = false;
	drawMode = DrawPoints;
	c->Blur();
}

void GameView::OnFileDrop(ByteString filename)
{
	if (!(filename.EndsWith(".cps") || filename.EndsWith(".stm")))
	{
		new ErrorMessage("Error loading save", "Dropped file is not a TPT save file (.cps or .stm format)");
		return;
	}

	SaveFile *saveFile = Client::Ref().LoadSaveFile(filename);
	if (!saveFile)
		return;
	if (saveFile->GetError().length())
	{
		new ErrorMessage("Error loading save", "Dropped save file could not be loaded: " + saveFile->GetError());
		return;
	}
	c->LoadSaveFile(saveFile);
	delete saveFile;

	// hide the info text if it's not already hidden
	introText = 0;
}

void GameView::OnTick(float dt)
{
	if (selectMode == PlaceSave && !placeSaveThumb)
		selectMode = SelectNone;
	if (zoomEnabled && !zoomCursorFixed)
		c->SetZoomPosition(currentMouse);

	if (skipDraw)
	{
		skipDraw = false;
	}
	else if (selectMode == SelectNone && isMouseDown)
	{
		if (drawMode == DrawPoints)
		{
			c->DrawPoints(toolIndex, lastPoint, currentPoint, true);
			lastPoint = currentPoint;
		}
		else if (drawMode == DrawFill)
		{
			c->DrawFill(toolIndex, c->PointTranslate(currentMouse));
		}
		else if (windTool && drawMode == DrawLine)
		{
			ui::Point drawPoint2 = currentMouse;
			if (altBehaviour)
				drawPoint2 = lineSnapCoords(c->PointTranslate(drawPoint1), currentMouse);
			c->DrawLine(toolIndex, c->PointTranslate(drawPoint1), drawPoint2);
		}
	}

	int foundSignID = c->GetSignAt(mousePosition.X, mousePosition.Y);
	if (foundSignID != -1)
	{
		String str = c->GetSignText(foundSignID);
		auto si = c->GetSignSplit(foundSignID);

		StringBuilder tooltip;
		switch (si.second)
		{
		case sign::Type::Save:
			tooltip << "Go to save ID:" << str.Substr(3, si.first - 3);
			break;
		case sign::Type::Thread:
			tooltip << "Open forum thread " << str.Substr(3, si.first - 3) << " in browser";
			break;
		case sign::Type::Search:
			tooltip << "Search for " << str.Substr(3, si.first - 3);
			break;
		default: break;
		}

		if (tooltip.Size())
		{
			ToolTip(ui::Point(0, Size.Y), tooltip.Build());
		}
	}

	if(introText)
	{
		introText -= int(dt)>0?(int(dt) < 5? int(dt):5):1;
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
}


void GameView::DoMouseMove(int x, int y, int dx, int dy)
{
	if(c->MouseMove(x, y, dx, dy))
		Window::DoMouseMove(x, y, dx, dy);
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
	if(c->MouseUp(x, y, button, GameController::mouseUpNormal))
		Window::DoMouseUp(x, y, button);
}

void GameView::DoMouseWheel(int x, int y, int d)
{
	if(c->MouseWheel(x, y, d))
		Window::DoMouseWheel(x, y, d);
}

void GameView::DoTextInput(String text)
{
	if (c->TextInput(text))
		Window::DoTextInput(text);
}

void GameView::DoTextEditing(String text)
{
	if (c->TextEditing(text))
		Window::DoTextEditing(text);
}

void GameView::DoKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (shift && !shiftBehaviour)
		enableShiftBehaviour();
	if (ctrl && !ctrlBehaviour)
		enableCtrlBehaviour();
	if (alt && !altBehaviour)
		enableAltBehaviour();
	if (c->KeyPress(key, scan, repeat, shift, ctrl, alt))
		Window::DoKeyPress(key, scan, repeat, shift, ctrl, alt);
}

void GameView::DoKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (!shift && shiftBehaviour)
		disableShiftBehaviour();
	if (!ctrl && ctrlBehaviour)
		disableCtrlBehaviour();
	if (!alt && altBehaviour)
		disableAltBehaviour();
	if (c->KeyRelease(key, scan, repeat, shift, ctrl, alt))
		Window::DoKeyRelease(key, scan, repeat, shift, ctrl, alt);
}

void GameView::DoExit()
{
	Window::DoExit();
	c->Exit();
}

void GameView::DoDraw()
{
	Window::DoDraw();
	constexpr std::array<int, 9> fadeout = { { // * Gamma-corrected.
		255, 195, 145, 103, 69, 42, 23, 10, 3
	} };
	auto *g = GetGraphics();
	for (auto x = 0U; x < fadeout.size(); ++x)
	{
		g->draw_line(x, YRES + 1, x, YRES + 18, 0, 0, 0, fadeout[x]);
		g->draw_line(XRES - x, YRES + 1, XRES - x, YRES + 18, 0, 0, 0, fadeout[x]);
	}

	c->Tick();
	{
		int x = 0;
		int y = 0;
		int w = WINDOWW;
		int h = WINDOWH;
		g->SetClipRect(x, y, w, h); // reset any nonsense cliprect Lua left configured
	}
}

void GameView::NotifyNotificationsChanged(GameModel * sender)
{
	for (auto *notificationComponent : notificationComponents)
	{
		RemoveComponent(notificationComponent);
		delete notificationComponent;
	}
	notificationComponents.clear();

	std::vector<Notification*> notifications = sender->GetNotifications();

	int currentY = YRES-23;
	for (auto *notification : notifications)
	{
		int width = (Graphics::textwidth(notification->Message))+8;
		ui::Button * tempButton = new ui::Button(ui::Point(XRES-width-22, currentY), ui::Point(width, 15), notification->Message);
		tempButton->SetActionCallback({ [notification] {
			notification->Action();
		} });
		tempButton->Appearance.BorderInactive = style::Colour::WarningTitle;
		tempButton->Appearance.TextInactive = style::Colour::WarningTitle;
		tempButton->Appearance.BorderHover = ui::Colour(255, 175, 0);
		tempButton->Appearance.TextHover = ui::Colour(255, 175, 0);
		AddComponent(tempButton);
		notificationComponents.push_back(tempButton);

		tempButton = new ui::Button(ui::Point(XRES-20, currentY), ui::Point(15, 15), 0xE02A);
		//tempButton->SetIcon(IconClose);
		auto closeNotification = [this, notification] {
			c->RemoveNotification(notification);
		};
		tempButton->SetActionCallback({ closeNotification, closeNotification });
		tempButton->Appearance.Margin.Left -= 1;
		tempButton->Appearance.Margin.Top -= 1;
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

void GameView::NotifyLogChanged(GameModel * sender, String entry)
{
	logEntries.push_front(std::pair<String, int>(entry, 600));
	if (logEntries.size() > 20)
		logEntries.pop_back();
}

void GameView::NotifyPlaceSaveChanged(GameModel * sender)
{
	delete placeSaveThumb;
	placeSaveOffset = ui::Point(0, 0);
	if(sender->GetPlaceSave())
	{
		placeSaveThumb = SaveRenderer::Ref().Render(sender->GetPlaceSave(), true, true, sender->GetRenderer());
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
	if (!shiftBehaviour)
	{
		shiftBehaviour = true;
		if (!isMouseDown || selectMode != SelectNone)
			UpdateDrawMode();
		UpdateToolStrength();
	}
}

void GameView::disableShiftBehaviour()
{
	if (shiftBehaviour)
	{
		shiftBehaviour = false;
		if (!isMouseDown || selectMode != SelectNone)
			UpdateDrawMode();
		UpdateToolStrength();
	}
}

void GameView::enableAltBehaviour()
{
	if (!altBehaviour)
	{
		altBehaviour = true;
		drawSnap = true;
	}
}

void GameView::disableAltBehaviour()
{
	if (altBehaviour)
	{
		altBehaviour = false;
		drawSnap = false;
	}
}

void GameView::enableCtrlBehaviour()
{
	if (!ctrlBehaviour)
	{
		ctrlBehaviour = true;
		if (!isMouseDown || selectMode != SelectNone)
			UpdateDrawMode();
		UpdateToolStrength();

		//Show HDD save & load buttons
		saveSimulationButton->Appearance.BackgroundInactive = saveSimulationButton->Appearance.BackgroundHover = ui::Colour(255, 255, 255);
		saveSimulationButton->Appearance.TextInactive = saveSimulationButton->Appearance.TextHover = ui::Colour(0, 0, 0);

		saveSimulationButton->Enabled = true;
		SetSaveButtonTooltips();

		searchButton->Appearance.BackgroundInactive = searchButton->Appearance.BackgroundHover = ui::Colour(255, 255, 255);
		searchButton->Appearance.TextInactive = searchButton->Appearance.TextHover = ui::Colour(0, 0, 0);

		searchButton->SetToolTip("Open a simulation from your hard drive.");
		if (currentSaveType == 2)
			saveSimulationButton->SetShowSplit(true);
	}
}

void GameView::disableCtrlBehaviour()
{
	if (ctrlBehaviour)
	{
		ctrlBehaviour = false;
		if (!isMouseDown || selectMode != SelectNone)
			UpdateDrawMode();
		UpdateToolStrength();

		//Hide HDD save & load buttons
		saveSimulationButton->Appearance.BackgroundInactive = ui::Colour(0, 0, 0);
		saveSimulationButton->Appearance.BackgroundHover = ui::Colour(20, 20, 20);
		saveSimulationButton->Appearance.TextInactive = saveSimulationButton->Appearance.TextHover = ui::Colour(255, 255, 255);
		saveSimulationButton->Enabled = saveSimulationButtonEnabled && saveReuploadAllowed;
		SetSaveButtonTooltips();
		searchButton->Appearance.BackgroundInactive = ui::Colour(0, 0, 0);
		searchButton->Appearance.BackgroundHover = ui::Colour(20, 20, 20);
		searchButton->Appearance.TextInactive = searchButton->Appearance.TextHover = ui::Colour(255, 255, 255);
		searchButton->SetToolTip("Find & open a simulation. Hold Ctrl to load offline saves.");
		if (currentSaveType == 2)
			saveSimulationButton->SetShowSplit(false);
	}
}

void GameView::UpdateDrawMode()
{
	if (ctrlBehaviour && shiftBehaviour)
	{
		if (toolBrush)
			drawMode = DrawPoints;
		else
			drawMode = DrawFill;
	}
	else if (ctrlBehaviour)
		drawMode = DrawRect;
	else if (shiftBehaviour)
		drawMode = DrawLine;
	else
		drawMode = DrawPoints;
}

void GameView::UpdateToolStrength()
{
	if (shiftBehaviour)
		c->SetToolStrength(10.0f);
	else if (ctrlBehaviour)
		c->SetToolStrength(.1f);
	else
		c->SetToolStrength(1.0f);
}

void GameView::SetSaveButtonTooltips()
{
	if (!Client::Ref().GetAuthUser().UserID)
		saveSimulationButton->SetToolTips("Overwrite the open simulation on your hard drive.", "Save the simulation to your hard drive. Login to save online.");
	else if (ctrlBehaviour)
		saveSimulationButton->SetToolTips("Overwrite the open simulation on your hard drive.", "Save the simulation to your hard drive.");
	else if (saveSimulationButton->GetShowSplit())
		saveSimulationButton->SetToolTips("Re-upload the current simulation", "Modify simulation properties");
	else
		saveSimulationButton->SetToolTips("Re-upload the current simulation", "Upload a new simulation. Hold Ctrl to save offline.");
}

void GameView::OnDraw()
{
	Graphics * g = GetGraphics();
	if (ren)
	{
		ren->clearScreen(1.0f);
		ren->RenderBegin();
		ren->SetSample(c->PointTranslate(currentMouse).X, c->PointTranslate(currentMouse).Y);
		if (showBrush && selectMode == SelectNone && (!zoomEnabled || zoomCursorFixed) && activeBrush && (isMouseDown || (currentMouse.X >= 0 && currentMouse.X < XRES && currentMouse.Y >= 0 && currentMouse.Y < YRES)))
		{
			ui::Point finalCurrentMouse = c->PointTranslate(currentMouse);
			ui::Point initialDrawPoint = drawPoint1;

			if (wallBrush)
			{
				finalCurrentMouse = c->NormaliseBlockCoord(finalCurrentMouse);
				initialDrawPoint = c->NormaliseBlockCoord(initialDrawPoint);
			}

			if (drawMode == DrawRect && isMouseDown)
			{
				if (drawSnap)
				{
					finalCurrentMouse = rectSnapCoords(c->PointTranslate(initialDrawPoint), finalCurrentMouse);
				}
				if (wallBrush)
				{
					if (finalCurrentMouse.X > initialDrawPoint.X)
						finalCurrentMouse.X += CELL-1;
					else
						initialDrawPoint.X += CELL-1;

					if (finalCurrentMouse.Y > initialDrawPoint.Y)
						finalCurrentMouse.Y += CELL-1;
					else
						initialDrawPoint.Y += CELL-1;
				}
				activeBrush->RenderRect(ren, c->PointTranslate(initialDrawPoint), finalCurrentMouse);
			}
			else if (drawMode == DrawLine && isMouseDown)
			{
				if (drawSnap)
				{
					finalCurrentMouse = lineSnapCoords(c->PointTranslate(initialDrawPoint), finalCurrentMouse);
				}
				activeBrush->RenderLine(ren, c->PointTranslate(initialDrawPoint), finalCurrentMouse);
			}
			else if (drawMode == DrawFill)// || altBehaviour)
			{
				if (!decoBrush)
					activeBrush->RenderFill(ren, finalCurrentMouse);
			}
			if (drawMode == DrawPoints || drawMode==DrawLine || (drawMode == DrawRect && !isMouseDown))
			{
				if (wallBrush)
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
					int thumbX = selectPoint2.X - ((placeSaveThumb->Width-placeSaveOffset.X)/2) + CELL/2;
					int thumbY = selectPoint2.Y - ((placeSaveThumb->Height-placeSaveOffset.Y)/2) + CELL/2;

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

		if (doScreenshot)
		{
			doScreenshot = false;
			TakeScreenshot(0, 0);
		}

		if(recording)
		{
			VideoBuffer screenshot(ren->DumpFrame());
			std::vector<char> data = format::VideoBufferToPPM(screenshot);

			ByteString filename = ByteString::Build("recordings", PATH_SEP, recordingFolder, PATH_SEP, "frame_", Format::Width(recordingIndex++, 6), ".ppm");

			Platform::WriteFile(data, filename);
		}

		if (logEntries.size())
		{
			int startX = 20;
			int startY = YRES-20;
			std::deque<std::pair<String, int> >::iterator iter;
			for(iter = logEntries.begin(); iter != logEntries.end(); iter++)
			{
				String message = (*iter).first;
				int alpha = std::min((*iter).second, 255);
				if (alpha <= 0) //erase this and everything older
				{
					logEntries.erase(iter, logEntries.end());
					break;
				}
				startY -= 14;
				g->fillrect(startX-3, startY-3, Graphics::textwidth(message)+6, 14, 0, 0, 0, std::min(100, alpha));
				g->drawtext(startX, startY, message, 255, 255, 255, alpha);
				(*iter).second -= 3;
			}
		}
	}

	if (recording)
	{
		String sampleInfo = String::Build("#", screenshotIndex, " ", String(0xE00E), " REC");

		int textWidth = Graphics::textwidth(sampleInfo);
		g->fillrect(XRES-20-textWidth, 12, textWidth+8, 15, 0, 0, 0, 127);
		g->drawtext(XRES-16-textWidth, 16, sampleInfo, 255, 50, 20, 255);
	}
	else if(showHud)
	{
		//Draw info about simulation under cursor
		int wavelengthGfx = 0, alpha = 255;
		if (toolTipPosition.Y < 120)
			alpha = 255-toolTipPresence*3;
		if (alpha < 50)
			alpha = 50;
		StringBuilder sampleInfo;
		sampleInfo << Format::Precision(2);

		int type = sample.particle.type;
		if (type)
		{
			int ctype = sample.particle.ctype;

			if (type == PT_PHOT || type == PT_BIZR || type == PT_BIZRG || type == PT_BIZRS || type == PT_FILT || type == PT_BRAY || type == PT_C5)
				wavelengthGfx = (ctype&0x3FFFFFFF);

			if (showDebug)
			{
				if (type == PT_LAVA && c->IsValidElement(ctype))
				{
					sampleInfo << "Molten " << c->ElementResolve(ctype, 0);
				}
				else if ((type == PT_PIPE || type == PT_PPIP) && c->IsValidElement(ctype))
				{
					if (ctype == PT_LAVA && c->IsValidElement(sample.particle.tmp4))
					{
						sampleInfo << c->ElementResolve(type, 0) << " with molten " << c->ElementResolve(sample.particle.tmp4, -1);
					}
					else
					{
						sampleInfo << c->ElementResolve(type, 0) << " with " << c->ElementResolve(ctype, sample.particle.tmp4);
					}
				}
				else if (type == PT_LIFE)
				{
					sampleInfo << c->ElementResolve(type, ctype);
				}
				else if (type == PT_FILT)
				{
					sampleInfo << c->ElementResolve(type, ctype);
					String filtModes[] = {"set colour", "AND", "OR", "subtract colour", "red shift", "blue shift", "no effect", "XOR", "NOT", "old QRTZ scattering", "variable red shift", "variable blue shift"};
					if (sample.particle.tmp>=0 && sample.particle.tmp<=11)
						sampleInfo << " (" << filtModes[sample.particle.tmp] << ")";
					else
						sampleInfo << " (unknown mode)";
				}
				else
				{
					sampleInfo << c->ElementResolve(type, ctype);
					if (wavelengthGfx || type == PT_EMBR || type == PT_PRTI || type == PT_PRTO)
					{
						// Do nothing, ctype is meaningless for these elements
					}
					// Some elements store extra LIFE info in upper bits of ctype, instead of tmp/tmp2
					else if (type == PT_CRAY || type == PT_DRAY || type == PT_CONV || type == PT_LDTC)
						sampleInfo << " (" << c->ElementResolve(TYP(ctype), ID(ctype)) << ")";
					else if (type == PT_CLNE || type == PT_BCLN || type == PT_PCLN || type == PT_PBCN || type == PT_DTEC)
						sampleInfo << " (" << c->ElementResolve(ctype, sample.particle.tmp) << ")";
					else if (c->IsValidElement(ctype) && type != PT_GLOW && type != PT_WIRE && type != PT_SOAP && type != PT_LITH)
						sampleInfo << " (" << c->ElementResolve(ctype, 0) << ")";
					else if (ctype)
						sampleInfo << " (" << ctype << ")";
				}
				sampleInfo << ", Temp: " << (sample.particle.temp - 273.15f) << " C";
				sampleInfo << ", Life: " << sample.particle.life;
				if (sample.particle.type != PT_RFRG && sample.particle.type != PT_RFGL && sample.particle.type != PT_LIFE)
				{
					if (sample.particle.type == PT_CONV)
					{
						String elemName = c->ElementResolve(
							TYP(sample.particle.tmp),
							ID(sample.particle.tmp));
						if (elemName == "")
							sampleInfo << ", Tmp: " << sample.particle.tmp;
						else
							sampleInfo << ", Tmp: " << elemName;
					}
					else
						sampleInfo << ", Tmp: " << sample.particle.tmp;
				}

				// only elements that use .tmp2 show it in the debug HUD
				if (type == PT_CRAY || type == PT_DRAY || type == PT_EXOT || type == PT_LIGH || type == PT_SOAP || type == PT_TRON
						|| type == PT_VIBR || type == PT_VIRS || type == PT_WARP || type == PT_LCRY || type == PT_CBNW || type == PT_TSNS
						|| type == PT_DTEC || type == PT_LSNS || type == PT_PSTN || type == PT_LDTC || type == PT_VSNS || type == PT_LITH
						|| type == PT_CONV)
					sampleInfo << ", Tmp2: " << sample.particle.tmp2;

				sampleInfo << ", Pressure: " << sample.AirPressure;
			}
			else
			{
				sampleInfo << c->BasicParticleInfo(sample.particle);
				sampleInfo << ", Temp: " << sample.particle.temp - 273.15f << " C";
				sampleInfo << ", Pressure: " << sample.AirPressure;
			}
		}
		else if (sample.WallType)
		{
			sampleInfo << c->WallName(sample.WallType);
			sampleInfo << ", Pressure: " << sample.AirPressure;
		}
		else if (sample.isMouseInSim)
		{
			sampleInfo << "Empty, Pressure: " << sample.AirPressure;
		}
		else
		{
			sampleInfo << "Empty";
		}

		int textWidth = Graphics::textwidth(sampleInfo.Build());
		g->fillrect(XRES-20-textWidth, 12, textWidth+8, 15, 0, 0, 0, int(alpha*0.5f));
		g->drawtext(XRES-16-textWidth, 16, sampleInfo.Build(), 255, 255, 255, int(alpha*0.75f));

		if (wavelengthGfx)
		{
			int i, cr, cg, cb, j, h = 3, x = XRES-19-textWidth, y = 10;
			int tmp;
			g->fillrect(x, y, 30, h, 64, 64, 64, alpha); // coords -1 size +1 to work around bug in fillrect - TODO: fix fillrect
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
						g->blendpixel(x+29-i, y+j, cr>255?255:cr, cg>255?255:cg, cb>255?255:cb, alpha);
				}
			}
		}

		if (showDebug)
		{
			StringBuilder sampleInfo;
			sampleInfo << Format::Precision(2);

			if (type)
				sampleInfo << "#" << sample.ParticleID << ", ";

			sampleInfo << "X:" << sample.PositionX << " Y:" << sample.PositionY;

			if (sample.Gravity)
				sampleInfo << ", GX: " << sample.GravityVelocityX << " GY: " << sample.GravityVelocityY;

			if (c->GetAHeatEnable())
				sampleInfo << ", AHeat: " << sample.AirTemperature - 273.15f << " C";

			textWidth = Graphics::textwidth(sampleInfo.Build());
			g->fillrect(XRES-20-textWidth, 27, textWidth+8, 14, 0, 0, 0, int(alpha*0.5f));
			g->drawtext(XRES-16-textWidth, 30, sampleInfo.Build(), 255, 255, 255, int(alpha*0.75f));
		}
	}

	if(showHud && introText < 51)
	{
		//FPS and some version info
		StringBuilder fpsInfo;
		fpsInfo << Format::Precision(2) << "FPS: " << ui::Engine::Ref().GetFps();

		if (showDebug)
		{
			if (ren->findingElement)
				fpsInfo << " Parts: " << ren->foundElements << "/" << sample.NumParts;
			else
				fpsInfo << " Parts: " << sample.NumParts;
		}
		if (c->GetReplaceModeFlags()&REPLACE_MODE)
			fpsInfo << " [REPLACE MODE]";
		if (c->GetReplaceModeFlags()&SPECIFIC_DELETE)
			fpsInfo << " [SPECIFIC DELETE]";
		if (ren && ren->GetGridSize())
			fpsInfo << " [GRID: " << ren->GetGridSize() << "]";
		if (ren && ren->findingElement)
			fpsInfo << " [FIND]";

		int textWidth = Graphics::textwidth(fpsInfo.Build());
		int alpha = 255-introText*5;
		g->fillrect(12, 12, textWidth+8, 15, 0, 0, 0, int(alpha*0.5));
		g->drawtext(16, 16, fpsInfo.Build(), 32, 216, 255, int(alpha*0.75));
	}

	//Tooltips
	if(infoTipPresence)
	{
		int infoTipAlpha = (infoTipPresence>50?50:infoTipPresence)*5;
		g->drawtext_outline((XRES-Graphics::textwidth(infoTip))/2, (YRES/2)-2, infoTip, 255, 255, 255, infoTipAlpha);
	}

	if(toolTipPresence && toolTipPosition.X!=-1 && toolTipPosition.Y!=-1 && toolTip.length())
	{
		if (toolTipPosition.Y == Size.Y-MENUSIZE-10)
			g->drawtext_outline(toolTipPosition.X, toolTipPosition.Y, toolTip, 255, 255, 255, toolTipPresence>51?255:toolTipPresence*5);
		else
			g->drawtext(toolTipPosition.X, toolTipPosition.Y, toolTip, 255, 255, 255, toolTipPresence>51?255:toolTipPresence*5);
	}

	if(buttonTipShow > 0)
	{
		g->drawtext(16, Size.Y-MENUSIZE-24, buttonTip, 255, 255, 255, buttonTipShow>51?255:buttonTipShow*5);
	}

	//Introduction text
	if(introText && showHud)
	{
		g->fillrect(0, 0, WINDOWW, WINDOWH, 0, 0, 0, introText>51?102:introText*2);
		g->drawtext(16, 20, introTextMessage, 255, 255, 255, introText>51?255:introText*5);
	}

	// Clear menu areas, to ensure particle graphics don't overlap
	memset(g->vid+((XRES+BARSIZE)*YRES), 0, (PIXELSIZE*(XRES+BARSIZE))*MENUSIZE);
	g->clearrect(XRES, 1, BARSIZE, YRES-1);
}

ui::Point GameView::lineSnapCoords(ui::Point point1, ui::Point point2)
{
	ui::Point diff = point2 - point1;
	if(abs(diff.X / 2) > abs(diff.Y)) // vertical
		return point1 + ui::Point(diff.X, 0);
	if(abs(diff.X) < abs(diff.Y / 2)) // horizontal
		return point1 + ui::Point(0, diff.Y);
	if(diff.X * diff.Y > 0) // NW-SE
		return point1 + ui::Point((diff.X + diff.Y)/2, (diff.X + diff.Y)/2);
	// SW-NE
	return point1 + ui::Point((diff.X - diff.Y)/2, (diff.Y - diff.X)/2);
}

ui::Point GameView::rectSnapCoords(ui::Point point1, ui::Point point2)
{
	ui::Point diff = point2 - point1;
	if(diff.X * diff.Y > 0) // NW-SE
		return point1 + ui::Point((diff.X + diff.Y)/2, (diff.X + diff.Y)/2);
	// SW-NE
	return point1 + ui::Point((diff.X - diff.Y)/2, (diff.Y - diff.X)/2);
}
