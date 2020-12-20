#include "ElementSearchActivity.h"

#include <set>
#include <map>
#include <algorithm>

#include "gui/interface/Textbox.h"
#include "gui/interface/Label.h"
#include "gui/interface/Keys.h"
#include "gui/game/Tool.h"
#include "gui/game/Menu.h"
#include "gui/Style.h"
#include "gui/game/Favorite.h"
#include "gui/game/GameController.h"
#include "gui/game/ToolButton.h"
#include "gui/game/Favorite.h"

#include "graphics/Graphics.h"

ElementSearchActivity::ElementSearchActivity(GameController * gameController, std::vector<Tool*> tools) :
	WindowActivity(ui::Point(-1, -1), ui::Point(236, 302)),
	firstResult(NULL),
	gameController(gameController),
	tools(tools),
	toolTip(""),
	toolTipPresence(0),
	shiftPressed(false),
	ctrlPressed(false),
	altPressed(false),
	isToolTipFadingIn(false),
	exit(false)
{
	ui::Label * title = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 15), "Element Search");
	title->SetTextColour(style::Colour::InformationTitle);
	title->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(title);

	searchField = new ui::Textbox(ui::Point(8, 23), ui::Point(Size.X-16, 17), "");
	searchField->SetActionCallback({ [this] { searchTools(searchField->GetText()); } });
	searchField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(searchField);
	FocusComponent(searchField);

	ui::Button * closeButton = new ui::Button(ui::Point(0, Size.Y-15), ui::Point((Size.X/2)+1, 15), "Close");
	closeButton->SetActionCallback({ [this] { exit = true; } });
	ui::Button * okButton = new ui::Button(ui::Point(Size.X/2, Size.Y-15), ui::Point(Size.X/2, 15), "OK");
	okButton->SetActionCallback({ [this] {
		if (GetFirstResult())
			SetActiveTool(0, GetFirstResult());
	} });

	AddComponent(okButton);
	AddComponent(closeButton);

	searchTools("");
}

void ElementSearchActivity::searchTools(String query)
{
	firstResult = NULL;
	for (auto &toolButton : toolButtons) {
		RemoveComponent(toolButton);
		delete toolButton;
	}
	toolButtons.clear();

	ui::Point viewPosition = searchField->Position + ui::Point(2+0, searchField->Size.Y+2+8);
	ui::Point current = ui::Point(0, 0);

	String queryLower = query.ToLower();

	struct Match
	{
		int favouritePriority; // relevant by whether the tool is favourited
		int toolIndex; // relevance by position of tool in tools vector
		int haystackOrigin; // relevance by origin of haystack
		int needlePosition; // relevance by position of needle in haystack

		bool operator <(Match const &other) const
		{
			return std::tie(favouritePriority, haystackOrigin, needlePosition, toolIndex) < std::tie(other.favouritePriority, other.haystackOrigin, other.needlePosition, other.toolIndex);
		}
	};

	std::set<ByteString> favs;
	for (auto fav : Favorite::Ref().GetFavoritesList())
	{
		favs.insert(fav);
	}

	std::map<int, Match> indexToMatch;
	auto push = [ &indexToMatch ](Match match) {
		auto it = indexToMatch.find(match.toolIndex);
		if (it == indexToMatch.end())
		{
			indexToMatch.insert(std::make_pair(match.toolIndex, match));
		}
		else if (match < it->second)
		{
			it->second = match;
		}
	};

	auto pushIfMatches = [ &queryLower, &push ](String infoLower, int toolIndex, int favouritePriority, int haystackRelevance) {
		if (infoLower == queryLower)
		{
			push(Match{ favouritePriority, toolIndex, haystackRelevance, 0 });
		}
		if (infoLower.BeginsWith(queryLower))
		{
			push(Match{ favouritePriority, toolIndex, haystackRelevance, 1 });
		}
		if (infoLower.Contains(queryLower))
		{
			push(Match{ favouritePriority, toolIndex, haystackRelevance, 2 });
		}
	};

	std::map<Tool *, String> menudescriptionLower;
	for (auto *menu : gameController->GetMenuList())
	{
		for (auto *tool : menu->GetToolList())
		{
			menudescriptionLower.insert(std::make_pair(tool, menu->GetDescription().ToLower()));
		}
	}

	for (int toolIndex = 0; toolIndex < (int)tools.size(); ++toolIndex)
	{
		int favouritePriority = favs.find(tools[toolIndex]->GetIdentifier()) != favs.end() ? 0 : 1;
		pushIfMatches(tools[toolIndex]->GetName().ToLower(), toolIndex, favouritePriority, 0);
		pushIfMatches(tools[toolIndex]->GetDescription().ToLower(), toolIndex, favouritePriority, 1);
		auto it = menudescriptionLower.find(tools[toolIndex]);
		if (it != menudescriptionLower.end())
		{
			pushIfMatches(it->second, toolIndex, favouritePriority, 2);
		}
	}

	std::vector<Match> matches;
	std::transform(indexToMatch.begin(), indexToMatch.end(), std::back_inserter(matches), [](decltype(indexToMatch)::value_type const &pair) {
		return pair.second;
	});
	std::sort(matches.begin(), matches.end());
	for (auto &match : matches)
	{
		Tool *tool = tools[match.toolIndex];

		if(!firstResult)
			firstResult = tool;

		VideoBuffer * tempTexture = tool->GetTexture(26, 14);
		ToolButton * tempButton;

		if(tempTexture)
			tempButton = new ToolButton(current+viewPosition, ui::Point(30, 18), "", tool->GetIdentifier(), tool->GetDescription());
		else
			tempButton = new ToolButton(current+viewPosition, ui::Point(30, 18), tool->GetName(), tool->GetIdentifier(), tool->GetDescription());

		tempButton->Appearance.SetTexture(tempTexture);
		tempButton->Appearance.BackgroundInactive = ui::Colour(tool->colRed, tool->colGreen, tool->colBlue);
		tempButton->SetActionCallback({ [this, tempButton, tool] {
			if (tempButton->GetSelectionState() >= 0 && tempButton->GetSelectionState() <= 2)
				SetActiveTool(tempButton->GetSelectionState(), tool);
		} });

		if(gameController->GetActiveTool(0) == tool)
		{
			tempButton->SetSelectionState(0);	//Primary
		}
		else if(gameController->GetActiveTool(1) == tool)
		{
			tempButton->SetSelectionState(1);	//Secondary
		}
		else if(gameController->GetActiveTool(2) == tool)
		{
			tempButton->SetSelectionState(2);	//Tertiary
		}

		toolButtons.push_back(tempButton);
		AddComponent(tempButton);

		current.X += 31;

		if(current.X + 30 > searchField->Size.X) {
			current.X = 0;
			current.Y += 19;
		}

		if(current.Y + viewPosition.Y + 18 > Size.Y-23)
			break;
	}
}

void ElementSearchActivity::SetActiveTool(int selectionState, Tool * tool)
{
	if (ctrlPressed && shiftPressed && !altPressed)
	{
		Favorite::Ref().AddFavorite(tool->GetIdentifier());
		gameController->RebuildFavoritesMenu();
	}
	else if (ctrlPressed && altPressed && !shiftPressed &&
	         tool->GetIdentifier().Contains("DEFAULT_PT_"))
	{
		gameController->SetActiveTool(3, tool);
	}
	else
		gameController->SetActiveTool(selectionState, tool);
	exit = true;
}

void ElementSearchActivity::OnDraw()
{
	Graphics * g = GetGraphics();
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	g->drawrect(Position.X+searchField->Position.X, Position.Y+searchField->Position.Y+searchField->Size.Y+8, searchField->Size.X, Size.Y-(searchField->Position.Y+searchField->Size.Y+8)-23, 255, 255, 255, 180);
	if (toolTipPresence && toolTip.length())
	{
		g->drawtext(10, Size.Y+70, toolTip, 255, 255, 255, toolTipPresence>51?255:toolTipPresence*5);
	}
}

void ElementSearchActivity::OnTick(float dt)
{
	if (exit)
		Exit();
	if (isToolTipFadingIn)
	{
		isToolTipFadingIn = false;
		if (toolTipPresence < 120)
			toolTipPresence += int(dt*2)>1?int(dt*2):2;
	}
	else if (toolTipPresence>0)
	{
		toolTipPresence -= int(dt)>0?int(dt):1;
		if (toolTipPresence<0)
			toolTipPresence = 0;
	}
}

void ElementSearchActivity::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	switch (key)
	{
	case SDLK_KP_ENTER:
	case SDLK_RETURN:
		if(firstResult)
			gameController->SetActiveTool(0, firstResult);
	case SDLK_ESCAPE:
		exit = true;
		break;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		shiftPressed = true;
		break;
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		ctrlPressed = true;
		break;
	case SDLK_LALT:
	case SDLK_RALT:
		altPressed = true;
		break;
	}
}

void ElementSearchActivity::OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	switch (key)
	{
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		shiftPressed = false;
		break;
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		ctrlPressed = false;
		break;
	case SDLK_LALT:
	case SDLK_RALT:
		altPressed = false;
		break;
	}
}

void ElementSearchActivity::ToolTip(ui::Point senderPosition, String toolTip)
{
	this->toolTip = toolTip;
	this->isToolTipFadingIn = true;
}

ElementSearchActivity::~ElementSearchActivity() {
}

