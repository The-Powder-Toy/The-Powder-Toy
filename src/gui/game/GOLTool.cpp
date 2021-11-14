#include "Tool.h"

#include "client/Client.h"
#include "common/tpt-rand.h"
#include "simulation/GOLString.h"
#include "simulation/Simulation.h"

#include "gui/Style.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Window.h"
#include "gui/colourpicker/ColourPickerActivity.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/game/GameModel.h"

#include "graphics/Graphics.h"

class GOLWindow: public ui::Window
{
	void UpdateGradient();

public:
	ui::Colour highColour, lowColour;
	ui::Button *highColourButton, *lowColourButton;
	ui::Textbox *nameField, *ruleField;
	GameModel * gameModel;
	Simulation *sim;
	int toolSelection;
	GOLWindow(GameModel *gameModel, Simulation *sim, int toolSelection, int rule, int colour1, int colour2);
	void Validate();
	void OnDraw() override;
	void OnTryExit(ExitMethod method) override;
	virtual ~GOLWindow() {}
};

GOLWindow::GOLWindow(GameModel * gameModel_, Simulation *sim_, int toolSelection, int rule, int colour1, int colour2):
ui::Window(ui::Point(-1, -1), ui::Point(200, 108)),
gameModel(gameModel_),
sim(sim_),
toolSelection(toolSelection)
{
	ui::Label * messageLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 14), "Edit custom GOL type");
	messageLabel->SetTextColour(style::Colour::InformationTitle);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);

	auto *okayButton = new ui::Button(ui::Point(0, Size.Y-17), ui::Point(Size.X, 17), "OK");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	okayButton->SetActionCallback({ [this] {
		if (nameField->GetText().length() && ruleField->GetText().length())
		{
			CloseActiveWindow();
			Validate();
			SelfDestruct();
		}
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	nameField = new ui::Textbox(ui::Point(8, 25), ui::Point(Size.X-16, 16), "", "[name]");
	nameField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	nameField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	nameField->SetLimit(7);
	AddComponent(nameField);
	FocusComponent(nameField);

	ruleField = new ui::Textbox(ui::Point(8, 46), ui::Point(Size.X-16, 16), "", "[rule]");
	ruleField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	ruleField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(ruleField);
	FocusComponent(ruleField);

	highColourButton = new ui::Button(ui::Point(8, 67), ui::Point(16, 16), "");
	highColourButton->SetActionCallback({ [this] {
		new ColourPickerActivity(highColour, [this](ui::Colour colour) {
			highColour = colour;
			UpdateGradient();
		});
	} });
	AddComponent(highColourButton);

	lowColourButton = new ui::Button(ui::Point(Size.X - 24, 67), ui::Point(16, 16), "");
	lowColourButton->SetActionCallback({ [this] {
		new ColourPickerActivity(lowColour, [this](ui::Colour colour) {
			lowColour = colour;
			UpdateGradient();
		});
	} });
	AddComponent(lowColourButton);

	if (rule)
	{
		ruleField->SetText(SerialiseGOLRule(rule));
		nameField->SetText("");
		highColour.Red = PIXR(colour1);
		highColour.Green = PIXG(colour1);
		highColour.Blue = PIXB(colour1);
		lowColour.Red = PIXR(colour2);
		lowColour.Green = PIXG(colour2);
		lowColour.Blue = PIXB(colour2);
	}
	else
	{
		ruleField->SetText(Client::Ref().GetPrefString("CustomGOL.Rule", "B3/S23"));
		nameField->SetText(Client::Ref().GetPrefString("CustomGOL.Name", "CGOL"));
		highColour.Red = RNG::Ref().between(0x80, 0xFF);
		highColour.Green = RNG::Ref().between(0x80, 0xFF);
		highColour.Blue = RNG::Ref().between(0x80, 0xFF);
		lowColour.Red = RNG::Ref().between(0x00, 0x7F);
		lowColour.Green = RNG::Ref().between(0x00, 0x7F);
		lowColour.Blue = RNG::Ref().between(0x00, 0x7F);
	}
	highColour.Alpha = 255;
	lowColour.Alpha = 255;
	UpdateGradient();

	MakeActiveWindow();
}

void GOLWindow::UpdateGradient()
{
	highColourButton->Appearance.BackgroundInactive = highColour;
	highColourButton->Appearance.BackgroundHover = highColour;
	lowColourButton->Appearance.BackgroundInactive = lowColour;
	lowColourButton->Appearance.BackgroundHover = lowColour;
}

void GOLWindow::Validate()
{
	auto nameString = nameField->GetText();
	auto ruleString = ruleField->GetText();
	if (!ValidateGOLName(nameString))
	{
		new ErrorMessage("Could not add GOL type", "Invalid name provided");
		return;
	}
	nameString = nameString.ToUpper();
	int rule = ParseGOLString(ruleString);
	if (rule == -1)
	{
		new ErrorMessage("Could not add GOL type", "Invalid rule provided");
		return;
	}
	if (sim->GetCustomGOLByRule(rule))
	{
		new ErrorMessage("Could not add GOL type", "This Custom GoL rule already exists");
		return;
	}
	ruleString = SerialiseGOLRule(rule); // * Make it canonical.

	Client::Ref().SetPrefUnicode("CustomGOL.Name", nameString);
	Client::Ref().SetPrefUnicode("CustomGOL.Rule", ruleString);

	auto color1 = (((highColour.Red << 8) | highColour.Green) << 8) | highColour.Blue;
	auto color2 = (((lowColour.Red << 8) | lowColour.Green) << 8) | lowColour.Blue;
	if (!AddCustomGol(ruleString, nameString, color1, color2))
	{
		new ErrorMessage("Could not add GOL type", "Name already taken");
		return;
	}

	gameModel->SelectNextIdentifier = "DEFAULT_PT_LIFECUST_" + nameString.ToAscii();
	gameModel->SelectNextTool = toolSelection;
}

void GOLWindow::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}

void GOLWindow::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);

	int width = Size.X - 60;
	for (int xx = 0; xx < width; ++xx)
	{
		auto f = xx / (float)width;
		for (int yy = 0; yy < 16; ++yy)
		{
			auto rr = int(highColour.Red * (1.f - f) + lowColour.Red * f);
			auto gg = int(highColour.Green * (1.f - f) + lowColour.Green * f);
			auto bb = int(highColour.Blue * (1.f - f) + lowColour.Blue * f);
			g->blendpixel(Position.X + xx + 30, Position.Y + yy + 67, rr, gg, bb, 255);
		}
	}
}

void GOLTool::OpenWindow(Simulation *sim, int toolSelection, int rule, int colour1, int colour2)
{
	new GOLWindow(gameModel, sim, toolSelection, rule, colour1, colour2);
}
