#include <algorithm>
#include "CardInput.h"
#include "Format.h"
#include "client/Client.h"
#include "graphics/Graphics.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/DropDown.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"

CardInput::CardInput(int chance, int amount):
	ui::Window(ui::Point(-1, -1), ui::Point(300, 200)),
	purchaseChance(chance),
	amount(amount)
{
	class NextAction: public ui::ButtonAction
	{
	public:
		void ActionCallback(ui::Button * sender)
		{
			((CardInput*)sender->GetParentWindow())->Next();
		}
	};
	next = new ui::Button(ui::Point(250, 175), ui::Point(45, 20), "Next");
	next->SetActionCallback(new NextAction());
	AddComponent(next);

	SetOkayButton(next);

	signature = nullptr;
	Step1();
}

void CardInput::Next()
{
	if (step == 1)
		Step2();
	else if (step == 2)
		AttemptPurchase();
}

void CardInput::AttemptPurchase()
{
	std::string error;
	int num = rand()%100;
	if (num < purchaseChance)
	{
		bool confirm = ConfirmPrompt::Blocking("Purchase \xEA""owdercoins?", "Clicking 'Confirm' will charge your card and add " +
											   format::NumberToString<int>(amount) + " \xEA""owdercoins to your account");
		if (confirm)
		{
			Client::Ref().AddCoins(amount);
			CloseActiveWindow();
			SelfDestruct();
			new InformationMessage("Confirmed!", "New balance: " + format::NumberToString<int>(Client::Ref().GetCoins()), false);
			//ui::Engine::Ref().ShowWindow(info);
			return;
		}
		return;
	}
	if (num < 30)
		error = "Card did not go through. Please revalidate card information";
	else if (num < 40)
		error = "Card expired. Please use a valid card";
	else if (num < 50)
		error = "Signature too messy. Please take handwriting classes then retry.";
	else if (num < 60)
		error = "We cannot process purchases in this country at this time";
	else if (num < 70)
		error = "Invalid address. We could not find this address in our systems";
	else if (num < 90)
		error = "Card type not supported. Supported card types are: 'L\xF0""F Lirpa The Powder Toy EXTRAVote Co-Branded Credit Card";
	else
		error = "Our payment processing systems are currently on fire. Please try again later";
	ErrorMessage::Blocking("Failure buying \xEA""owdercoins", error);
	CloseActiveWindow();
	// You have 10 seconds to read this message before it self destructs
	SelfDestruct();
}

void CardInput::Step1()
{
	ClearComponents(1);

	std::string labelText = "\boInput credit or debit card";
	int labelSize = Graphics::textwidth(labelText.c_str());
	ui::Label *label = new ui::Label(ui::Point(8, 5), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	class CardInputHelper : public ui::TextboxAction
	{
		CardInput * v;
		ui::Component *next;
	public:
		CardInputHelper(CardInput * v, ui::Component *next) { this->v = v; this->next = next; }
		virtual void TextChangedCallback(ui::Textbox * sender)
		{
			if (sender->GetText().length() == 4)
				v->FocusComponent(next);
		}
	};

	labelText = "Card Number: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(15, 25), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	ui::Textbox *card1 = new ui::Textbox(ui::Point(labelSize + 15, 25), ui::Point(35, 20));
	card1->SetInputType(ui::Textbox::Number);
	card1->SetLimit(4);
	AddComponent(card1);

	ui::Textbox *card2 = new ui::Textbox(ui::Point(labelSize + 55, 25), ui::Point(35, 20));
	card2->SetInputType(ui::Textbox::Number);
	card2->SetLimit(4);
	AddComponent(card2);

	ui::Textbox *card3 = new ui::Textbox(ui::Point(labelSize + 95, 25), ui::Point(35, 20));
	card3->SetInputType(ui::Textbox::Number);
	card3->SetLimit(4);
	AddComponent(card3);

	ui::Textbox *card4 = new ui::Textbox(ui::Point(labelSize + 135, 25), ui::Point(35, 20));
	card4->SetInputType(ui::Textbox::Number);
	card4->SetLimit(3);
	AddComponent(card4);

	card1->SetActionCallback(new CardInputHelper(this, card2));
	card2->SetActionCallback(new CardInputHelper(this, card3));
	card3->SetActionCallback(new CardInputHelper(this, card4));
	//card4->SetActionCallback(new CardInputHelper(this, card2));
	textboxChecks.push_back(card1);
	textboxChecks.push_back(card2);
	textboxChecks.push_back(card3);
	textboxChecks.push_back(card4);


	labelText = "Expiration Date: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(15, 55), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	ui::DropDown *expirationMonth = new ui::DropDown(ui::Point(labelSize + 15, 55), ui::Point(25, 20));
	expirationMonth->SetOptions({
									{"01", 1},
									{"02", 2},
									{"03", 3},
									{"04", 4},
									{"05", 5},
									{"06", 6},
									{"07", 7},
									{"08", 8},
									{"09", 9},
									{"10", 10},
									{"11", 11},
									{"12", 12}
								});
	expirationMonth->SetOption(1);
	AddComponent(expirationMonth);

	ui::DropDown *expirationYear = new ui::DropDown(ui::Point(labelSize + 45, 55), ui::Point(35, 20));
	for (int i = 0; i < 15; i++)
		expirationYear->AddOption({format::NumberToString(2017-i), i+1});
	expirationYear->SetOption(1);
	AddComponent(expirationYear);

	labelText = "Security Code: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(180, 55), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	ui::Textbox *securityCode = new ui::Textbox(ui::Point(labelSize + 180, 55), ui::Point(25, 20));
	securityCode->SetInputType(ui::Textbox::Number);
	securityCode->SetLimit(3);
	AddComponent(securityCode);
	textboxChecks.push_back(securityCode);


	labelText = "Sign Here: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(15, 85), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	signature = new VideoBuffer(250, 50);
	class ClearSigAction: public ui::ButtonAction
	{
		VideoBuffer * signature;
	public:
		ClearSigAction(VideoBuffer * signature) { this->signature = signature; }
		void ActionCallback(ui::Button * sender)
		{
			pixel *buf = signature->Buffer;
			std::fill(&buf[0], &buf[signature->Width*signature->Height], 0);
			((CardInput*)sender->GetParentWindow())->signatureValid = false;
		}
	};

	ui::Button *clearSignature = new ui::Button(ui::Point(10, 154), ui::Point(75, 20), "Clear Signature");
	clearSignature->SetActionCallback(new ClearSigAction(signature));
	AddComponent(clearSignature);
}

void CardInput::Step2()
{
	ClearComponents(2);

	std::string labelText = "\boInput billing address";
	int labelSize = Graphics::textwidth(labelText.c_str());
	ui::Label *label = new ui::Label(ui::Point(8, 5), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	labelText = "Country: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(15, 25), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	//std::vector<std::string> countryList = {"Afghanistan", "Albania", "Algeria", "Antarctica", "Argentina", "Armenia", "Atlantis", "Australia", "Austria", "Azerbaijan", "Bahamas", "Bahrain", "Bangladesh", "Belarus", "Belgium", "Bolivia", "Brazil", "Bulgaria", "Cambodia", "Cameroon", "Canada, eh?", "California", "Chad", "Chile", "China", "Colombia", "Democratic People's Republic of Korea", "Costa Rica", "Croatia", "Cuba", "Cyprus", "Czech Republic", "Denmark", "Djibouti", "Dominica", "Dominican Republic", "Ecuador", "Egypt", "El Salvador", "Equatorial Guinea", "Estonia", "Ethiopia", "European Union", "Fiji", "Finland", "France", "Georgia", "East Germany", "West Germany", "Ghana", "Greece", "Guatemala", "Guinea", "Haiti", "Honduras", "Hungry", "Iceland", "India", "Indonesia", "Iran", "Iraq", "Ireland", "Islamic State of Iraq and Syria", "Israel", "Italy", "Jamaica", "Japan", "Jordan", "Kazakhstan", "Kenya", "Kiribati", "Kosovo", "Kuwait", "Kyrgyzstan", "Laos", "Latvia", "Lebanon", "Ledhlzigstan", "Lesotho", "Liberia", "Libya", "Liechtenstein", "Lithuania", "Longitudinal Guinea", "Luxembourg", "Macedonia (Greek Province)", "Madagascar", "Malaysia", "Malta", "Mexico", "Moldova", "Mongolia", "Montenegro", "Morocco", "Mozambique", "Murrica", "Myanmar", "Namibia", "Nauru", "Nepal", "Netherlands", "New York City", "New Zealand", "Nicaragua", "Niger", "Nigeria", "Norway", "Oman", "Pakistan", "Palau", "Palestine", "Panama", "Papua New Guinea", "Paraguay", "Peru", "Philippines", "Poland", "Portugal", "Qatar", "Romania", "Russia", "Rwanda", "Samoa", "San Marino", "Sao Tome and Principe", "Saudi Arabia", "Senegal", "Serbia", "Sierra Leone", "Singapore", "Slopokia", "Slovakia", "Slovenia", "Solomon Islands", "Somalia", "South Africa", "South Dakota", "South Korea", "South Sudan", "Spain", "Sri Lanka", "Sudan", "Suriname", "Swaziland", "Sweden", "Switzerland", "Syria", "Taiwan", "Tajikistan", "Tanzania", "Texas", "Tieland", "Turkey", "Ham", "Turkmenistan", "Tuvalu", "Uganda", "Ukraine", "UAE", "United Kingdom (UK)", "Uranus", "Uruguay", "USSR", "Uzbekistan", "Vanuatu", "Vatican City", "Venezuela", "Vietnam", "Wakanda", "Yemen", "Zambia", "Zimbabwe"};
	std::vector<std::string> countryList = {"United States of America", "Other"};
	ui::DropDown *countryDropdown = new ui::DropDown(ui::Point(labelSize + 15, 25), ui::Point(150, 20));
	int num = 0;
	for (std::string country : countryList)
	{
		countryDropdown->AddOption(std::pair<std::string, int>(country, num));
		num++;
	}
	countryDropdown->SetOption(0);
	AddComponent(countryDropdown);

	labelText = "State: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(15, 50), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	std::vector<std::string> stateList = {"Alabama", "Alaska", "Arizona", "Arkansas", "California", "Colorado", "Connecticut", "Delaware", "Florida", "Georgia", "Hawaii", "Idaho", "Illinois", "Indiana", "Iowa", "Kansas", "Kentucky", "Louisiana", "Maine", "Maryland", "Massachusetts", "Michigan", "Minnesota", "Mississippi", "Missouri", "Montana", "Nebraska", "Nevada", "New Hampshire", "New Jersey", "New Mexico", "New York", "North Carolina", "North Dakota", "Ohio", "Oklahoma", "Oregon", "Pennsylvania", "Rhode Island", "South Carolina", "South Dakota", "Tennessee", "Texas", "Utah", "Vermont", "Virginia", "Washington", "West Virginia", "Wisconsin", "Wyoming"};
	std::sort(&stateList[0], &stateList[stateList.size()], [](std::string a, std::string b) { return a.length() < b.length(); });
	ui::DropDown *stateDropdown = new ui::DropDown(ui::Point(labelSize + 15, 50), ui::Point(70, 20));
	num = 0;
	for (std::string state : stateList)
	{
		stateDropdown->AddOption(std::pair<std::string, int>(state, num));
		num++;
	}
	stateDropdown->SetOption(0);
	AddComponent(stateDropdown);


	labelText = "Street line 1: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(15, 75), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	ui::Textbox * textbox = new ui::Textbox(ui::Point(labelSize + 15, 75), ui::Point(200, 20));
	textbox->SetInputType(ui::Textbox::Number);
	textbox->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(textbox);
	textboxChecks.push_back(textbox);

	labelText = "Street line 2: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(15, 100), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	textbox = new ui::Textbox(ui::Point(labelSize + 15, 100), ui::Point(200, 20));
	textbox->SetLimit(5);
	textbox->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(textbox);
	textboxChecks.push_back(textbox);


	labelText = "City: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(15, 125), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	textbox = new ui::Textbox(ui::Point(labelSize + 15, 125), ui::Point(100, 20));
	textbox->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(textbox);
	textboxChecks.push_back(textbox);

	labelText = "Zip Code: ";
	labelSize = Graphics::textwidth(labelText.c_str());
	label = new ui::Label(ui::Point(15, 150), ui::Point(labelSize, 16), labelText);
	AddComponent(label);

	textbox = new ui::Textbox(ui::Point(labelSize + 15, 150), ui::Point(50, 20));
	textbox->SetInputType(ui::Textbox::Numeric);
	textbox->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(textbox);
	textboxChecks.push_back(textbox);
}

void CardInput::ClearComponents(int step)
{
	textboxChecks.clear();
	for (int i = Components.size()-1; i >= 0; i--)
	{
		if (Components[i] != next)
			RemoveComponent(i);
	}
	this->step = step;
}

CardInput::~CardInput()
{
	delete signature;
}

void CardInput::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);

	if (step == 1)
	{
		g->draw_image(signature->Buffer, Position.X+10, Position.Y+105, 250, 50, 255);
		g->drawrect(Position.X+10, Position.Y+105, 250, 50, 255, 255, 255, 255);
	}
}

void CardInput::OnTick(float dt)
{
	bool valid = true;
	for (auto textbox : textboxChecks)
	{
		if (textbox->GetText().length() < 3)
			valid = false;
	}

	if (next)
		next->Enabled = valid && (step != 1 || signatureValid);
}

void CreateLine(int x1, int y1, int x2, int y2, VideoBuffer *signature)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy;
	float e, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	e = 0.0f;
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			signature->SetPixel(y, x, 255, 255, 255, 255);
		else
			signature->SetPixel(x, y, 255, 255, 255, 255);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if ((y1<y2) ? (y<=y2) : (y>=y2))
			{
				if (reverseXY)
					signature->SetPixel(y, x, 255, 255, 255, 255);
				else
					signature->SetPixel(x, y, 255, 255, 255, 255);
			}
			e -= 1.0f;
		}
	}
}

void CardInput::OnMouseMove(int x, int y, int dx, int dy)
{
	if (step == 1 && x >= Position.X+10 && x < Position.X+260 && y > Position.Y+105 && y < Position.Y+155)
	{
		if (drawing)
		{
			// extra accuracy
			dx += (int)(((rand()%11)-5)/5);
			dy += (int)(((rand()%11)-5)/5);
			CreateLine(x - (Position.X+10), y - (Position.Y+105), x - (Position.X+10) - dx, y - (Position.Y+105) - dy, signature);
			signatureValid = true;
		}
	}
}

void CardInput::OnMouseDown(int x, int y, unsigned button)
{
	drawing = true;
}

void CardInput::OnMouseUp(int x, int y, unsigned button)
{
	drawing = false;
}

void CardInput::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}
