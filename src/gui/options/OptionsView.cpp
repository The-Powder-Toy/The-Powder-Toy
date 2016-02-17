#include <stdio.h>
#ifdef WIN
	#include <direct.h>
	#define getcwd _getcwd
#else
	#include <unistd.h>
#endif
#ifdef USE_SDL
#ifdef SDL_INC
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif
#endif

#include "OptionsView.h"
#include "gui/Style.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/DropDown.h"
#include "gui/dialogues/ErrorMessage.h"
#include "Lang.h"

OptionsView::OptionsView():
	ui::Window(ui::Point(-1, -1), ui::Point(300, 330)){

	ui::Label * tempLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 14), TEXT_GUI_SIM_OPT_TITLE);
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

	heatSimulation = new ui::Checkbox(ui::Point(8, 23), ui::Point(Size.X-6, 16), TEXT_GUI_SIM_OPT_HEATSIM_CHECK, L"");
	heatSimulation->SetActionCallback(new HeatSimulationAction(this));
	AddComponent(heatSimulation);
	tempLabel = new ui::Label(ui::Point(24, heatSimulation->Position.Y+14), ui::Point(Size.X-28, 16), TEXT_GUI_SIM_OPT_HEATSIM_LABEL);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class AmbientHeatSimulationAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		AmbientHeatSimulationAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetAmbientHeatSimulation(sender->GetChecked()); }
	};

	ambientHeatSimulation = new ui::Checkbox(ui::Point(8, 53), ui::Point(Size.X-6, 16), TEXT_GUI_SIM_OPT_AHEAT_CHECK, L"");
	ambientHeatSimulation->SetActionCallback(new AmbientHeatSimulationAction(this));
	AddComponent(ambientHeatSimulation);
	tempLabel = new ui::Label(ui::Point(24, ambientHeatSimulation->Position.Y+14), ui::Point(Size.X-28, 16), TEXT_GUI_SIM_OPT_AHEAT_LABEL);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class NewtonianGravityAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		NewtonianGravityAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetNewtonianGravity(sender->GetChecked()); }
	};

	newtonianGravity = new ui::Checkbox(ui::Point(8, 83), ui::Point(Size.X-6, 16), TEXT_GUI_SIM_OPT_NEWTON_CHECK, L"");
	newtonianGravity->SetActionCallback(new NewtonianGravityAction(this));
	AddComponent(newtonianGravity);
	tempLabel = new ui::Label(ui::Point(24, newtonianGravity->Position.Y+14), ui::Point(Size.X-28, 16), TEXT_GUI_SIM_OPT_NEWTON_LABEL);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class WaterEqualisationAction: public ui::CheckboxAction
		{
			OptionsView * v;
		public:
			WaterEqualisationAction(OptionsView * v_){	v = v_;	}
			virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetWaterEqualisation(sender->GetChecked()); }
		};

	waterEqualisation = new ui::Checkbox(ui::Point(8, 113), ui::Point(Size.X-6, 16), TEXT_GUI_SIM_OPT_WEQUAL_CHECK, L"");
	waterEqualisation->SetActionCallback(new WaterEqualisationAction(this));
	AddComponent(waterEqualisation);
	tempLabel = new ui::Label(ui::Point(24, waterEqualisation->Position.Y+14), ui::Point(Size.X-28, 16), TEXT_GUI_SIM_OPT_WEQUAL_LABEL);
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
	airMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_AIRMODE_OPT_ON, 0));
	airMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_AIRMODE_OPT_PRESSOFF, 1));
	airMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_AIRMODE_OPT_VELOOFF, 2));
	airMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_AIRMODE_OPT_OFF, 3));
	airMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_AIRMODE_OPT_NOUPD, 4));
	airMode->SetActionCallback(new AirModeChanged(this));
		
	tempLabel = new ui::Label(ui::Point(8, 146), ui::Point(Size.X-96, 16), TEXT_GUI_SIM_OPT_AIRMODE_LABEL);
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
	gravityMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_GRAVMODE_OPT_VER, 0));
	gravityMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_GRAVMODE_OPT_OFF, 1));
	gravityMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_GRAVMODE_OPT_RAD, 2));
	gravityMode->SetActionCallback(new GravityModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, 166), ui::Point(Size.X-96, 16), TEXT_GUI_SIM_OPT_GRAVMODE_LABEL);
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
	edgeMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_EDGEMODE_OPT_VOID, 0));
	edgeMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_EDGEMODE_OPT_SOLID, 1));
	edgeMode->AddOption(std::pair<std::wstring, int>(TEXT_GUI_SIM_OPT_EDGEMODE_OPT_LOOP, 2));
	edgeMode->SetActionCallback(new EdgeModeChanged(this));

	tempLabel = new ui::Label(ui::Point(8, 186), ui::Point(Size.X-96, 16), TEXT_GUI_SIM_OPT_EDGEMODE_LABEL);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	class ScaleAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		ScaleAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender){	v->c->SetScale(sender->GetChecked()); }
	};

	scale = new ui::Checkbox(ui::Point(8, 210), ui::Point(Size.X-6, 16), TEXT_GUI_SIM_OPT_LARGE_CHECK, L"");
	scale->SetActionCallback(new ScaleAction(this));
	tempLabel = new ui::Label(ui::Point(scale->Position.X+Graphics::textwidth(scale->GetText().c_str())+20, scale->Position.Y), ui::Point(Size.X-28, 16), TEXT_GUI_SIM_OPT_LARGE_LABEL);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);
	AddComponent(scale);


	class FullscreenAction: public ui::CheckboxAction
	{
		OptionsView * v;
	public:
		FullscreenAction(OptionsView * v_){	v = v_;	}
		virtual void ActionCallback(ui::Checkbox * sender)
		{
#ifdef USE_SDL
#if defined(MACOSX) && !SDL_VERSION_ATLEAST(1, 2, 15)
			ErrorMessage::Blocking(TEXT_GUI_SIM_OPT_FULL_ERR_TITLE, TEXT_GUI_SIM_OPT_FULL_ERR_MSG);
#else
			v->c->SetFullscreen(sender->GetChecked());
#endif
#endif
		}
	};

	fullscreen = new ui::Checkbox(ui::Point(8, 230), ui::Point(Size.X-6, 16), TEXT_GUI_SIM_OPT_FULL_CHECK, L"");
	fullscreen->SetActionCallback(new FullscreenAction(this));
	tempLabel = new ui::Label(ui::Point(fullscreen->Position.X+Graphics::textwidth(fullscreen->GetText().c_str())+20, fullscreen->Position.Y), ui::Point(Size.X-28, 16), TEXT_GUI_SIM_OPT_FULL_LABEL);
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

	fastquit = new ui::Checkbox(ui::Point(8, 250), ui::Point(Size.X-6, 16), TEXT_GUI_SIM_OPT_QUIT_CHECK, L"");
	fastquit->SetActionCallback(new FastQuitAction(this));
	tempLabel = new ui::Label(ui::Point(fastquit->Position.X+Graphics::textwidth(fastquit->GetText().c_str())+20, fastquit->Position.Y), ui::Point(Size.X-28, 16), TEXT_GUI_SIM_OPT_QUIT_LABEL);
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

	showAvatars = new ui::Checkbox(ui::Point(8, 270), ui::Point(Size.X-6, 16), TEXT_GUI_SIM_OPT_AVATAR_CHECK, L"");
	showAvatars->SetActionCallback(new ShowAvatarsAction(this));
	tempLabel = new ui::Label(ui::Point(showAvatars->Position.X+Graphics::textwidth(showAvatars->GetText().c_str())+20, showAvatars->Position.Y), ui::Point(Size.X-28, 16), TEXT_GUI_SIM_OPT_AVATAR_LABEL);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);
	AddComponent(showAvatars);

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
	ui::Button * dataFolderButton = new ui::Button(ui::Point(8, Size.Y-38), ui::Point(90, 16), TEXT_GUI_SIM_OPT_FOLDER_BTN);
	dataFolderButton->SetActionCallback(new DataFolderAction());
	AddComponent(dataFolderButton);

	tempLabel = new ui::Label(ui::Point(dataFolderButton->Position.X+dataFolderButton->Size.X+3, dataFolderButton->Position.Y), ui::Point(Size.X-28, 16), TEXT_GUI_SIM_OPT_FOLDER_LABEL);
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

	ui::Button * tempButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), TEXT_GUI_SIM_OPT_BTN_OK);
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
	scale->SetChecked(sender->GetScale());
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
	Graphics * g = ui::Engine::Ref().g;
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

