#include <iostream>
#include "Style.h"
#include "simulation/Simulation.h"
#include "Tool.h"
#include "interface/Window.h"
#include "interface/Button.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "interface/DropDown.h"

class SignWindow: public ui::Window
{
public:
	ui::DropDown * justification;
	ui::Textbox * textField;
	SignTool * tool;
	sign * movingSign;
	bool signMoving;
	Simulation * sim;
	int signID;
	ui::Point signPosition;
	SignWindow(SignTool * tool_, Simulation * sim_, int signID_, ui::Point position_);
	virtual void OnDraw();
	virtual void DoDraw();
	virtual void DoMouseMove(int x, int y, int dx, int dy);
	virtual void DoMouseDown(int x, int y, unsigned button);
	virtual void DoMouseUp(int x, int y, unsigned button) { if(!signMoving) ui::Window::DoMouseUp(x, y, button); }
	virtual void DoMouseWheel(int x, int y, int d) { if(!signMoving) ui::Window::DoMouseWheel(x, y, d); }
	virtual void DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt) { if(!signMoving) ui::Window::DoKeyPress(key, character, shift, ctrl, alt); };
	virtual void DoKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt) { if(!signMoving) ui::Window::DoKeyRelease(key, character, shift, ctrl, alt); };
	virtual ~SignWindow() {}
	virtual void OnTryExit(ui::Window::ExitMethod method);
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
				prompt->sim->signs.push_back(sign(prompt->textField->GetText(), prompt->signPosition.X, prompt->signPosition.Y, (sign::Justification)prompt->justification->GetOption().second));
			}
			else if(prompt->signID!=-1 && prompt->textField->GetText().length())
			{
				prompt->sim->signs[prompt->signID] = sign(sign(prompt->textField->GetText(), prompt->signPosition.X, prompt->signPosition.Y, (sign::Justification)prompt->justification->GetOption().second));
			}
			prompt->SelfDestruct();
		}
	};
	class DeleteAction: public ui::ButtonAction
	{
	public:
		SignWindow * prompt;
		DeleteAction(SignWindow * prompt_) { prompt = prompt_; }
		void ActionCallback(ui::Button * sender)
		{
			ui::Engine::Ref().CloseWindow();
			if(prompt->signID!=-1)
			{
				prompt->sim->signs.erase(prompt->sim->signs.begin()+prompt->signID);
			}
			prompt->SelfDestruct();
		}
	};

	class SignTextAction: public ui::TextboxAction
	{
	public:
		SignWindow * prompt;
		SignTextAction(SignWindow * prompt_) { prompt = prompt_; }
		virtual void TextChangedCallback(ui::Textbox * sender)
		{
			if(prompt->signID!=-1)
			{
				prompt->sim->signs[prompt->signID].text = sender->GetText();
			}
		}
	};

	class MoveAction: public ui::ButtonAction
	{
	public:
		SignWindow * prompt;
		MoveAction(SignWindow * prompt_) { prompt = prompt_; }
		void ActionCallback(ui::Button * sender)
		{
			if(prompt->signID!=-1)
			{
				prompt->movingSign = &prompt->sim->signs[prompt->signID];
				prompt->signMoving = true;
			}
		}
	};
};

SignWindow::SignWindow(SignTool * tool_, Simulation * sim_, int signID_, ui::Point position_):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 87)),
	tool(tool_),
	signID(signID_),
	sim(sim_),
	signPosition(position_),
	movingSign(NULL),
	signMoving(false)
{
	ui::Label * messageLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 15), "New sign");
	messageLabel->SetTextColour(style::Colour::InformationTitle);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(messageLabel);

	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.BorderInactive = (ui::Colour(200, 200, 200));
	okayButton->SetActionCallback(new OkayAction(this));
	AddComponent(okayButton);
	SetOkayButton(okayButton);
	
	ui::Label * tempLabel = new ui::Label(ui::Point(8, 48), ui::Point(40, 15), "Justify:");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	justification = new ui::DropDown(ui::Point(52, 48), ui::Point(50, 16));
	AddComponent(justification);
	justification->AddOption(std::pair<std::string, int>("\x9D Left", (int)sign::Left));
	justification->AddOption(std::pair<std::string, int>("\x9E Centre", (int)sign::Centre));
	justification->AddOption(std::pair<std::string, int>("\x9F Right", (int)sign::Right));
	justification->SetOption(1);
	justification->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	
	textField = new ui::Textbox(ui::Point(8, 25), ui::Point(Size.X-16, 17), "", "[message]");
	textField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	textField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	textField->SetActionCallback(new SignTextAction(this));
	AddComponent(textField);
	FocusComponent(textField);
	
	if(signID!=-1)
	{
		messageLabel->SetText("Edit sign");

		textField->SetText(sim->signs[signID].text);
		justification->SetOption(sim->signs[signID].ju);

		ui::Point position = ui::Point(justification->Position.X+justification->Size.X+3, 48);
		ui::Button * moveButton = new ui::Button(position, ui::Point(((Size.X-position.X-8)/2)-2, 16), "Move");
		moveButton->SetActionCallback(new MoveAction(this));
		AddComponent(moveButton);

		position = ui::Point(justification->Position.X+justification->Size.X+3, 48)+ui::Point(moveButton->Size.X+3, 0);
		ui::Button * deleteButton = new ui::Button(position, ui::Point((Size.X-position.X-8)-1, 16), "Delete");
		//deleteButton->SetIcon(IconDelete);
		deleteButton->SetActionCallback(new DeleteAction(this));

		signPosition.X = sim->signs[signID].x;
		signPosition.Y = sim->signs[signID].y;

		AddComponent(deleteButton);
	}

	ui::Engine::Ref().ShowWindow(this);
}

void SignWindow::OnTryExit(ui::Window::ExitMethod method)
{
	ui::Engine::Ref().CloseWindow();
	SelfDestruct();
}

void SignWindow::DoDraw()
{
	for(std::vector<sign>::iterator iter = sim->signs.begin(), end = sim->signs.end(); iter != end; ++iter)
	{
		sign & currentSign = *iter;
		int x, y, w, h;
		Graphics * g = ui::Engine::Ref().g;
		char buff[256];  //Buffer
		currentSign.pos(x, y, w, h);
		g->clearrect(x, y, w, h);
		g->drawrect(x, y, w, h, 192, 192, 192, 255);

		//Displaying special information
		if (currentSign.text == "{p}")
		{
			float pressure = 0.0f;
			if (currentSign.x>=0 && currentSign.x<XRES && currentSign.y>=0 && currentSign.y<YRES)
				pressure = sim->pv[currentSign.y/CELL][currentSign.x/CELL];
			sprintf(buff, "Pressure: %3.2f", pressure);  //...pressure
			g->drawtext(x+3, y+3, buff, 255, 255, 255, 255);
		}
		else if (currentSign.text == "{t}")
		{
			if (currentSign.x>=0 && currentSign.x<XRES && currentSign.y>=0 && currentSign.y<YRES && sim->pmap[currentSign.y][currentSign.x])
				sprintf(buff, "Temp: %4.2f", sim->parts[sim->pmap[currentSign.y][currentSign.x]>>8].temp-273.15);  //...temperature
			else
				sprintf(buff, "Temp: 0.00");  //...temperature
			g->drawtext(x+3, y+3, buff, 255, 255, 255, 255);
		}
		else if (sregexp(currentSign.text.c_str(), "^{c:[0-9]*|.*}$")==0)
		{
			int sldr, startm;
			memset(buff, 0, sizeof(buff));
			for (sldr=3; currentSign.text[sldr-1] != '|'; sldr++)
				startm = sldr + 1;
			sldr = startm;
			while (currentSign.text[sldr] != '}')
			{
				buff[sldr - startm] = currentSign.text[sldr];
				sldr++;
			}
			g->drawtext(x+3, y+3, buff, 0, 191, 255, 255);
		}
		else 
		{
			g->drawtext(x+3, y+3, currentSign.text, 255, 255, 255, 255);
		}
	}
	if(!signMoving)
	{
		ui::Window::DoDraw();
	}
}

void SignWindow::DoMouseMove(int x, int y, int dx, int dy) {
	if(!signMoving)
		ui::Window::DoMouseMove(x, y, dx, dy);
	else
	{
		if(x < XRES && y < YRES)
		{
			movingSign->x = x;
			movingSign->y = y;
			signPosition.X = x;
			signPosition.Y = y;
		}
	}
}

void SignWindow::DoMouseDown(int x, int y, unsigned button)
{
	if(!signMoving)
		ui::Window::DoMouseDown(x, y, button);
	else
	{
		signMoving = false;
	}
}

void SignWindow::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	
	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

VideoBuffer * SignTool::GetIcon(int toolID, int width, int height)
{
	VideoBuffer * newTexture = new VideoBuffer(width, height);
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			pixel pc =  x==0||x==width-1||y==0||y==height-1 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
			newTexture->SetPixel(x, y, PIXR(pc), PIXG(pc), PIXB(pc), 255);
		}
	}
	newTexture->SetCharacter((width/2)-5, (height/2)-5, 0xA1, 32, 64, 128, 255);
	newTexture->SetCharacter((width/2)-5, (height/2)-5, 0xA0, 255, 255, 255, 255);
	return newTexture;
}

void SignTool::Click(Simulation * sim, Brush * brush, ui::Point position)
{
	int signX, signY, signW, signH, signIndex = -1;
	for(int i = 0; i < sim->signs.size(); i++){
		sim->signs[i].pos(signX, signY, signW, signH);
		if(position.X > signX && position.X < signX+signW && position.Y > signY && position.Y < signY+signH)
		{
			signIndex = i;
			break;
		}
	}
	new SignWindow(this, sim, signIndex, position);
}
