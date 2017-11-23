#include <cstdio>
#ifdef WIN
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif
#include "SDLCompat.h"

#include "OptionsView.h"
#include "Format.h"
#include "gui/Style.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/DropDown.h"
#include "gui/interface/Engine.h"
#include "gui/dialogues/ErrorMessage.h"

OptionsView::OptionsView():
	ui::Window(ui::Point(-1, -1), ui::Point(300, 348)){

	ui::Label * tempLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 14), "Simulation Options");
	tempLabel->SetTextColour(style::Colour::InformationTitle);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class HeatSimulationAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		HeatSimulationAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetHeatSimulation(sender->GetChecked()); }
	};

	heatSimulation = new ui::Checkbox(ui::Point(8, 23), ui::Point(Size.X-6, 16), "Heat simulation \bgIntroduced in version 34", "");
	heatSimulation->SetActionCallback(new HeatSimulationAction(this));
	AddComponent(heatSimulation);
	tempLabel = new ui::Label(ui::Point(24, heatSimulation->Position.Y+14), ui::Point(Size.X-28, 16), "\bgCan cause odd behaviour when disabled");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class AmbientHeatSimulationAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		AmbientHeatSimulationAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetAmbientHeatSimulation(sender->GetChecked()); }
	};

	ambientHeatSimulation = new ui::Checkbox(ui::Point(8, 53), ui::Point(Size.X-6, 16), "Ambient heat simulation \bgIntroduced in version 50", "");
	ambientHeatSimulation->SetActionCallback(new AmbientHeatSimulationAction(this));
	AddComponent(ambientHeatSimulation);
	tempLabel = new ui::Label(ui::Point(24, ambientHeatSimulation->Position.Y+14), ui::Point(Size.X-28, 16), "\bgCan cause odd / broken behaviour with many saves");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class NewtonianGravityAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		NewtonianGravityAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetNewtonianGravity(sender->GetChecked()); }
	};

	newtonianGravity = new ui::Checkbox(ui::Point(8, 83), ui::Point(Size.X-6, 16), "Newtonian gravity \bgIntroduced in version 48", "");
	newtonianGravity->SetActionCallback(new NewtonianGravityAction(this));
	AddComponent(newtonianGravity);
	tempLabel = new ui::Label(ui::Point(24, newtonianGravity->Position.Y+14), ui::Point(Size.X-28, 16), "\bgMay cause poor performance on older computers");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class WaterEqualisationAction: public ui::CheckboxAction
		{
			OptionsView * v;
		public:
			WaterEqualisationAction(OptionsView * v_){	v = v_;	}
			virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetWaterEqualisation(sender->GetChecked()); }
		};

	waterEqualisation = new ui::Checkbox(ui::Point(8, 113), ui::Point(Size.X-6, 16), "Water equalisation \bgIntroduced in version 61", "");
	waterEqualisation->SetActionCallback(new WaterEqualisationAction(this));
	AddComponent(waterEqualisation);
	tempLabel = new ui::Label(ui::Point(24, waterEqualisation->Position.Y+14), ui::Point(Size.X-28, 16), "\bgMay cause poor performance with a lot of water");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class AirModeChanged: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		AirModeChanged(OptionsView * v): v(v) { }
		virtual void OptionChanged(ui::DropDown * sender, std::pair<std::string, int> option) { v->c->SetAirMode(option.second); }
	};
	airMode = new ui::DropDown(ui::Point(Size.X-88, 146), ui::Point(80, 16));
	AddComponent(airMode);
	airMode->AddOption(std::pair<std::string, int>("On", 0));
	airMode->AddOption(std::pair<std::string, int>("Pressure off", 1));
	airMode->AddOption(std::pair<std::string, int>("Velocity off", 2));
	airMode->AddOption(std::pair<std::string, int>("Off", 3));
	airMode->AddOption(std::pair<std::string, int>("No Update", 4));
	airMode->SetActionCallback(new AirModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, 146), ui::Point(Size.X-96, 16), "Air Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class GravityModeChanged: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		GravityModeChanged(OptionsView * v): v(v) { }
		virtual void OptionChanged(ui::DropDown * sender, std::pair<std::string, int> option) { v->c->SetGravityMode(option.second); }
	};

	gravityMode = new ui::DropDown(ui::Point(Size.X-88, 166), ui::Point(80, 16));
	AddComponent(gravityMode);
	gravityMode->AddOption(std::pair<std::string, int>("Vertical", 0));
	gravityMode->AddOption(std::pair<std::string, int>("Off", 1));
	gravityMode->AddOption(std::pair<std::string, int>("Radial", 2));
	gravityMode->SetActionCallback(new GravityModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, 166), ui::Point(Size.X-96, 16), "Gravity Simulation Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class EdgeModeChanged: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		EdgeModeChanged(OptionsView * v): v(v) { }
		virtual void OptionChanged(ui::DropDown * sender, std::pair<std::string, int> option) { v->c->SetEdgeMode(option.second); }
	};

	edgeMode = new ui::DropDown(ui::Point(Size.X-88, 186), ui::Point(80, 16));
	AddComponent(edgeMode);
	edgeMode->AddOption(std::pair<std::string, int>("Void", 0));
	edgeMode->AddOption(std::pair<std::string, int>("Solid", 1));
	edgeMode->AddOption(std::pair<std::string, int>("Loop", 2));
	edgeMode->SetActionCallback(new EdgeModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, 186), ui::Point(Size.X-96, 16), "Edge Mode");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class ScaleAction: public ui::DropDownAction
	{
		OptionsView * v;
	public:
		ScaleAction(OptionsView * v): v(v) { }
		virtual void OptionChanged(ui::DropDown * sender, std::pair<std::string, int> option) { v->c->SetScale(option.second); }
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
			scale->AddOption(std::pair<std::string, int>(format::NumberToString<int>(ix_scale), ix_scale));
			ix_scale += 1;
		}
		while (ui::Engine::Ref().GetMaxWidth() >= ui::Engine::Ref().GetWidth() * ix_scale && ui::Engine::Ref().GetMaxHeight() >= ui::Engine::Ref().GetHeight() * ix_scale);
		if (!current_scale_valid)
			scale->AddOption(std::pair<std::string, int>("current", current_scale));
	}
	scale->SetActionCallback(new ScaleAction(this));
	AddComponent(scale);

	tempLabel = new ui::Label(ui::Point(scale->Position.X+scale->Size.X+3, scale->Position.Y), ui::Point(Size.X-28, 16), "\bg- Window scale factor for larger screens");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);


	class FullscreenAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		FullscreenAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender)
		{
#ifdef USE_SDL
#if defined(MACOSX) && !SDL_VERSION_ATLEAST(1, 2, 15)
			ErrorMessage::Blocking("Information", "Fullscreen doesn't work on OS X");
#else
			v->c->SetFullscreen(sender->GetChecked());
#endif
#endif
		}
	};

	fullscreen = new ui::Checkbox(ui::Point(8, 230), ui::Point(Size.X-6, 16), "Fullscreen", "");
	fullscreen->SetActionCallback(new FullscreenAction(this));
	tempLabel = new ui::Label(ui::Point(fullscreen->Position.X+Graphics::textwidth(fullscreen->GetText().c_str())+20, fullscreen->Position.Y), ui::Point(Size.X-28, 16), "\bg- Fill the entire screen");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);
	AddComponent(fullscreen);


	class FastQuitAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		FastQuitAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetFastQuit(sender->GetChecked()); }
	};

	fastquit = new ui::Checkbox(ui::Point(8, 250), ui::Point(Size.X-6, 16), "Fast Quit", "");
	fastquit->SetActionCallback(new FastQuitAction(this));
	tempLabel = new ui::Label(ui::Point(fastquit->Position.X+Graphics::textwidth(fastquit->GetText().c_str())+20, fastquit->Position.Y), ui::Point(Size.X-28, 16), "\bg- Always exit completely when hitting close");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);
	AddComponent(fastquit);

	class ShowAvatarsAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		ShowAvatarsAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetShowAvatars(sender->GetChecked()); }
	};

	showAvatars = new ui::Checkbox(ui::Point(8, 270), ui::Point(Size.X-6, 16), "Show Avatars", "");
	showAvatars->SetActionCallback(new ShowAvatarsAction(this));
	tempLabel = new ui::Label(ui::Point(showAvatars->Position.X+Graphics::textwidth(showAvatars->GetText().c_str())+20, showAvatars->Position.Y), ui::Point(Size.X-28, 16), "\bg- Disable if you have a slow connection");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);
	AddComponent(showAvatars);

	class DepthAction: public ui::TextboxAction
	{
		OptionsView * v;
	public:
		DepthAction(OptionsView * v_) { v = v_; }
		virtual void TextChangedCallback(ui::Textbox * sender) { v->c->Set3dDepth(format::StringToNumber<int>(sender->GetText())); }
	};
	depthTextbox = new ui::Textbox(ui::Point(8, Size.Y-58), ui::Point(25, 16), format::NumberToString<int>(ui::Engine::Ref().Get3dDepth()));
	depthTextbox->SetInputType(ui::Textbox::Numeric);
	depthTextbox->SetActionCallback(new DepthAction(this));
	AddComponent(depthTextbox);

	tempLabel = new ui::Label(ui::Point(depthTextbox->Position.X+depthTextbox->Size.X+3, depthTextbox->Position.Y), ui::Point(Size.X-28, 16), "\bg- Change the depth of the 3D anaglyph effect");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class DataFolderAction: public ui::ButtonAction
	{
	public:
		DataFolderAction() { }
		void ActionCallback(ui::Button * sender)
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
	ui::Button * dataFolderButton = new ui::Button(ui::Point(8, Size.Y-38), ui::Point(90, 16), "Open Data Folder");
	dataFolderButton->SetActionCallback(new DataFolderAction());
	AddComponent(dataFolderButton);

	tempLabel = new ui::Label(ui::Point(dataFolderButton->Position.X+dataFolderButton->Size.X+3, dataFolderButton->Position.Y), ui::Point(Size.X-28, 16), "\bg- Open the data and preferences folder");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class CloseAction: public ui::ButtonAction
	{
	public:
		OptionsView * v;
		CloseAction(OptionsView * v_) { v = v_; }
		void ActionCallback(ui::Button * sender)
		{
			v->c->Exit();
		}
	};

	ui::Button * tempButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	tempButton->SetActionCallback(new CloseAction(this));
	AddComponent(tempButton);
	SetCancelButton(tempButton);
	SetOkayButton(tempButton);
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
	fullscreen->SetChecked(sender->GetFullscreen());
	fastquit->SetChecked(sender->GetFastQuit());
	showAvatars->SetChecked(sender->GetShowAvatars());
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
