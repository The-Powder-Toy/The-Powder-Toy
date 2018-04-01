#include <map>
#include "Redeem.h"
#include "Format.h"
#include "client/Client.h"
#include "graphics/Graphics.h"
#include "simulation/Simulation.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/game/Tool.h"
#include "gui/game/Menu.h"
#include "gui/game/ToolButton.h"
#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"

class PackRedeem : public ui::ButtonAction
{
	std::vector<Tool*> toolList;
	int price;
	Simulation * sim;

public:
	PackRedeem(std::vector<Tool*> toolList, int price, Simulation *sim):
		toolList(toolList),
		price(price),
		sim(sim)
	{

	}

	void ActionCallback(ui::Button * sender_)
	{
		int numCoins = Client::Ref().GetCoins();
		std::string unlockPrice = format::NumberToString<int>(price);
		if (numCoins < price)
		{
			new ErrorMessage("Cannot afford pack", "This pack requires " + unlockPrice
							 + " \xEA""owdercoins to purchase.\n\nYou have " + format::NumberToString<int>(numCoins) + " coins");
		}
		else
		{
			bool buy = ConfirmPrompt::Blocking("Buy element pack?", "Buy this element for " + unlockPrice + " \xEA"
										  "owdercoins? You will have " + format::NumberToString<int>(numCoins - price) + " \xEA"
										  "owdercoins after this transaction.");
			if (buy)
			{
				Client::Ref().AddCoins(-price);
				for (Tool * tool : toolList)
				{
					int toolId = tool->GetToolID();
					if (toolId > 0 && toolId < PT_NUM)
						sim->elements[toolId].Unlocked = true;
				}
			}
		}
		sender_->GetParentWindow()->CloseActiveWindow();
		sender_->GetParentWindow()->SelfDestruct();
	}
};

Redeem::Redeem(std::vector<Menu*> menus, Simulation *simulation):
	ui::Window(ui::Point(-1, -1), ui::Point(300, 300)),
	menus(menus),
	simulation(simulation)
{
	int y = 10;
	std::vector<int> menuPacks = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	for (int pack : menuPacks)
	{
		Tool * tool = nullptr;
		int count = 0;
		int price = 0;
		for (Tool *menuTool : menus[pack]->GetToolList())
			if (!menuTool->unlocked)
			{
				if (!count++)
					tool = menuTool;
				price += menuTool->unlockPrice;
			}
		if (!tool)
			continue;

		std::string desc = menus[pack]->GetDescription() + " Pack: ";
		int textSize = Graphics::textwidth(desc.c_str());
		ui::Label * label = new ui::Label(ui::Point(20, y), ui::Point(textSize, 16), desc);
		AddComponent(label);

		ToolButton * tempButton = new ToolButton(ui::Point(20 + textSize, y), ui::Point(30, 18), tool->GetName(), tool->GetIdentifier(), tool->GetDescription());
		tempButton->Appearance.BackgroundInactive = ui::Colour(tool->colRed, tool->colGreen, tool->colBlue);
		// 1.1 times normal price for the added convenience of buying all the elements at once
		tempButton->SetActionCallback(new PackRedeem(menus[pack]->GetToolList(), price * 1.1, simulation));
		tempButton->SetUnlocked(tool->unlocked);
		AddComponent(tempButton);

		if (count > 1)
		{
			desc = " and " + format::NumberToString<int>(count-1) + " others";
			int textSize2 = Graphics::textwidth(desc.c_str());
			label = new ui::Label(ui::Point(20 + textSize + 30, y), ui::Point(textSize2, 16), desc);
			AddComponent(label);
		}

		y += 25;
	}
}

void Redeem::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

void Redeem::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}
