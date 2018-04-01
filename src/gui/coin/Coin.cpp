#include <sstream>
#include <vector>
#include "Coin.h"
#include "CardInput.h"
#include "Format.h"
#include "Platform.h"
#include "Redeem.h"
#include "client/Client.h"
#include "client/Download.h"
#include "graphics/Graphics.h"
#include "simulation/Simulation.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/TextPrompt.h"
#include "gui/game/GameModel.h"
#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"

Coin::Coin(GameModel *gameModel):
	ui::Window(ui::Point(-1, -1), ui::Point(500, 400)),
	gameModel(gameModel)
{
	std::string labelText = "TPT has now gone free to play! Many elements are unlocked by default, but by using \brNEW\bw \xEA""owdercoins you "
							"can unlock new elements and features never seen before. No need to get out your wallet though - \boyou can earn "
							"coins just by playing TPT!\n\n"
							"\bwWhy are you doing this?\n"
							"\bgTimes have been hard recently - server costs are increasing due to the rise in bitcoin mining. "
							"The tariffs placed by the US government on foreign explosives makers has also affected us significantly.\n"
							"We also incurred large costs paying for the funeral of our server admin, Simon.\n\n"
							"\bwWhat will the money go towards?\n"
							"\bgThe money will go towards paying for new and upgraded servers, advertising, and entertainment expenses for jacob1 "
							"and the other admins to keep us productive.";

	//int textsize = Graphics::textwidth(labelText.c_str());
	int height = Graphics::textwrapheight((char*)labelText.c_str(), 400);
	ui::Label *label = new ui::Label(ui::Point(10, 15), ui::Point(400, height), labelText);
	label->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	label->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	label->SetMultiline(true);
	AddComponent(label);

	labelText = "\bo\xEA""owdercoins: " + format::NumberToString<int>(Client::Ref().GetCoins());
	int textsize = Graphics::textwidth(labelText.c_str());
	powderCoins = new ui::Label(ui::Point(450 - textsize/2, 15), ui::Point(textsize, 16), labelText);
	AddComponent(powderCoins);

	class BuyCoinsAction: public ui::ButtonAction
	{
		Coin * coin;
		int chance;
		int amount;
	public:
		BuyCoinsAction(Coin * coin, int chance, int amount) { this->coin = coin; this->chance = chance; this->amount = amount; }
		void ActionCallback(ui::Button * sender)
		{
			//ConfirmPrompt::Blocking("Agree to ToS?", "test message asdf", "Confirm", true);
			CardInput *card = new CardInput(chance, amount);
			ui::Engine::Ref().ShowWindow(card);
		}
	};

	struct coinData
	{
		int amount;
		int chance;
		std::string title;
		//std::string description;

		coinData(int amount, int chance, std::string title):
			amount(amount),
			chance(chance),
			title(title)
		{

		}
	};
	std::vector<coinData> dataList = std::vector<coinData>();
	dataList.push_back(coinData(100, 50, "$0.99 - basic package"));
	dataList.push_back(coinData(200, 40, "$1.94 - save 2%"));
	dataList.push_back(coinData(500, 30, "$4.74 - save 5%"));
	dataList.push_back(coinData(1000, 20, "$10.99 - save 10% - BEST DEAL!"));
	dataList.push_back(coinData(1500, 10, "$13.99 - ultra package"));
	dataList.push_back(coinData(2500, 5, "$23.49 - used, 60,000 miles, no accidents"));
	dataList.push_back(coinData(5000, 0, "$49.99 - 50 basic packages included!"));
	dataList.push_back(coinData(9999, 0, "$94.99 - All proceeds from this option go to saving the whales"));

	ui::Button *button;

	int x = 10;
	int y = 200;
	for (coinData data : dataList)
	{
		std::stringstream buttonText;
		buttonText << "\xEA";
		buttonText << data.amount;
		button = new ui::Button(ui::Point(x, y), ui::Point(40, 25), buttonText.str());
		button->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		button->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
		button->SetActionCallback(new BuyCoinsAction(this, data.chance, data.amount));
		AddComponent(button);

		label = new ui::Label(ui::Point(x + 45, y+2), ui::Point(200, 16), data.title);
		label->SetMultiline(true);
		label->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		label->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
		AddComponent(label);

		if (x == 10)
			x = 250;
		else
		{
			x = 10;
			y += 35;
		}
	}

	class OpenLinkAction : public ui::ButtonAction
	{
		std::string uri;
	public:
		OpenLinkAction(std::string uri) { this->uri = uri; }
		void ActionCallback(ui::Button * sender)
		{
			Platform::OpenURI(uri);
		}
	};

	button = new ui::Button(ui::Point(20, 375), ui::Point(90, 20), "\x0F\x14\xA0\xFE\xEB\x0E @PowderToy");
	button->SetActionCallback(new OpenLinkAction("https://twitter.com/PowderToy"));
	AddComponent(button);

	button = new ui::Button(ui::Point(130, 375), ui::Point(90, 20), "\bb\xEC\bw PowderToy");
	button->SetActionCallback(new OpenLinkAction("https://www.facebook.com/PowderToy"));
	AddComponent(button);

	button = new ui::Button(ui::Point(240, 375), ui::Point(90, 20), "\x0F\xFE\x47\x1A\xED\x0E PowderToy");
	button->SetActionCallback(new OpenLinkAction("https://www.youtube.com/watch?v=lcZwzlb2O_4"));
	AddComponent(button);

	button = new ui::Button(ui::Point(350, 375), ui::Point(90, 20), "\x0F\xFE\x72\x1E\xEF\x0E /r/powdertoy");
	button->SetActionCallback(new OpenLinkAction("https://www.reddit.com/r/birdswitharms/"));
	AddComponent(button);


	class RedeemAction : public ui::ButtonAction
	{
		Simulation *simulation;
		std::vector<Menu*> menus;
	public:
		RedeemAction(std::vector<Menu*> menus, Simulation *simulation) { this->menus = menus; this->simulation = simulation; }
		void ActionCallback(ui::Button * sender)
		{
			Redeem *redeem = new Redeem(menus, simulation);
			ui::Engine::Ref().ShowWindow(redeem);
		}
	};

	button = new ui::Button(ui::Point(400, 49), ui::Point(100, 50), "REDEEM >");
	button->SetActionCallback(new RedeemAction(gameModel->GetMenuList(), gameModel->GetSimulation()));
	AddComponent(button);

	class CodeAction : public ui::ButtonAction
	{
	public:
		void ActionCallback(ui::Button * sender)
		{
			std::string code = TextPrompt::Blocking("Enter Code", "Enter a valid code to receive extra \xEA""owderCoins and other prizes", "", "", false);
			if (code.find(' ') != code.npos || code.find('&') != code.npos)
			{
				new ErrorMessage("Invalid code", "Code cannot contain spaces");
				return;
			}

			std::stringstream codeUrl;
			codeUrl << "http://aprilfools.starcatcher.us/Moo/AprilFool.lua";
			codeUrl << "?username=" << Client::Ref().GetAuthUser().Username;
			codeUrl << "&code=" << code;
			Download *download = new Download(codeUrl.str());
			// Lua on server side doesn't understand this format and I don't have the time to figure out how to make it understand this
			/*std::map<std::string, std::string> postData = {
				{"username", Client::Ref().GetAuthUser().Username},
				{"code", code}
			};*/
			//download->AddPostData(postData);
			download->Start();
			int length, status;
			char *data = download->Finish(&length, &status);
			if (status != 200)
			{
				new ErrorMessage("Server error", "Recieved http error code " + format::NumberToString<int>(status));
				return;
			}
			if (length == 0)
			{
				new ErrorMessage("Invalid code", "This code is not valid");
				return;
			}

			char *data2 = new char[length+1];
			strncpy(data2, data, length);
			data2[length] = 0;

			int amount = format::StringToNumber<int>(data2);
			if (amount == 0)
			{
				new ErrorMessage("Server error", "Recieved message from server: " + std::string(data2));
				return;
			}
			Client::Ref().AddCoins(amount);
			new InformationMessage("Received Coins!", "You received " + std::string(data2) + " coins! New balance: " +
								   format::NumberToString<int>(Client::Ref().GetCoins()) + " \xEA""owdercoins", false);
			//std::cout << data2 << std::endl;
		}
	};

	button = new ui::Button(ui::Point(400, 98), ui::Point(100, 50), "CODE >");
	button->SetActionCallback(new CodeAction());
	AddComponent(button);
}

void Coin::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);

	g->drawrect(Position.X+400, Position.Y, Size.X-400, 50, 200, 200, 200, 255);
}

void Coin::OnTick(float dt)
{
	std::string text = "\bo\xEA""owdercoins: " + format::NumberToString<int>(Client::Ref().GetCoins());
	powderCoins->SetText(text);
}

void Coin::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
	gameModel->BuildMenus();
}
