#include <iostream>
#include <sstream>
#include "Tool.h"
#include "client/Client.h"
#include "gui/Style.h"
#include "gui/game/Brush.h"
#include "gui/interface/Window.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/DropDown.h"
#include "gui/interface/Keys.h"
#include "gui/dialogues/ErrorMessage.h"
#include "simulation/Simulation.h"

class PropertyWindow: public ui::Window
{
public:
	ui::DropDown * property;
	ui::Textbox * textField;
	PropertyTool * tool;
	Simulation *sim;
	std::vector<StructProperty> properties;
	PropertyWindow(PropertyTool *tool_, Simulation *sim);
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
			prompt->CloseActiveWindow();
			if(prompt->textField->GetText().length())
				prompt->SetProperty();
			prompt->SelfDestruct();
			return;
		}
	};
};

PropertyWindow::PropertyWindow(PropertyTool * tool_, Simulation *sim_):
ui::Window(ui::Point(-1, -1), ui::Point(200, 87)),
tool(tool_),
sim(sim_)
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
	for (size_t i = 0; i < properties.size(); i++)
	{
		property->AddOption(std::pair<std::string, int>(properties[i].Name, i));
	}
	property->SetOption(Client::Ref().GetPrefInteger("Prop.Type", 0));

	textField = new ui::Textbox(ui::Point(8, 46), ui::Point(Size.X-16, 16), "", "[value]");
	textField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	textField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	textField->SetText(Client::Ref().GetPrefString("Prop.Value", ""));
	AddComponent(textField);
	FocusComponent(textField);

	MakeActiveWindow();
}

void PropertyWindow::SetProperty()
{
	if(property->GetOption().second!=-1 && textField->GetText().length() > 0)
	{
		std::string value = textField->GetText();
		try {
			switch(properties[property->GetOption().second].Type)
			{
				case StructProperty::Integer:
				case StructProperty::ParticleType:
				{
					int v;
					if(value.length() > 2 && value.substr(0, 2) == "0x")
					{
						//0xC0FFEE
						std::stringstream buffer;
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer << std::hex << value.substr(2);
						buffer >> v;
					}
					else if(value.length() > 1 && value[0] == '#')
					{
						//#C0FFEE
						std::stringstream buffer;
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer << std::hex << value.substr(1);
						buffer >> v;
					}
					else
					{
						int type;
						if (properties[property->GetOption().second].Type == StructProperty::ParticleType && (type = sim->GetParticleType(value)) != -1)
						{
							v = type;
							
#ifdef DEBUG
							std::cout << "Got type from particle name" << std::endl;
#endif
						}
						else
						{
							std::stringstream buffer(value);
							buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
							buffer >> v;
						}
					}
						
					if (properties[property->GetOption().second].Name == "type" && (v < 0 || v >= PT_NUM || !sim->elements[v].Enabled))
					{
						new ErrorMessage("Could not set property", "Invalid particle type");
						return;
					}
						
#ifdef DEBUG
					std::cout << "Got int value " << v << std::endl;
#endif

					tool->propValue.Integer = v;
					break;
				}
				case StructProperty::UInteger:
				{
					unsigned int v;
					if(value.length() > 2 && value.substr(0, 2) == "0x")
					{
						//0xC0FFEE
						std::stringstream buffer;
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer << std::hex << value.substr(2);
						buffer >> v;
					}
					else if(value.length() > 1 && value[0] == '#')
					{
						//#C0FFEE
						std::stringstream buffer;
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer << std::hex << value.substr(1);
						buffer >> v;
					}
					else
					{
						std::stringstream buffer(value);
						buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
						buffer >> v;
					}
#ifdef DEBUG
					std::cout << "Got uint value " << v << std::endl;
#endif
					tool->propValue.UInteger = v;
					break;
				}
				case StructProperty::Float:
				{
					std::stringstream buffer(value);
					buffer.exceptions(std::stringstream::failbit | std::stringstream::badbit);
					buffer >> tool->propValue.Float;
					if (properties[property->GetOption().second].Name == "temp" && value.length())
					{
						if (value.substr(value.length()-1) == "C")
							tool->propValue.Float += 273.15;
						else if (value.substr(value.length()-1) == "F")
							tool->propValue.Float = (tool->propValue.Float-32.0f)*5/9+273.15f;
					}
#ifdef DEBUG
					std::cout << "Got float value " << tool->propValue.Float << std::endl;
#endif
				}
					break;
				default:
					new ErrorMessage("Could not set property", "Invalid property");
					return;
			}
			tool->propOffset = properties[property->GetOption().second].Offset;
			tool->propType = properties[property->GetOption().second].Type;
		} catch (const std::exception& ex) {
			new ErrorMessage("Could not set property", "Invalid value provided");
			return;
		}
		Client::Ref().SetPref("Prop.Type", property->GetOption().second);
		Client::Ref().SetPref("Prop.Value", textField->GetText());
	}
}

void PropertyWindow::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}

void PropertyWindow::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

void PropertyWindow::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if (key == SDLK_UP)
		property->SetOption(property->GetOption().second-1);
	else if (key == SDLK_DOWN)
		property->SetOption(property->GetOption().second+1);
}

void PropertyTool::OpenWindow(Simulation *sim)
{
	new PropertyWindow(this, sim);
}

void PropertyTool::SetProperty(Simulation *sim, ui::Point position)
{
	if(position.X<0 || position.X>XRES || position.Y<0 || position.Y>YRES)
		return;
	int i = sim->pmap[position.Y][position.X];
	if(!i)
		i = sim->photons[position.Y][position.X];
	if(!i)
		return;
	switch (propType)
	{
		case StructProperty::Float:
			*((float*)(((char*)&sim->parts[ID(i)])+propOffset)) = propValue.Float;
			break;
		case StructProperty::ParticleType:
		case StructProperty::Integer:
			*((int*)(((char*)&sim->parts[ID(i)])+propOffset)) = propValue.Integer;
			break;
		case StructProperty::UInteger:
			*((unsigned int*)(((char*)&sim->parts[ID(i)])+propOffset)) = propValue.UInteger;
			break;
		default:
			break;
	}
}

void PropertyTool::Draw(Simulation *sim, Brush *cBrush, ui::Point position)
{
	if(cBrush)
	{
		int radiusX = cBrush->GetRadius().X, radiusY = cBrush->GetRadius().Y, sizeX = cBrush->GetSize().X, sizeY = cBrush->GetSize().Y;
		unsigned char *bitmap = cBrush->GetBitmap();
		for(int y = 0; y < sizeY; y++)
			for(int x = 0; x < sizeX; x++)
				if(bitmap[(y*sizeX)+x] && (position.X+(x-radiusX) >= 0 && position.Y+(y-radiusY) >= 0 && position.X+(x-radiusX) < XRES && position.Y+(y-radiusY) < YRES))
					SetProperty(sim, ui::Point(position.X+(x-radiusX), position.Y+(y-radiusY)));
	}
}

void PropertyTool::DrawLine(Simulation *sim, Brush *cBrush, ui::Point position, ui::Point position2, bool dragging)
{
	int x1 = position.X, y1 = position.Y, x2 = position2.X, y2 = position2.Y;
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy, rx = cBrush->GetRadius().X, ry = cBrush->GetRadius().Y;
	float e = 0.0f, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			Draw(sim, cBrush, ui::Point(y, x));
		else
			Draw(sim, cBrush, ui::Point(x, y));
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					Draw(sim, cBrush, ui::Point(y, x));
				else
					Draw(sim, cBrush, ui::Point(x, y));
			}
			e -= 1.0f;
		}
	}
}

void PropertyTool::DrawRect(Simulation *sim, Brush *cBrush, ui::Point position, ui::Point position2)
{
	int x1 = position.X, y1 = position.Y, x2 = position2.X, y2 = position2.Y;
	int i, j;
	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			SetProperty(sim, ui::Point(i, j));
}

void PropertyTool::DrawFill(Simulation *sim, Brush *cBrush, ui::Point position)
{
	sim->flood_prop(position.X, position.Y, propOffset, propValue, propType);
}
