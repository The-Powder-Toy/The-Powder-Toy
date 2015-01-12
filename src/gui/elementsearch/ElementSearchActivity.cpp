#include <algorithm>
#include "ElementSearchActivity.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Label.h"
#include "gui/interface/Keys.h"
#include "gui/game/Tool.h"
#include "gui/Style.h"
#include "gui/game/GameController.h"

class ElementSearchActivity::ToolAction: public ui::ButtonAction
{
	ElementSearchActivity * a;
public:
	Tool * tool;
	ToolAction(ElementSearchActivity * a, Tool * tool) : a(a), tool(tool) {  }
	void ActionCallback(ui::Button * sender_)
	{
		ToolButton *sender = (ToolButton*)sender_;
		if(sender->GetSelectionState() >= 0 && sender->GetSelectionState() <= 2)
			a->SetActiveTool(sender->GetSelectionState(), tool);
	}
};

ElementSearchActivity::ElementSearchActivity(GameController * gameController, std::vector<Tool*> tools) :
	WindowActivity(ui::Point(-1, -1), ui::Point(236, 302)),
	gameController(gameController),
	tools(tools),
	firstResult(NULL),
	exit(false)
{
	ui::Label * title = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 15), "Element Search");
	title->SetTextColour(style::Colour::InformationTitle);
	title->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(title);

	class SearchAction : public ui::TextboxAction
	{
	private:
		ElementSearchActivity * a;
	public:
		SearchAction(ElementSearchActivity * a) : a(a) {}
		virtual void TextChangedCallback(ui::Textbox * sender) {
			a->searchTools(sender->GetText());
		}
	};

	searchField = new ui::Textbox(ui::Point(8, 23), ui::Point(Size.X-16, 17), "");
	searchField->SetActionCallback(new SearchAction(this));
	searchField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(searchField);
	FocusComponent(searchField);

	class CloseAction: public ui::ButtonAction
		{
			ElementSearchActivity * a;
		public:
			CloseAction(ElementSearchActivity * a) : a(a) {  }
			void ActionCallback(ui::Button * sender_)
			{
				a->exit = true;
			}
		};

	class OKAction: public ui::ButtonAction
		{
			ElementSearchActivity * a;
		public:
			OKAction(ElementSearchActivity * a) : a(a) {  }
			void ActionCallback(ui::Button * sender_)
			{
				if(a->GetFirstResult())
					a->SetActiveTool(0, a->GetFirstResult());
			}
		};

	ui::Button * closeButton = new ui::Button(ui::Point(0, Size.Y-15), ui::Point((Size.X/2)+1, 15), "Close");
	closeButton->SetActionCallback(new CloseAction(this));
	ui::Button * okButton = new ui::Button(ui::Point(Size.X/2, Size.Y-15), ui::Point(Size.X/2, 15), "OK");
	okButton->SetActionCallback(new OKAction(this));

	AddComponent(okButton);
	AddComponent(closeButton);

	searchTools("");
}

void ElementSearchActivity::searchTools(std::string query)
{
	firstResult = NULL;
	for(std::vector<ToolButton*>::iterator iter = toolButtons.begin(), end = toolButtons.end(); iter != end; ++iter) {
		delete *iter;
		RemoveComponent(*iter);
	}
	toolButtons.clear();

	ui::Point viewPosition = searchField->Position + ui::Point(2+0, searchField->Size.Y+2+8);
	ui::Point current = ui::Point(0, 0);

	std::string queryLower = std::string(query);
	std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);

	std::vector<Tool *> matches;
	std::vector<Tool *> frontmatches;
	std::vector<Tool *> exactmatches;

	for(std::vector<Tool*>::const_iterator iter = tools.begin(), end = tools.end(); iter != end; ++iter)
	{
		std::string nameLower = std::string((*iter)->GetName());
		std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
		if(!strcmp(nameLower.c_str(), queryLower.c_str()))
			exactmatches.push_back(*iter);
		else if(!strncmp(nameLower.c_str(), queryLower.c_str(), queryLower.length()))
			frontmatches.push_back(*iter);
		else if(strstr(nameLower.c_str(), queryLower.c_str()))
			matches.push_back(*iter);
	}

	matches.insert(matches.begin(), frontmatches.begin(), frontmatches.end());
	matches.insert(matches.begin(), exactmatches.begin(), exactmatches.end());

	for(std::vector<Tool*>::const_iterator iter = matches.begin(), end = matches.end(); iter != end; ++iter)
	{
		Tool * tool = *iter;

		if(!firstResult)
			firstResult = tool;

		VideoBuffer * tempTexture = tool->GetTexture(26, 14);
		ToolButton * tempButton;

		if(tempTexture)
			tempButton = new ToolButton(current+viewPosition, ui::Point(30, 18), "", tool->GetDescription());
		else
			tempButton = new ToolButton(current+viewPosition, ui::Point(30, 18), tool->GetName(), tool->GetDescription());

		tempButton->Appearance.SetTexture(tempTexture);
		tempButton->Appearance.BackgroundInactive = ui::Colour(tool->colRed, tool->colGreen, tool->colBlue);
		tempButton->SetActionCallback(new ToolAction(this, tool));

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
	gameController->SetActiveTool(selectionState, tool);
	exit = true;
}

void ElementSearchActivity::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	g->drawrect(Position.X+searchField->Position.X, Position.Y+searchField->Position.Y+searchField->Size.Y+8, searchField->Size.X, Size.Y-(searchField->Position.Y+searchField->Size.Y+8)-23, 255, 255, 255, 180);
}

void ElementSearchActivity::OnTick(float dt)
{
	if (exit)
		Exit();
}

void ElementSearchActivity::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(key == KEY_ENTER || key == KEY_RETURN)
	{
		if(firstResult)
			gameController->SetActiveTool(0, firstResult);
		exit = true;
	}
	if(key == KEY_ESCAPE)
	{
		exit = true;
	}
}

ElementSearchActivity::~ElementSearchActivity() {
}

