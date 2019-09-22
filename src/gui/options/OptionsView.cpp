#include "OptionsView.h"

#include "OptionsController.h"
#include "OptionsModel.h"

#include <cstdio>
#ifdef WIN
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif
#include "SDLCompat.h"

#include "gui/Style.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/DropDown.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Checkbox.h"

#include "graphics/Graphics.h"

OptionsView::OptionsView():
	ui::Window(ui::Point(-1, -1), ui::Point(320, 340)){

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

		void Draw(const ui::Point& screenPos)
		{
			GetGraphics()->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 180);
		}		
	};
	
	Separator *tmpSeparator = new Separator(ui::Point(0, 22), ui::Point(Size.X, 1));
	AddComponent(tmpSeparator);

	int currentY = 6;
	scrollPanel = new ui::ScrollPanel(ui::Point(1, 23), ui::Point(Size.X-2, Size.Y-39));
	
	AddComponent(scrollPanel);

	class HeatSimulationAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		HeatSimulationAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override {
			v->c->SetHeatSimulation(sender->GetChecked());
		}
	};

	heatSimulation = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Heat simulation \bgIntroduced in version 34", "");
	autowidth(heatSimulation);
	heatSimulation->SetActionCallback(new HeatSimulationAction(this));
	scrollPanel->AddChild(heatSimulation);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bgCan cause odd behaviour when disabled");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	class AmbientHeatSimulationAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		AmbientHeatSimulationAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override {
			v->c->SetAmbientHeatSimulation(sender->GetChecked());
		}
	};

	currentY+=16;
	ambientHeatSimulation = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Ambient heat simulation \bgIntroduced in version 50", "");
	autowidth(ambientHeatSimulation);
	ambientHeatSimulation->SetActionCallback(new AmbientHeatSimulationAction(this));
	scrollPanel->AddChild(ambientHeatSimulation);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bgCan cause odd / broken behaviour with many saves");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	class NewtonianGravityAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		NewtonianGravityAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override {
			v->c->SetNewtonianGravity(sender->GetChecked());
		}
	};

	currentY+=16;
	newtonianGravity = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Newtonian gravity \bgIntroduced in version 48", "");
	autowidth(newtonianGravity);
	newtonianGravity->SetActionCallback(new NewtonianGravityAction(this));
	scrollPanel->AddChild(newtonianGravity);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bgMay cause poor performance on older computers");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	class WaterEqualisationAction: public ui::CheckboxAction
		{
			OptionsView * v;
		public:
			WaterEqualisationAction(OptionsView * v_){	v = v_;	}
			void ActionCallback(ui::Checkbox * sender) override {
				v->c->SetWaterEqualisation(sender->GetChecked());
			}
		};

	currentY+=16;
	waterEqualisation = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Water equalisation \bgIntroduced in version 61", "");
	autowidth(waterEqualisation);
	waterEqualisation->SetActionCallback(new WaterEqualisationAction(this));
	scrollPanel->AddChild(waterEqualisation);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bgMay cause poor performance with a lot of water");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	class AirModeChanged: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		AirModeChanged(OptionsView * v): v(v) { }
		void OptionChanged(ui::DropDown * sender, std::pair<String, int> option) override {
			v->c->SetAirMode(option.second);
		}
	};
	currentY+=19;
	airMode = new ui::DropDown(ui::Point(Size.X-95, currentY), ui::Point(80, 16));
	scrollPanel->AddChild(airMode);
	airMode->AddOption(std::pair<String, int>("On", 0));
	airMode->AddOption(std::pair<String, int>("Pressure off", 1));
	airMode->AddOption(std::pair<String, int>("Velocity off", 2));
	airMode->AddOption(std::pair<String, int>("Off", 3));
	airMode->AddOption(std::pair<String, int>("No Update", 4));
	airMode->SetActionCallback(new AirModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "Air Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	class GravityModeChanged: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		GravityModeChanged(OptionsView * v): v(v) { }
		void OptionChanged(ui::DropDown * sender, std::pair<String, int> option) override {
			v->c->SetGravityMode(option.second);
		}
	};

	currentY+=20;
	gravityMode = new ui::DropDown(ui::Point(Size.X-95, currentY), ui::Point(80, 16));
	scrollPanel->AddChild(gravityMode);
	gravityMode->AddOption(std::pair<String, int>("Vertical", 0));
	gravityMode->AddOption(std::pair<String, int>("Off", 1));
	gravityMode->AddOption(std::pair<String, int>("Radial", 2));
	gravityMode->SetActionCallback(new GravityModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "Gravity Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	class EdgeModeChanged: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		EdgeModeChanged(OptionsView * v): v(v) { }
		void OptionChanged(ui::DropDown * sender, std::pair<String, int> option) override {
			v->c->SetEdgeMode(option.second);
		}
	};
	currentY+=20;
	edgeMode = new ui::DropDown(ui::Point(Size.X-95, currentY), ui::Point(80, 16));
	scrollPanel->AddChild(edgeMode);
	edgeMode->AddOption(std::pair<String, int>("Void", 0));
	edgeMode->AddOption(std::pair<String, int>("Solid", 1));
	edgeMode->AddOption(std::pair<String, int>("Loop", 2));
	edgeMode->SetActionCallback(new EdgeModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "Edge Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	tmpSeparator = new Separator(ui::Point(0, currentY), ui::Point(Size.X, 1));
	scrollPanel->AddChild(tmpSeparator);

	class ScaleAction: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		ScaleAction(OptionsView * v): v(v) { }
		void OptionChanged(ui::DropDown * sender, std::pair<String, int> option) override {
			v->c->SetScale(option.second);
		}
	};
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
	scale->SetActionCallback(new ScaleAction(this));
	scrollPanel->AddChild(scale);

	tempLabel = new ui::Label(ui::Point(scale->Position.X+scale->Size.X+3, currentY), ui::Point(Size.X-40, 16), "\bg- Window scale factor for larger screens");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);


	class ResizableAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		ResizableAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override
		{
			v->c->SetResizable(sender->GetChecked());
		}
	};
	currentY+=20;
	resizable = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Resizable", "");
	autowidth(resizable);
	resizable->SetActionCallback(new ResizableAction(this));
	tempLabel = new ui::Label(ui::Point(resizable->Position.X+Graphics::textwidth(resizable->GetText())+20, currentY), ui::Point(1, 16), "\bg- Allow resizing and maximizing window");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(resizable);

	class FullscreenAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		FullscreenAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override
		{
			v->c->SetFullscreen(sender->GetChecked());
		}
	};

	currentY+=20;
	fullscreen = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Fullscreen", "");
	autowidth(fullscreen);
	fullscreen->SetActionCallback(new FullscreenAction(this));
	tempLabel = new ui::Label(ui::Point(fullscreen->Position.X+Graphics::textwidth(fullscreen->GetText())+20, currentY), ui::Point(1, 16), "\bg- Fill the entire screen");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(fullscreen);

	class AltFullscreenAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		AltFullscreenAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override
		{
			v->c->SetAltFullscreen(sender->GetChecked());
		}
	};
	currentY+=20;
	altFullscreen = new ui::Checkbox(ui::Point(23, currentY), ui::Point(1, 16), "Change Resolution", "");
	autowidth(altFullscreen);
	altFullscreen->SetActionCallback(new AltFullscreenAction(this));
	tempLabel = new ui::Label(ui::Point(altFullscreen->Position.X+Graphics::textwidth(altFullscreen->GetText())+20, currentY), ui::Point(1, 16), "\bg- Set optimial screen resolution");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(altFullscreen);

	class ForceIntegerScalingAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		ForceIntegerScalingAction(OptionsView * v_) { v = v_; }
		void ActionCallback(ui::Checkbox * sender) override
		{
			v->c->SetForceIntegerScaling(sender->GetChecked());
		}
	};

	currentY+=20;
	forceIntegerScaling = new ui::Checkbox(ui::Point(23, currentY), ui::Point(1, 16), "Force Integer Scaling", "");
	autowidth(forceIntegerScaling);
	forceIntegerScaling->SetActionCallback(new ForceIntegerScalingAction(this));
	tempLabel = new ui::Label(ui::Point(altFullscreen->Position.X+Graphics::textwidth(forceIntegerScaling->GetText())+20, currentY), ui::Point(1, 16), "\bg- Less blurry");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(forceIntegerScaling);


	class FastQuitAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		FastQuitAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override {
			v->c->SetFastQuit(sender->GetChecked());
		}
	};
	currentY+=20;
	fastquit = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Fast Quit", "");
	autowidth(fastquit);
	fastquit->SetActionCallback(new FastQuitAction(this));
	tempLabel = new ui::Label(ui::Point(fastquit->Position.X+Graphics::textwidth(fastquit->GetText())+20, currentY), ui::Point(1, 16), "\bg- Always exit completely when hitting close");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(fastquit);

	class ShowAvatarsAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		ShowAvatarsAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override {
			v->c->SetShowAvatars(sender->GetChecked());
		}
	};
	currentY+=20;
	showAvatars = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Show Avatars", "");
	autowidth(showAvatars);
	showAvatars->SetActionCallback(new ShowAvatarsAction(this));
	tempLabel = new ui::Label(ui::Point(showAvatars->Position.X+Graphics::textwidth(showAvatars->GetText())+20, currentY), ui::Point(1, 16), "\bg- Disable if you have a slow connection");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(showAvatars);

	class MouseClickRequiredAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		MouseClickRequiredAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override {
			v->c->SetMouseClickrequired(sender->GetChecked());
		}
	};
	currentY+=20;
	mouseClickRequired = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Sticky Categories", "");
	autowidth(mouseClickRequired);
	mouseClickRequired->SetActionCallback(new MouseClickRequiredAction(this));
	tempLabel = new ui::Label(ui::Point(mouseClickRequired->Position.X+Graphics::textwidth(mouseClickRequired->GetText())+20, currentY), ui::Point(1, 16), "\bg- Switch between categories by clicking");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(mouseClickRequired);

	class IncludePressureAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		IncludePressureAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override {
			v->c->SetIncludePressure(sender->GetChecked());
		}
	};
	currentY+=20;
	includePressure = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "Include Pressure", "");
	autowidth(includePressure);
	includePressure->SetActionCallback(new IncludePressureAction(this));
	tempLabel = new ui::Label(ui::Point(includePressure->Position.X+Graphics::textwidth(includePressure->GetText())+20, currentY), ui::Point(1, 16), "\bg- When saving, copying, stamping, etc.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(includePressure);

	class DecoSpaceAction: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		DecoSpaceAction(OptionsView * v): v(v) { }
		void OptionChanged(ui::DropDown * sender, std::pair<String, int> option) override {
			v->c->SetDecoSpace(option.second);
		}
	};
	currentY+=20;
	decoSpace = new ui::DropDown(ui::Point(8, currentY), ui::Point(60, 16));
	decoSpace->SetActionCallback(new DecoSpaceAction(this));
	scrollPanel->AddChild(decoSpace);
	decoSpace->AddOption(std::pair<String, int>("sRGB", 0));
	decoSpace->AddOption(std::pair<String, int>("Linear", 1));
	decoSpace->AddOption(std::pair<String, int>("Gamma 2.2", 2));
	decoSpace->AddOption(std::pair<String, int>("Gamma 1.8", 3));

	tempLabel = new ui::Label(ui::Point(decoSpace->Position.X+decoSpace->Size.X+3, currentY), ui::Point(Size.X-40, 16), "\bg- Colour space used by decoration tools");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	class DataFolderAction: public ui::ButtonAction
	{
	public:
		DataFolderAction() { }
		void ActionCallback(ui::Button * sender) override
		{
//one of these should always be defined
#ifdef WIN
			const char* openCommand = "explorer ";
#elif MACOSX
			const char* openCommand = "open ";
//#elif LIN
#else
			const char* openCommand = "xdg-open ";
#endif
			char* workingDirectory = new char[FILENAME_MAX+strlen(openCommand)];
			sprintf(workingDirectory, "%s\"%s\"", openCommand, getcwd(NULL, 0));
			system(workingDirectory);
			delete[] workingDirectory;
		}
	};
	currentY+=20;
	ui::Button * dataFolderButton = new ui::Button(ui::Point(8, currentY), ui::Point(90, 16), "Open Data Folder");
	dataFolderButton->SetActionCallback(new DataFolderAction());
	scrollPanel->AddChild(dataFolderButton);

	tempLabel = new ui::Label(ui::Point(dataFolderButton->Position.X+dataFolderButton->Size.X+3, currentY), ui::Point(1, 16), "\bg- Open the data and preferences folder");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	class CloseAction: public ui::ButtonAction
	{
	public:
		OptionsView * v;
		CloseAction(OptionsView * v_) { v = v_; }
		void ActionCallback(ui::Button * sender) override
		{
			v->c->Exit();
		}
	};

	ui::Button * tempButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	tempButton->SetActionCallback(new CloseAction(this));
	AddComponent(tempButton);
	SetCancelButton(tempButton);
	SetOkayButton(tempButton);
	currentY+=20;
	scrollPanel->InnerSize = ui::Point(Size.X, currentY);
}

void OptionsView::NotifySettingsChanged(OptionsModel * sender)
{
	heatSimulation->SetChecked(sender->GetHeatSimulation());
	ambientHeatSimulation->SetChecked(sender->GetAmbientHeatSimulation());
	newtonianGravity->SetChecked(sender->GetNewtonianGravity());
	waterEqualisation->SetChecked(sender->GetWaterEqualisation());
	airMode->SetOption(sender->GetAirMode());
	gravityMode->SetOption(sender->GetGravityMode());
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
