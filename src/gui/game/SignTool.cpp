#include <iostream>
#include "gui/Style.h"
#include "simulation/Simulation.h"
#include "Tool.h"
#include "gui/interface/Window.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/DropDown.h"
#include "gui/game/GameModel.h"

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
	virtual void DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt) { if(!signMoving) ui::Window::DoKeyPress(key, character, shift, ctrl, alt); }
	virtual void DoKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt) { if(!signMoving) ui::Window::DoKeyRelease(key, character, shift, ctrl, alt); }
	virtual ~SignWindow() {}
	virtual void OnTryExit(ui::Window::ExitMethod method);
	class OkayAction: public ui::ButtonAction
	{
	public:
		SignWindow * prompt;
		OkayAction(SignWindow * prompt_) { prompt = prompt_; }
		void ActionCallback(ui::Button * sender)
		{
			prompt->CloseActiveWindow();	
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
			prompt->CloseActiveWindow();
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
				prompt->sim->signs[prompt->signID].ju = (sign::Justification)prompt->justification->GetOption().second;
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
				prompt->sim->signs[prompt->signID].ju = (sign::Justification)prompt->justification->GetOption().second;
				prompt->signMoving = true;
			}
		}
	};
};

SignWindow::SignWindow(SignTool * tool_, Simulation * sim_, int signID_, ui::Point position_):
	ui::Window(ui::Point(-1, -1), ui::Point(250, 87)),
	tool(tool_),
	movingSign(NULL),
	signMoving(false),
	sim(sim_),
	signID(signID_),
	signPosition(position_)
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
	
	ui::Label * tempLabel = new ui::Label(ui::Point(8, 48), ui::Point(40, 15), "Pointer:");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	justification = new ui::DropDown(ui::Point(52, 48), ui::Point(50, 16));
	AddComponent(justification);
	justification->AddOption(std::pair<std::string, int>("\xA0 Left", (int)sign::Left));
	justification->AddOption(std::pair<std::string, int>("\x9E Middle", (int)sign::Middle));
	justification->AddOption(std::pair<std::string, int>("\x9F Right", (int)sign::Right));
	justification->AddOption(std::pair<std::string, int>("\x9D None", (int)sign::None));
	justification->SetOption(1);
	justification->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	
	textField = new ui::Textbox(ui::Point(8, 25), ui::Point(Size.X-16, 17), "", "[message]");
	textField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	textField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	textField->SetLimit(45);
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

	MakeActiveWindow();
}

void SignWindow::OnTryExit(ui::Window::ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}

void SignWindow::DoDraw()
{
	for(std::vector<sign>::iterator iter = sim->signs.begin(), end = sim->signs.end(); iter != end; ++iter)
	{
		sign & currentSign = *iter;
		int x, y, w, h, dx, dy;
		char type = 0;
		Graphics * g = GetGraphics();
		std::string text = currentSign.getText(sim);
		sign::splitsign(currentSign.text.c_str(), &type);
		currentSign.pos(text, x, y, w, h);
		g->clearrect(x, y, w+1, h);
		g->drawrect(x, y, w+1, h, 192, 192, 192, 255);
		if (!type)
			g->drawtext(x+3, y+3, text, 255, 255, 255, 255);
		else if(type == 'b')
			g->drawtext(x+3, y+3, text, 211, 211, 40, 255);
		else
			g->drawtext(x+3, y+3, text, 0, 191, 255, 255);

		if (currentSign.ju != sign::None)
		{
			x = currentSign.x;
			y = currentSign.y;
			dx = 1 - currentSign.ju;
			dy = (currentSign.y > 18) ? -1 : 1;
#ifdef OGLR
			glBegin(GL_LINES);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glVertex2i(x, y);
			glVertex2i(x+(dx*4), y+(dy*4));
			glEnd();
#else
			for (int j=0; j<4; j++)
			{
				g->blendpixel(x, y, 192, 192, 192, 255);
				x+=dx;
				y+=dy;
			}
#endif
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
		ui::Point pos = tool->gameModel->AdjustZoomCoords(ui::Point(x, y));
		if(pos.X < XRES && pos.Y < YRES)
		{
			movingSign->x = pos.X;
			movingSign->y = pos.Y;
			signPosition.X = pos.X;
			signPosition.Y = pos.Y;
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
	Graphics * g = GetGraphics();
	
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
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
	newTexture->BlendCharacter((width/2)-5, (height/2)-5, 0xA0, 255, 255, 255, 255);
	return newTexture;
}

void SignTool::Click(Simulation * sim, Brush * brush, ui::Point position)
{
	int signX, signY, signW, signH, signIndex = -1;
	for (size_t i = 0; i < sim->signs.size(); i++)
	{
		sim->signs[i].pos(sim->signs[i].getText(sim), signX, signY, signW, signH);
		if (position.X > signX && position.X < signX+signW && position.Y > signY && position.Y < signY+signH)
		{
			signIndex = i;
			break;
		}
	}
	if (signIndex != -1 || sim->signs.size() < MAXSIGNS)
		new SignWindow(this, sim, signIndex, position);
}
