#include <iostream>
#include <sstream>
#include "gui/Style.h"
#include "simulation/Simulation.h"
#include "Tool.h"
#include "gui/interface/Window.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/DropDown.h"
#include "gui/interface/Keys.h"
#include "gui/dialogues/ErrorMessage.h"

class PropertyWindow: public ui::Window
{
public:
	ui::DropDown * property;
	ui::Textbox * textField;
	SignTool * tool;
	Simulation * sim;
	int signID;
	ui::Point position;
	std::vector<StructProperty> properties;
	PropertyWindow(PropertyTool * tool_, Simulation * sim_, ui::Point position_);
	void SetProperty();
	virtual void OnDraw();
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnTryExit(ExitMethod method);
	virtual ~PropertyWindow() {}
	class OkayAction: public ui::ButtonAction
	{
	public:
		PropertyWindow * prompt;
		OkayAction(PropertyWindow * prompt_) { prompt = prompt_; }
		void ActionCallback(ui::Button * sender)
		{
			ui::Engine::Ref().CloseWindow();
			if(prompt->textField->GetText().length())
				prompt->SetProperty();
			prompt->SelfDestruct();
			return;
		}
	};
};

PropertyWindow::PropertyWindow(PropertyTool * tool_, Simulation * sim_, ui::Point position_):
ui::Window(ui::Point(-1, -1), ui::Point(200, 87)),
sim(sim_),
position(position_)
{
	properties = Particle::GetProperties();
	
	ui::Label * messageLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 14), "Edit property");
	messageLabel->SetTextColour(style::Colour::InformationTitle);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);
	
	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-17), ui::Point(Size.X, 17), "OK");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	okayButton->SetActionCallback(new OkayAction(this));
	AddComponent(okayButton);
	SetOkayButton(okayButton);
	
	class PropertyChanged: public ui::DropDownAction
	{
		PropertyWindow * w;
	public:
		PropertyChanged(PropertyWindow * w): w(w) { }
		virtual void OptionChanged(ui::DropDown * sender, std::pair<std::string, int> option)
		{
			w->FocusComponent(w->textField);
		}
	};
	property = new ui::DropDown(ui::Point(8, 25), ui::Point(Size.X-16, 17));
	property->SetActionCallback(new PropertyChanged(this));
	AddComponent(property);
	for(int i = 0; i < properties.size(); i++)
	{
		property->AddOption(std::pair<std::string, int>(properties[i].Name, i));
	}
	property->SetOption(0);
	
	textField = new ui::Textbox(ui::Point(8, 46), ui::Point(Size.X-16, 16), "", "[value]");
	textField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	textField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(textField);
	FocusComponent(textField);
	
	ui::Engine::Ref().ShowWindow(this);
}
void PropertyWindow::SetProperty()
{
	if(property->GetOption().second!=-1 && textField->GetText().length() > 0)
	{
		void * propValue;
		int tempInt;
		unsigned int tempUInt;
		float tempFloat;
		std::string value = textField->GetText();
		try {
			switch(properties[property->GetOption().second].Type)
			{
				case StructProperty::Integer:
				case StructProperty::ParticleType:
					if(value.length() > 2 && value.substr(0, 2) == "0x")
					{
						//0xC0FFEE
						std::stringstream buffer;
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer << std::hex << value.substr(2);
						buffer >> tempInt;
					}
					else if(value.length() > 1 && value[0] == '#')
					{
						//#C0FFEE
						std::stringstream buffer;
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer << std::hex << value.substr(1);
						buffer >> tempInt;
					}
					else 
					{
						if(properties[property->GetOption().second].Type == StructProperty::ParticleType)
						{
							int type = sim->GetParticleType(value);
							if(type != -1)
							{
#ifdef DEBUG
								std::cout << "Got type from particle name" << std::endl;
#endif
								tempInt = type;
							}
							else
							{
								std::stringstream buffer(value);
								buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
								buffer >> tempInt;
							}
							if (property->GetOption().first == "type" && (tempInt < 0 || tempInt >= PT_NUM || !sim->elements[tempInt].Enabled))
							{
								new ErrorMessage("Could not set property", "Invalid Particle Type");
								return;
							}
						}
						else
						{
							std::stringstream buffer(value);
							buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
							buffer >> tempInt;
						}
					}
#ifdef DEBUG
					std::cout << "Got int value " << tempInt << std::endl;
#endif
					propValue = &tempInt;
					break;
				case StructProperty::UInteger:
					if(value.length() > 2 && value.substr(0, 2) == "0x")
					{
						//0xC0FFEE
						std::stringstream buffer;
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer << std::hex << value.substr(2);
						buffer >> tempUInt;
					}
					else if(value.length() > 1 && value[0] == '#')
					{
						//#C0FFEE
						std::stringstream buffer;
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer << std::hex << value.substr(1);
						buffer >> tempUInt;
					}
					else 
					{
						std::stringstream buffer(value);
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer >> tempUInt;
					}
#ifdef DEBUG
					std::cout << "Got uint value " << tempUInt << std::endl;
#endif
					propValue = &tempUInt;
					break;
				case StructProperty::Float:
				{
					std::stringstream buffer(value);
					buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
					buffer >> tempFloat;
#ifdef DEBUG
					std::cout << "Got float value " << tempFloat << std::endl;
#endif
					propValue = &tempFloat;
				}
					break;
				default:
					new ErrorMessage("Could not set property", "Invalid property");
					return;
			}
			sim->flood_prop(
							position.X,
							position.Y,
							properties[property->GetOption().second].Offset,
							propValue,
							properties[property->GetOption().second].Type
							);
		} catch (const std::exception& ex) {
			new ErrorMessage("Could not set property", "Invalid value provided");
		}
	}
}

void PropertyWindow::OnTryExit(ExitMethod method)
{
	ui::Engine::Ref().CloseWindow();
	SelfDestruct();
}

void PropertyWindow::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

void PropertyWindow::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if (key == KEY_UP)
		property->SetOption(property->GetOption().second-1);
	else if (key == KEY_DOWN)
		property->SetOption(property->GetOption().second+1);
}

void PropertyTool::Click(Simulation * sim, Brush * brush, ui::Point position)
{
	new PropertyWindow(this, sim, position);
}
