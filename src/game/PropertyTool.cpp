#include <iostream>
#include <sstream>
#include "Style.h"
#include "simulation/Simulation.h"
#include "Tool.h"
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "interface/DropDown.h"
#include "dialogues/ErrorMessage.h"

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
	messageLabel->SetAlignment(AlignLeft, AlignTop);
	AddComponent(messageLabel);
	
	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 17), "OK");
	okayButton->SetAlignment(AlignLeft, AlignBottom);
	okayButton->SetBorderColour(ui::Colour(200, 200, 200));
	okayButton->SetActionCallback(new OkayAction(this));
	AddComponent(okayButton);
	
	property = new ui::DropDown(ui::Point(8, 25), ui::Point(Size.X-16, 17));
	AddComponent(property);
	for(int i = 0; i < properties.size(); i++)
	{
		property->AddOption(std::pair<std::string, int>(properties[i].Name, i));
	}
	property->SetOption(0);
	
	textField = new ui::Textbox(ui::Point(8, 46), ui::Point(Size.X-16, 16), "");
	textField->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(textField);
	
	ui::Engine::Ref().ShowWindow(this);
}
void PropertyWindow::SetProperty()
{
	if(property->GetOption().second!=-1)
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
						stringstream buffer;
						buffer << std::hex << value.substr(2);
						buffer >> tempInt;
					}
					else if(value.length() > 1 && value[0] == '#')
					{
						//#C0FFEE
						stringstream buffer;
						buffer << std::hex << value.substr(1);
						buffer >> tempInt;
					}
					else 
					{
						istringstream(value) >> tempInt;
					}
					propValue = &tempInt;
					break;
				case StructProperty::UInteger:
					if(value.length() > 2 && value.substr(0, 2) == "0x")
					{
						//0xC0FFEE
						stringstream buffer;
						buffer << std::hex << value.substr(2);
						buffer >> tempUInt;
					}
					else if(value.length() > 1 && value[0] == '#')
					{
						//#C0FFEE
						stringstream buffer;
						buffer << std::hex << value.substr(1);
						buffer >> tempUInt;
					}
					else 
					{
						istringstream(value) >> tempUInt;
					}
					propValue = &tempUInt;
					break;
				case StructProperty::Float:
					istringstream(value) >> tempFloat;
					propValue = &tempFloat;
					break;
				default:
					new ErrorMessage("Could not set property", "Invalid property");
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
void PropertyWindow::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	
	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

void PropertyTool::Click(Simulation * sim, Brush * brush, ui::Point position)
{
	new PropertyWindow(this, sim, position);
}