#include "OptionsView.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include "SDLCompat.h"

#include "OptionsController.h"
#include "OptionsModel.h"

#include "client/Client.h"
#include "common/Platform.h"
#include "graphics/Graphics.h"
#include "gui/Style.h"
#include "simulation/ElementDefs.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/Checkbox.h"
#include "gui/interface/DropDown.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/DirectionSelector.h"

OptionsView::OptionsView():
	ui::Window(ui::Point(-1, -1), ui::Point(320, 340))
	{

	auto autowidth = [this](ui::Component *c) {
		c->Size.X = Size.X - c->Position.X - 12;
	};
	
	ui::Label * tempLabel = new ui::Label(ui::Point(4, 1), ui::Point(Size.X-8, 22), "Simulation Options");
	tempLabel->SetTextColour(style::Colour::InformationTitle);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	autowidth(tempLabel);
	AddComponent(tempLabel);

	class Separator : public ui::Component
	{
		public:
		Separator(ui::Point position, ui::Point size) : Component(position, size){}
		virtual ~Separator(){}

		void Draw(const ui::Point& screenPos) override
		{
			GetGraphics()->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 180);
		}		
	};
	
	Separator *tmpSeparator = new Separator(ui::Point(0, 22), ui::Point(Size.X, 1));
	AddComponent(tmpSeparator);

	int currentY = 6;
	scrollPanel = new ui::ScrollPanel(ui::Point(1, 23), ui::Point(Size.X-2, Size.Y-39));
	
	AddComponent(scrollPanel);

	heatSimulation = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Heat simulation \bgIntroduced in version 34", "");
	autowidth(heatSimulation);
	heatSimulation->SetActionCallback({ [this] { c->SetHeatSimulation(heatSimulation->GetChecked()); } });
	scrollPanel->AddChild(heatSimulation);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bgCan cause odd behaviour when disabled");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=16;
	ambientHeatSimulation = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Ambient heat simulation \bgIntroduced in version 50", "");
	autowidth(ambientHeatSimulation);
	ambientHeatSimulation->SetActionCallback({ [this] { c->SetAmbientHeatSimulation(ambientHeatSimulation->GetChecked()); } });
	scrollPanel->AddChild(ambientHeatSimulation);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bgCan cause odd / broken behaviour with many saves");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=16;
	newtonianGravity = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Newtonian gravity \bgIntroduced in version 48", "");
	autowidth(newtonianGravity);
	newtonianGravity->SetActionCallback({ [this] { c->SetNewtonianGravity(newtonianGravity->GetChecked()); } });
	scrollPanel->AddChild(newtonianGravity);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bgMay cause poor performance on older computers");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=16;
	waterEqualisation = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Water equalisation \bgIntroduced in version 61", "");
	autowidth(waterEqualisation);
	waterEqualisation->SetActionCallback({ [this] { c->SetWaterEqualisation(waterEqualisation->GetChecked()); } });
	scrollPanel->AddChild(waterEqualisation);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bgMay cause poor performance with a lot of water");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=19;
	airMode = new ui::DropDown(ui::Point(Size.X-95, currentY), ui::Point(80, 16));
	scrollPanel->AddChild(airMode);
	airMode->AddOption(std::pair<String, int>("On", 0));
	airMode->AddOption(std::pair<String, int>("Pressure off", 1));
	airMode->AddOption(std::pair<String, int>("Velocity off", 2));
	airMode->AddOption(std::pair<String, int>("Off", 3));
	airMode->AddOption(std::pair<String, int>("No Update", 4));
	airMode->SetActionCallback({ [this] { c->SetAirMode(airMode->GetOption().second); } });

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "Air Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	ambientAirTemp = new ui::Textbox(ui::Point(Size.X-95, currentY), ui::Point(60, 16));
	ambientAirTemp->SetActionCallback({ [this] {
		UpdateAirTemp(ambientAirTemp->GetText(), false);
	} });
	ambientAirTemp->SetDefocusCallback({ [this] {
		UpdateAirTemp(ambientAirTemp->GetText(), true);
	}});
	scrollPanel->AddChild(ambientAirTemp);

	ambientAirTempPreview = new ui::Button(ui::Point(Size.X-31, currentY), ui::Point(16, 16), "", "Preview");
	scrollPanel->AddChild(ambientAirTempPreview);

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "Ambient Air Temperature");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	gravityMode = new ui::DropDown(ui::Point(Size.X-95, currentY), ui::Point(80, 16));
	scrollPanel->AddChild(gravityMode);
	gravityMode->AddOption(std::pair<String, int>("Vertical", 0));
	gravityMode->AddOption(std::pair<String, int>("Off", 1));
	gravityMode->AddOption(std::pair<String, int>("Radial", 2));
	gravityMode->AddOption(std::pair<String, int>("Custom", 3));

	class GravityWindow : public ui::Window
	{
		void OnTryExit(ExitMethod method)
		{
			CloseActiveWindow();
			SelfDestruct();
		}

		void OnDraw()
		{
			Graphics * g = GetGraphics();

			g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
			g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
		}

		ui::DirectionSelector * gravityDirection;
		ui::Label * labelValues;

		OptionsController * c;

	public:
		GravityWindow(ui::Point position, int radius, float x, float y, OptionsController * c_):
			ui::Window(position, ui::Point((radius * 2) + 20, (radius * 2) + 75)),
			gravityDirection(new ui::DirectionSelector(ui::Point(10, 32), radius)),
			c(c_)
			{
				ui::Label * tempLabel = new ui::Label(ui::Point(4, 1), ui::Point(Size.X - 8, 22), "Custom Gravity");
				tempLabel->SetTextColour(style::Colour::InformationTitle);
				tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
				tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
				AddComponent(tempLabel);

				Separator * tempSeparator = new Separator(ui::Point(0, 22), ui::Point(Size.X, 1));
				AddComponent(tempSeparator);

				labelValues = new ui::Label(ui::Point(0, (radius * 2) + 37), ui::Point(Size.X, 16), String::Build(	"X:", std::round(x * 10.0f) / 10,
																													" Y:", std::round(y * 10.0f) / 10,
																													" Total:", std::round(std::hypot(x, y) * 10.0f) / 10));
				labelValues->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
				labelValues->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
				AddComponent(labelValues);

				gravityDirection->SetValues(x / 2.0f, y / 2.0f);
				gravityDirection->SetUpdateCallback([this](float x, float y) {
					labelValues->SetText(String::Build(	"X:", std::round(x * 20.0f) / 10,
														" Y:", std::round(y * 20.0f) / 10,
														" Total:", std::round(gravityDirection->GetTotalValue() * 20.0f) / 10));
				});
				gravityDirection->SetSnapPoints(5, 5);
				AddComponent(gravityDirection);

				ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y - 17), ui::Point(Size.X, 17), "OK");
				okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
				okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
				okayButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
				okayButton->SetActionCallback({ [this] {
					c->SetCustomGravityX(gravityDirection->GetXValue() * 2.0f);
					c->SetCustomGravityY(gravityDirection->GetYValue() * 2.0f);
					CloseActiveWindow();
					SelfDestruct();
				} });
				AddComponent(okayButton);
				SetOkayButton(okayButton);

				MakeActiveWindow();
			}
	};

	gravityMode->SetActionCallback({ [this] {
		c->SetGravityMode(gravityMode->GetOption().second);
		if (gravityMode->GetOption().second == 3)
			new GravityWindow(ui::Point(-1, -1), 50, customGravityX, customGravityY, c);
	} });

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "Gravity Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	edgeMode = new ui::DropDown(ui::Point(Size.X-95, currentY), ui::Point(80, 16));
	scrollPanel->AddChild(edgeMode);
	edgeMode->AddOption(std::pair<String, int>("Void", 0));
	edgeMode->AddOption(std::pair<String, int>("Solid", 1));
	edgeMode->AddOption(std::pair<String, int>("Loop", 2));
	edgeMode->SetActionCallback({ [this] { c->SetEdgeMode(edgeMode->GetOption().second); } });

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "Edge Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	tmpSeparator = new Separator(ui::Point(0, currentY), ui::Point(Size.X, 1));
	scrollPanel->AddChild(tmpSeparator);

	currentY+=4;
	scale = new ui::DropDown(ui::Point(8, currentY), ui::Point(40, 16));
	{
		int current_scale = ui::Engine::Ref().GetScale();
		int ix_scale = 1;
		bool current_scale_valid = false;
		do
		{
			if (current_scale == ix_scale)
				current_scale_valid = true;
			scale->AddOption(std::pair<String, int>(String::Build(ix_scale), ix_scale));
			ix_scale += 1;
		}
		while (ui::Engine::Ref().GetMaxWidth() >= ui::Engine::Ref().GetWidth() * ix_scale && ui::Engine::Ref().GetMaxHeight() >= ui::Engine::Ref().GetHeight() * ix_scale);
		if (!current_scale_valid)
			scale->AddOption(std::pair<String, int>("current", current_scale));
	}
	scale->SetActionCallback({ [this] { c->SetScale(scale->GetOption().second); } });
	scrollPanel->AddChild(scale);

	tempLabel = new ui::Label(ui::Point(scale->Position.X+scale->Size.X+3, currentY), ui::Point(Size.X-40, 16), "\bg- Window scale factor for larger screens");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	resizable = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Resizable", "");
	autowidth(resizable);
	resizable->SetActionCallback({ [this] { c->SetResizable(resizable->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(resizable->Position.X+Graphics::textwidth(resizable->GetText())+20, currentY), ui::Point(1, 16), "\bg- Allow resizing and maximizing window");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(resizable);

	currentY+=20;
	fullscreen = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Fullscreen", "");
	autowidth(fullscreen);
	fullscreen->SetActionCallback({ [this] { c->SetFullscreen(fullscreen->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(fullscreen->Position.X+Graphics::textwidth(fullscreen->GetText())+20, currentY), ui::Point(1, 16), "\bg- Fill the entire screen");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(fullscreen);

	currentY+=20;
	altFullscreen = new ui::Checkbox(ui::Point(23, currentY), ui::Point(1, 16), "Change Resolution", "");
	autowidth(altFullscreen);
	altFullscreen->SetActionCallback({ [this] { c->SetAltFullscreen(altFullscreen->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(altFullscreen->Position.X+Graphics::textwidth(altFullscreen->GetText())+20, currentY), ui::Point(1, 16), "\bg- Set optimal screen resolution");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(altFullscreen);

	currentY+=20;
	forceIntegerScaling = new ui::Checkbox(ui::Point(23, currentY), ui::Point(1, 16), "Force Integer Scaling", "");
	autowidth(forceIntegerScaling);
	forceIntegerScaling->SetActionCallback({ [this] { c->SetForceIntegerScaling(forceIntegerScaling->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(altFullscreen->Position.X+Graphics::textwidth(forceIntegerScaling->GetText())+20, currentY), ui::Point(1, 16), "\bg- Less blurry");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(forceIntegerScaling);

	currentY+=20;
	fastquit = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Fast Quit", "");
	autowidth(fastquit);
	fastquit->SetActionCallback({ [this] { c->SetFastQuit(fastquit->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(fastquit->Position.X+Graphics::textwidth(fastquit->GetText())+20, currentY), ui::Point(1, 16), "\bg- Always exit completely when hitting close");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(fastquit);

	currentY+=20;
	showAvatars = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Show Avatars", "");
	autowidth(showAvatars);
	showAvatars->SetActionCallback({ [this] { c->SetShowAvatars(showAvatars->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(showAvatars->Position.X+Graphics::textwidth(showAvatars->GetText())+20, currentY), ui::Point(1, 16), "\bg- Disable if you have a slow connection");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(showAvatars);

	currentY += 20;
	momentumScroll = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Momentum/Old Scrolling", "");
	autowidth(momentumScroll);
	momentumScroll->SetActionCallback({ [this] { c->SetMomentumScroll(momentumScroll->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(momentumScroll->Position.X + Graphics::textwidth(momentumScroll->GetText()) + 20, currentY), ui::Point(1, 16), "\bg- Accelerating instead of step scroll");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(momentumScroll);

	currentY+=20;
	mouseClickRequired = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Sticky Categories", "");
	autowidth(mouseClickRequired);
	mouseClickRequired->SetActionCallback({ [this] { c->SetMouseClickrequired(mouseClickRequired->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(mouseClickRequired->Position.X+Graphics::textwidth(mouseClickRequired->GetText())+20, currentY), ui::Point(1, 16), "\bg- Switch between categories by clicking");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(mouseClickRequired);

	currentY+=20;
	includePressure = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Include Pressure", "");
	autowidth(includePressure);
	includePressure->SetActionCallback({ [this] { c->SetIncludePressure(includePressure->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(includePressure->Position.X+Graphics::textwidth(includePressure->GetText())+20, currentY), ui::Point(1, 16), "\bg- When saving, copying, stamping, etc.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(includePressure);

	currentY+=20;
	perfectCirclePressure = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Perfect Circle", "");
	autowidth(perfectCirclePressure);
	perfectCirclePressure->SetActionCallback({ [this] { c->SetPerfectCircle(perfectCirclePressure->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(perfectCirclePressure->Position.X+Graphics::textwidth(perfectCirclePressure->GetText())+20, currentY), ui::Point(1, 16), "\bg- Better circle brush, without incorrect points on edges");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(perfectCirclePressure);

	currentY+=20;
	decoSpace = new ui::DropDown(ui::Point(8, currentY), ui::Point(60, 16));
	decoSpace->SetActionCallback({ [this] { c->SetDecoSpace(decoSpace->GetOption().second); } });
	scrollPanel->AddChild(decoSpace);
	decoSpace->AddOption(std::pair<String, int>("sRGB", 0));
	decoSpace->AddOption(std::pair<String, int>("Linear", 1));
	decoSpace->AddOption(std::pair<String, int>("Gamma 2.2", 2));
	decoSpace->AddOption(std::pair<String, int>("Gamma 1.8", 3));

	tempLabel = new ui::Label(ui::Point(decoSpace->Position.X+decoSpace->Size.X+3, currentY), ui::Point(Size.X-40, 16), "\bg- Colour space used by decoration tools");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	ui::Button * dataFolderButton = new ui::Button(ui::Point(8, currentY), ui::Point(90, 16), "Open Data Folder");
	dataFolderButton->SetActionCallback({ [] {
		ByteString cwd = Platform::GetCwd();
		if (!cwd.empty())
			Platform::OpenURI(cwd);
		else
			fprintf(stderr, "cannot open data folder: getcwd(...) failed\n");
	} });
	scrollPanel->AddChild(dataFolderButton);

	ui::Button * migrationButton = new ui::Button(ui::Point(Size.X - 178, currentY), ui::Point(163, 16), "Migrate to shared data directory");
	migrationButton->SetActionCallback({ [] {
		ByteString from = Platform::originalCwd;
		ByteString to = Platform::sharedCwd;
		new ConfirmPrompt("Do Migration?", "This will migrate all stamps, saves, and scripts from\n\bt" + from.FromUtf8() + "\bw\nto the shared data directory at\n\bt" + to.FromUtf8() + "\bw\n\n" +
			 "Files that already exist will not be overwritten.", { [=] () {
				 String ret = Platform::DoMigration(from, to);
				new InformationMessage("Migration Complete", ret, false);
			 } });
	} });
	scrollPanel->AddChild(migrationButton);

	ui::Button * tempButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	tempButton->SetActionCallback({ [this] { c->Exit(); } });
	AddComponent(tempButton);
	SetCancelButton(tempButton);
	SetOkayButton(tempButton);
	currentY+=20;
	scrollPanel->InnerSize = ui::Point(Size.X, currentY);
}

void OptionsView::UpdateAmbientAirTempPreview(float airTemp, bool isValid)
{
	if (isValid)
	{
		int HeatToColour(float temp);
		int c = HeatToColour(airTemp);
		ambientAirTempPreview->Appearance.BackgroundInactive = ui::Colour(PIXR(c), PIXG(c), PIXB(c));
		ambientAirTempPreview->SetText("");
	}
	else
	{
		ambientAirTempPreview->Appearance.BackgroundInactive = ui::Colour(0, 0, 0);
		ambientAirTempPreview->SetText("?");
	}
	ambientAirTempPreview->Appearance.BackgroundHover = ambientAirTempPreview->Appearance.BackgroundInactive;
}

void OptionsView::UpdateAirTemp(String temp, bool isDefocus)
{
	// Parse air temp and determine validity
	float airTemp;
	bool isValid;
	try
	{
		void ParseFloatProperty(String value, float &out);
		ParseFloatProperty(temp, airTemp);
		isValid = true;
	}
	catch (const std::exception &ex)
	{
		isValid = false;
	}

	// While defocusing, correct out of range temperatures and empty textboxes
	if (isDefocus)
	{
		if (temp.empty())
		{
			isValid = true;
			airTemp = float(R_TEMP) + 273.15f;
		}
		else if (!isValid)
			return;
		else if (airTemp < MIN_TEMP)
			airTemp = MIN_TEMP;
		else if (airTemp > MAX_TEMP)
			airTemp = MAX_TEMP;
		else
			return;

		// Update textbox with the new value
		StringBuilder sb;
		sb << Format::Precision(2) << airTemp;
		ambientAirTemp->SetText(sb.Build());
	}
	// Out of range temperatures are invalid, preview should go away
	else if (airTemp < MIN_TEMP || airTemp > MAX_TEMP)
		isValid = false;

	// If valid, set temp
	if (isValid)
		c->SetAmbientAirTemperature(airTemp);

	UpdateAmbientAirTempPreview(airTemp, isValid);
}

void OptionsView::NotifySettingsChanged(OptionsModel * sender)
{
	heatSimulation->SetChecked(sender->GetHeatSimulation());
	ambientHeatSimulation->SetChecked(sender->GetAmbientHeatSimulation());
	newtonianGravity->SetChecked(sender->GetNewtonianGravity());
	waterEqualisation->SetChecked(sender->GetWaterEqualisation());
	airMode->SetOption(sender->GetAirMode());
	// Initialize air temp and preview only when the options menu is opened, and not when user is actively editing the textbox
	if (!initializedAirTempPreview)
	{
		initializedAirTempPreview = true;
		float airTemp = sender->GetAmbientAirTemperature();
		UpdateAmbientAirTempPreview(airTemp, true);
		StringBuilder sb;
		sb << Format::Precision(2) << airTemp;
		ambientAirTemp->SetText(sb.Build());
	}
	gravityMode->SetOption(sender->GetGravityMode());
	customGravityX = sender->GetCustomGravityX();
	customGravityY = sender->GetCustomGravityY();
	decoSpace->SetOption(sender->GetDecoSpace());
	edgeMode->SetOption(sender->GetEdgeMode());
	scale->SetOption(sender->GetScale());
	resizable->SetChecked(sender->GetResizable());
	fullscreen->SetChecked(sender->GetFullscreen());
	altFullscreen->SetChecked(sender->GetAltFullscreen());
	forceIntegerScaling->SetChecked(sender->GetForceIntegerScaling());
	fastquit->SetChecked(sender->GetFastQuit());
	showAvatars->SetChecked(sender->GetShowAvatars());
	mouseClickRequired->SetChecked(sender->GetMouseClickRequired());
	includePressure->SetChecked(sender->GetIncludePressure());
	perfectCirclePressure->SetChecked(sender->GetPerfectCircle());
	momentumScroll->SetChecked(sender->GetMomentumScroll());
}

void OptionsView::AttachController(OptionsController * c_)
{
	c = c_;
}

void OptionsView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

void OptionsView::OnTryExit(ExitMethod method)
{
	c->Exit();
}


OptionsView::~OptionsView() {
}
