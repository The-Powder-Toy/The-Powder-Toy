#include <iostream>
#include "Style.h"
#include "simulation/Simulation.h"
#include "Tool.h"
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "interface/DropDown.h"

class PropertyWindow: public ui::Window
{
public:
	ui::DropDown * property;
	ui::Textbox * textField;
	SignTool * tool;
	Simulation * sim;
	int signID;
	ui::Point position;
	PropertyWindow(PropertyTool * tool_, Simulation * sim_, ui::Point position_);
	virtual void OnDraw();
	virtual ~PropertyWindow() {}
};

class OkayAction: public ui::ButtonAction
{
public:
	PropertyWindow * prompt;
	OkayAction(PropertyWindow * prompt_) { prompt = prompt_; }
	void ActionCallback(ui::Button * sender)
	{
		ui::Engine::Ref().CloseWindow();		

		prompt->SelfDestruct();
	}
};

PropertyWindow::PropertyWindow(PropertyTool * tool_, Simulation * sim_, ui::Point position_):
ui::Window(ui::Point(-1, -1), ui::Point(200, 87)),
sim(sim_),
position(position_)
{
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
	property->AddOption(std::pair<std::string, int>("Left", (int)sign::Left));
	property->AddOption(std::pair<std::string, int>("Centre", (int)sign::Centre));
	property->AddOption(std::pair<std::string, int>("Right", (int)sign::Right));
	property->SetOption(0);
	
	textField = new ui::Textbox(ui::Point(8, 46), ui::Point(Size.X-16, 16), "");
	textField->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(textField);
	
	ui::Engine::Ref().ShowWindow(this);
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