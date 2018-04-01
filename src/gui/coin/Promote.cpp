#include <map>
#include "Promote.h"
#include "Format.h"
#include "client/Client.h"
#include "client/Download.h"
#include "client/MD5.h"
#include "graphics/Graphics.h"
#include "gui/Style.h"
#include "simulation/Simulation.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/game/Tool.h"
#include "gui/game/Menu.h"
#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"

#ifndef SERVERSALT
#define SERVERSALT "Pepper"
#endif

Promote::Promote(int saveID):
	ui::Window(ui::Point(-1, -1), ui::Point(300, 235)),
	saveID(saveID)
{
	std::string labelText = "You can promote saves to the front page using \xEA""owdercoins! Your first save is only 100 "
							"\xEA""owdercoins to promote, with future promotions costing more.\n\nThere are only a few requirements for promoting "
							"a save to FP. It must have never been demoted or disabled; and it must have a positive total amount of votes. It also must be published.\n\n"
							"\bgNotes on price: In order to limit abuse, each promotion will cost 10x the amount of \xEA""owdercoins as the last. "
							"Price will reset back to 100 \xEA""owdercoins at midnight UTC on Friday each week. Prices are subject to change without "
							"notice.\n\n"
							"\bwClick Continue to promote this save. Processing may take 10 seconds or more.";

	int height = Graphics::textwrapheight((char*)labelText.c_str(), 400);
	ui::Label *label = new ui::Label(ui::Point(10, 35), ui::Point(280, height), labelText);
	label->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	label->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	label->SetMultiline(true);
	AddComponent(label);


	// Copy paste from ConfirmPrompt, maybe I should just use that instead but I already created these files
	class CloseAction: public ui::ButtonAction
	{
	public:
		Promote * promote;
		bool result;
		CloseAction(Promote * promote_, bool result_) { promote = promote_; result = result_; }
		void ActionCallback(ui::Button * sender)
		{
			promote->CloseActiveWindow();
			promote->Callback(result);
			promote->SelfDestruct();
		}
	};

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X-75, 16), "Cancel");
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	cancelButton->SetActionCallback(new CloseAction(this, false));
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X-76, Size.Y-16), ui::Point(76, 16), "Continue");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::WarningTitle;
	okayButton->SetActionCallback(new CloseAction(this, true));
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	ui::Engine::Ref().ShowWindow(this);
}

void Promote::Callback(bool confirmed)
{
	//Client::Ref().SetCoins(100000);
	if (!confirmed)
	{
		new InformationMessage("Canceled", "You will not be charged", false);
		return;
	}
	std::stringstream promoteUrl;
	promoteUrl << "http://aprilfools.starcatcher.us/Moo/PromoteSave.lua";
	promoteUrl << "?username=" << Client::Ref().GetAuthUser().Username;

	std::stringstream random;
	random << rand() << rand() << rand() << rand();
	promoteUrl << "&random=" << random.str().substr(0, 20);

	int numCoins = Client::Ref().GetCoins();

	std::string salt = SERVERSALT;
	salt += "-" + Client::Ref().GetAuthUser().Username;
	salt += "-" + random.str().substr(0, 20);
	salt += "-" + format::NumberToString<int>(numCoins);
	std::cout << salt << std::endl;
	char saltMd5[33];
	md5_ascii(saltMd5, (const unsigned char *)salt.c_str(), salt.length());
	promoteUrl << "&hash=" << std::string(saltMd5);
	promoteUrl << "&numcoins=" << numCoins;
	promoteUrl << "&saveid=" << saveID;

	Download *download = new Download(promoteUrl.str());
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
		new ErrorMessage("Internal Server Error", "This isn't supposed to happen, please bug the admins about this");
		return;
	}

	char *data2 = new char[length+1];
	strncpy(data2, data, length);
	data2[length] = 0;

	int amount = format::StringToNumber<int>(data2);
	if (amount == 0)
	{
		new InformationMessage("Message from server", std::string(data2), false);
	}
	else
	{
		Client::Ref().AddCoins(-amount);
		new InformationMessage("Promoted!", "Save will appear on front page momentarily. New \xEA""owdercoins balance: " +
							   format::NumberToString<int>(Client::Ref().GetCoins()), false);
	}
}

void Promote::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

void Promote::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}
