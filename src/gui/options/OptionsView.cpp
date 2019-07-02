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
#include "gui/interface/DropDown.h"

#include "graphics/Graphics.h"

OptionsView::OptionsView():
	ui::Window(ui::Point(-1, -1), ui::Point(350, 389)){

	int currentY = 400;
	scrollPanel = new ui::ScrollPanel(ui::Point(-1, -1), ui::Point(Size.X, Size.Y-16));
	
	AddComponent(scrollPanel);

	ui::Label * tempLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 14), "Simulation Options");
	tempLabel->SetTextColour(style::Colour::InformationTitle);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	

	class HeatSimulationAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		HeatSimulationAction(OptionsView * v_){	v = v_;	}
		void ActionCallback(ui::Checkbox * sender) override {
			v->c->SetHeatSimulation(sender->GetChecked());
		}
	};

	heatSimulation = new ui::Checkbox(ui::Point(8, 23), ui::Point(Size.X-6, 16), "Heat simulation \bgIntroduced in version 34", "");
	heatSimulation->SetActionCallback(new HeatSimulationAction(this));
	scrollPanel->AddChild(heatSimulation);
	tempLabel = new ui::Label(ui::Point(24, heatSimulation->Position.Y+14), ui::Point(Size.X-28, 16), "\bgCan cause odd behaviour when disabled");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	ambientHeatSimulation = new ui::Checkbox(ui::Point(8, 53), ui::Point(Size.X-6, 16), "Ambient heat simulation \bgIntroduced in version 50", "");
	ambientHeatSimulation->SetActionCallback(new AmbientHeatSimulationAction(this));
	scrollPanel->AddChild(ambientHeatSimulation);
	tempLabel = new ui::Label(ui::Point(24, ambientHeatSimulation->Position.Y+14), ui::Point(Size.X-28, 16), "\bgCan cause odd / broken behaviour with many saves");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	newtonianGravity = new ui::Checkbox(ui::Point(8, 83), ui::Point(Size.X-6, 16), "Newtonian gravity \bgIntroduced in version 48", "");
	newtonianGravity->SetActionCallback(new NewtonianGravityAction(this));
	scrollPanel->AddChild(newtonianGravity);
	tempLabel = new ui::Label(ui::Point(24, newtonianGravity->Position.Y+14), ui::Point(Size.X-28, 16), "\bgMay cause poor performance on older computers");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	waterEqualisation = new ui::Checkbox(ui::Point(8, 113), ui::Point(Size.X-6, 16), "Water equalisation \bgIntroduced in version 61", "");
	waterEqualisation->SetActionCallback(new WaterEqualisationAction(this));
	scrollPanel->AddChild(waterEqualisation);
	tempLabel = new ui::Label(ui::Point(24, waterEqualisation->Position.Y+14), ui::Point(Size.X-28, 16), "\bgMay cause poor performance with a lot of water");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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
	airMode = new ui::DropDown(ui::Point(Size.X-88, 146), ui::Point(80, 16));
	scrollPanel->AddChild(airMode);
	airMode->AddOption(std::pair<String, int>("On", 0));
	airMode->AddOption(std::pair<String, int>("Pressure off", 1));
	airMode->AddOption(std::pair<String, int>("Velocity off", 2));
	airMode->AddOption(std::pair<String, int>("Off", 3));
	airMode->AddOption(std::pair<String, int>("No Update", 4));
	airMode->SetActionCallback(new AirModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, 146), ui::Point(Size.X-96, 16), "Air Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	gravityMode = new ui::DropDown(ui::Point(Size.X-88, 166), ui::Point(80, 16));
	scrollPanel->AddChild(gravityMode);
	gravityMode->AddOption(std::pair<String, int>("Vertical", 0));
	gravityMode->AddOption(std::pair<String, int>("Off", 1));
	gravityMode->AddOption(std::pair<String, int>("Radial", 2));
	gravityMode->SetActionCallback(new GravityModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, 166), ui::Point(Size.X-96, 16), "Gravity Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	edgeMode = new ui::DropDown(ui::Point(Size.X-88, 186), ui::Point(80, 16));
	scrollPanel->AddChild(edgeMode);
	edgeMode->AddOption(std::pair<String, int>("Void", 0));
	edgeMode->AddOption(std::pair<String, int>("Solid", 1));
	edgeMode->AddOption(std::pair<String, int>("Loop", 2));
	edgeMode->SetActionCallback(new EdgeModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, 186), ui::Point(Size.X-96, 16), "Edge Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	class ScaleAction: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		ScaleAction(OptionsView * v): v(v) { }
		void OptionChanged(ui::DropDown * sender, std::pair<String, int> option) override {
			v->c->SetScale(option.second);
		}
	};
	scale = new ui::DropDown(ui::Point(8, 210), ui::Point(40, 16));
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

	tempLabel = new ui::Label(ui::Point(scale->Position.X+scale->Size.X+3, scale->Position.Y), ui::Point(Size.X-28, 16), "\bg- Window scale factor for larger screens");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	resizable = new ui::Checkbox(ui::Point(8, scale->Position.Y + 20), ui::Point(Size.X-6, 16), "Resizable", "");
	resizable->SetActionCallback(new ResizableAction(this));
	tempLabel = new ui::Label(ui::Point(resizable->Position.X+Graphics::textwidth(resizable->GetText().c_str())+20, resizable->Position.Y), ui::Point(Size.X-28, 16), "\bg- Allow resizing and maximizing window");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	fullscreen = new ui::Checkbox(ui::Point(8, resizable->Position.Y + 20), ui::Point(Size.X-6, 16), "Fullscreen", "");
	fullscreen->SetActionCallback(new FullscreenAction(this));
	tempLabel = new ui::Label(ui::Point(fullscreen->Position.X+Graphics::textwidth(fullscreen->GetText().c_str())+20, fullscreen->Position.Y), ui::Point(Size.X-28, 16), "\bg- Fill the entire screen");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	altFullscreen = new ui::Checkbox(ui::Point(23, fullscreen->Position.Y + 20), ui::Point(Size.X-6, 16), "Change Resolution", "");
	altFullscreen->SetActionCallback(new AltFullscreenAction(this));
	tempLabel = new ui::Label(ui::Point(altFullscreen->Position.X+Graphics::textwidth(altFullscreen->GetText().c_str())+20, altFullscreen->Position.Y), ui::Point(Size.X-28, 16), "\bg- Set optimial screen resolution");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	forceIntegerScaling = new ui::Checkbox(ui::Point(23, altFullscreen->Position.Y + 20), ui::Point(Size.X-6, 16), "Force Integer Scaling", "");
	forceIntegerScaling->SetActionCallback(new ForceIntegerScalingAction(this));
	tempLabel = new ui::Label(ui::Point(altFullscreen->Position.X+Graphics::textwidth(forceIntegerScaling->GetText().c_str())+20, forceIntegerScaling->Position.Y), ui::Point(Size.X-28, 16), "\bg- less blurry");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	fastquit = new ui::Checkbox(ui::Point(8, forceIntegerScaling->Position.Y + 20), ui::Point(Size.X-6, 16), "Fast Quit", "");
	fastquit->SetActionCallback(new FastQuitAction(this));
	tempLabel = new ui::Label(ui::Point(fastquit->Position.X+Graphics::textwidth(fastquit->GetText().c_str())+20, fastquit->Position.Y), ui::Point(Size.X-28, 16), "\bg- Always exit completely when hitting close");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	showAvatars = new ui::Checkbox(ui::Point(8, fastquit->Position.Y + 20), ui::Point(Size.X-6, 16), "Show Avatars", "");
	showAvatars->SetActionCallback(new ShowAvatarsAction(this));
	tempLabel = new ui::Label(ui::Point(showAvatars->Position.X+Graphics::textwidth(showAvatars->GetText().c_str())+20, showAvatars->Position.Y), ui::Point(Size.X-28, 16), "\bg- Disable if you have a slow connection");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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

	mouseClickRequired = new ui::Checkbox(ui::Point(8, showAvatars->Position.Y + 20), ui::Point(Size.X-6, 16), "Mouse click required", "");
	mouseClickRequired->SetActionCallback(new MouseClickRequiredAction(this));
	tempLabel = new ui::Label(ui::Point(mouseClickRequired->Position.X+Graphics::textwidth(mouseClickRequired->GetText().c_str())+20, mouseClickRequired->Position.Y), ui::Point(Size.X-28, 16), "\bg- click required to change category in menu");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(mouseClickRequired);

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
	ui::Button * dataFolderButton = new ui::Button(ui::Point(8, mouseClickRequired->Position.Y + 20), ui::Point(90, 16), "Open Data Folder");
	dataFolderButton->SetActionCallback(new DataFolderAction());
	scrollPanel->AddChild(dataFolderButton);

	tempLabel = new ui::Label(ui::Point(dataFolderButton->Position.X+dataFolderButton->Size.X+3, dataFolderButton->Position.Y), ui::Point(Size.X-28, 16), "\bg- Open the data and preferences folder");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
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
	edgeMode->SetOption(sender->GetEdgeMode());
	scale->SetOption(sender->GetScale());
	resizable->SetChecked(sender->GetResizable());
	fullscreen->SetChecked(sender->GetFullscreen());
	altFullscreen->SetChecked(sender->GetAltFullscreen());
	forceIntegerScaling->SetChecked(sender->GetForceIntegerScaling());
	fastquit->SetChecked(sender->GetFastQuit());
	showAvatars->SetChecked(sender->GetShowAvatars());
	mouseClickRequired->SetChecked(sender->GetMouseClickRequired());
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
	g->draw_line(Position.X+1, Position.Y+scale->Position.Y-4, Position.X+Size.X-1, Position.Y+scale->Position.Y-4, 255, 255, 255, 180);
}

void OptionsView::OnTryExit(ExitMethod method)
{
	c->Exit();
}


OptionsView::~OptionsView() {
}
