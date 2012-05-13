#include <iostream>
#include "simulation/Simulation.h"
#include "Tool.h"
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Label.h"
#include "interface/Textbox.h"

class SignWindow: public ui::Window
{
public:
	ui::Textbox * textField;
	SignTool * tool;
	Simulation * sim;
	int signID;
	ui::Point signPosition;
	SignWindow(SignTool * tool_, Simulation * sim_, int signID_, ui::Point position_);
	virtual void OnDraw();
	virtual ~SignWindow() {}
};

class OkayAction: public ui::ButtonAction
{
public:
	SignWindow * prompt;
	OkayAction(SignWindow * prompt_) { prompt = prompt_; }
	void ActionCallback(ui::Button * sender)
	{
		ui::Engine::Ref().CloseWindow();		
		if(prompt->signID==-1 && prompt->textField->GetText().length())
		{
			prompt->sim->signs.push_back(sign(prompt->textField->GetText(), prompt->signPosition.X, prompt->signPosition.Y, sign::Left));
		}
		else if(prompt->textField->GetText().length())
		{
			prompt->sim->signs[prompt->signID] = sign(sign(prompt->textField->GetText(), prompt->signPosition.X, prompt->signPosition.Y, sign::Left));
		}
		prompt->SelfDestruct();
	}
};

SignWindow::SignWindow(SignTool * tool_, Simulation * sim_, int signID_, ui::Point position_):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 75)),
	tool(tool_),
	signID(signID_),
	sim(sim_),
	signPosition(position_)
{
	ui::Label * messageLabel = new ui::Label(ui::Point(4, 18), ui::Point(Size.X-8, 60), "New sign");
	messageLabel->SetAlignment(AlignLeft, AlignTop);
	AddComponent(messageLabel);
	
	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	okayButton->SetAlignment(AlignLeft, AlignBottom);
	okayButton->SetBorderColour(ui::Colour(200, 200, 200));
	okayButton->SetActionCallback(new OkayAction(this));
	AddComponent(okayButton);
	
	textField = new ui::Textbox(ui::Point(4, 32), ui::Point(Size.X-8, 16), "");
	textField->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(textField);
	
	ui::Engine::Ref().ShowWindow(this);
}
void SignWindow::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	
	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

void SignTool::Click(Simulation * sim, Brush * brush, ui::Point position)
{
	new SignWindow(this, sim, -1, position);
}