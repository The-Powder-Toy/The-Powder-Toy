#include "Tool.h"

#include "prefs/GlobalPrefs.h"
#include "Menu.h"
#include "Format.h"

#include "gui/game/GameModel.h"
#include "gui/Style.h"
#include "gui/game/Brush.h"
#include "gui/interface/Window.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/DropDown.h"
#include "gui/dialogues/ErrorMessage.h"

#include "simulation/GOLString.h"
#include "simulation/BuiltinGOL.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationData.h"

#include "graphics/Graphics.h"

#include "Config.h"
#include <iostream>
#include <SDL.h>

class PropertyWindow: public ui::Window
{
	void HandlePropertyChange();
	std::optional<std::pair<int, String>> TakePropertyFrom(const Particle *takePropertyFrom) const;

public:
	ui::DropDown * property;
	ui::Textbox * textField;
	PropertyTool * tool;
	Simulation *sim;
	std::vector<StructProperty> properties;
	std::optional<PropertyTool::Configuration> configuration;
	PropertyWindow(PropertyTool *tool_, Simulation *sim, const Particle *takePropertyFrom);
	void SetProperty();
	void CheckProperty();
	void Update();
	void OnDraw() override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnTryExit(ExitMethod method) override;
	virtual ~PropertyWindow() {}
};

PropertyWindow::PropertyWindow(PropertyTool * tool_, Simulation *sim_, const Particle *takePropertyFrom):
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
	okayButton->SetActionCallback({ [this] {
		CloseActiveWindow();
		SetProperty();
		SelfDestruct();
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	property = new ui::DropDown(ui::Point(8, 25), ui::Point(Size.X-16, 16));
	property->SetActionCallback({ [this] {
		HandlePropertyChange();
	} });
	AddComponent(property);
	for (int i = 0; i < int(properties.size()); i++)
	{
		property->AddOption(std::pair<String, int>(properties[i].Name.FromAscii(), i));
	}

	textField = new ui::Textbox(ui::Point(8, 46), ui::Point(Size.X-16, 16), "", "[value]");
	textField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	textField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	textField->SetActionCallback({ [this]() {
		Update();
	} });
	AddComponent(textField);

	{
		auto &prefs = GlobalPrefs::Ref();
		auto propertyIndex = prefs.Get("Prop.Type", 0);
		auto valueString = prefs.Get("Prop.Value", String(""));
		auto taken = TakePropertyFrom(takePropertyFrom);
		if (taken)
		{
			std::tie(propertyIndex, valueString) = *taken;
		}
		property->SetOption(propertyIndex);
		textField->SetText(valueString);
	}

	FocusComponent(textField);
	Update();

	MakeActiveWindow();
}

std::optional<std::pair<int, String>> PropertyWindow::TakePropertyFrom(const Particle *takePropertyFrom) const
{
	auto toolConfiguration = tool->GetConfiguration();
	if (!toolConfiguration || !takePropertyFrom)
	{
		return {};
	}
	auto *prop = reinterpret_cast<const char *>(takePropertyFrom) + toolConfiguration->prop.Offset;
	auto takeValue = [this, toolConfiguration](auto &value) -> std::optional<std::pair<int, String>> {
		auto it = std::find(properties.begin(), properties.end(), toolConfiguration->prop);
		if (it != properties.end())
		{
			return std::pair{ int(it - properties.begin()), String::Build(value) };
		}
		return {};
	};
	switch (toolConfiguration->prop.Type)
	{
	case StructProperty::Float:
		return takeValue(*reinterpret_cast<const float *>(prop));

	case StructProperty::ParticleType:
	case StructProperty::Integer:
		return takeValue(*reinterpret_cast<const int *>(prop));

	case StructProperty::UInteger:
		return takeValue(*reinterpret_cast<const unsigned int *>(prop));

	default:
		break;
	}
	return {};
}

void PropertyWindow::HandlePropertyChange()
{
	FocusComponent(textField);
	Update();
}

void PropertyWindow::Update()
{
	CheckProperty();
	auto haveConfiguration = bool(configuration);
	okayButton->Enabled = haveConfiguration;
	textField->SetTextColour(haveConfiguration ? ui::Colour(255, 255, 255) : style::Colour::ErrorTitle);
}

void PropertyWindow::CheckProperty()
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	configuration.reset();
	PropertyTool::Configuration newConfiguration;
	if (!(property->GetOption().second!=-1 && textField->GetText().length() > 0))
	{
		return;
	}
	String value = textField->GetText().ToUpper();
	try
	{
		switch(properties[property->GetOption().second].Type)
		{
			case StructProperty::Integer:
			case StructProperty::ParticleType:
			{
				int v;
				if(value.length() > 2 && value.BeginsWith("0X"))
				{
					//0xC0FFEE
					v = value.Substr(2).ToNumber<unsigned int>(Format::Hex());
				}
				else if(value.length() > 1 && value.BeginsWith("#"))
				{
					//#C0FFEE
					v = value.Substr(1).ToNumber<unsigned int>(Format::Hex());
				}
				else
				{
					// Try to parse as particle name
					v = sd.GetParticleType(value.ToUtf8());

					// Try to parse special GoL rules
					if (v == -1 && properties[property->GetOption().second].Name == "ctype")
					{
						if (value.length() > 1 && value.BeginsWith("B") && value.Contains("/"))
						{
							v = ParseGOLString(value);
							if (v == -1)
							{
								class InvalidGOLString : public std::exception
								{
								};
								throw InvalidGOLString();
							}
						}
						else
						{
							v = sd.GetParticleType(value.ToUtf8());
							if (v == -1)
							{
								for (auto *elementTool : tool->gameModel.GetMenuList()[SC_LIFE]->GetToolList())
								{
									if (elementTool && elementTool->Name == value)
									{
										v = ID(elementTool->ToolID);
										break;
									}
								}
							}
						}
					}

					// Parse as plain number
					if (v == -1)
					{
						v = value.ToNumber<int>();
					}
				}

				if (properties[property->GetOption().second].Name == "type" && (v < 0 || v >= PT_NUM || !elements[v].Enabled))
				{
					return;
				}
				if constexpr (DEBUG)
				{
					std::cout << "Got int value " << v << std::endl;
				}
				newConfiguration.propValue = v;
				break;
			}
			case StructProperty::UInteger:
			{
				unsigned int v;
				if(value.length() > 2 && value.BeginsWith("0X"))
				{
					//0xC0FFEE
					v = value.Substr(2).ToNumber<unsigned int>(Format::Hex());
				}
				else if(value.length() > 1 && value.BeginsWith("#"))
				{
					//#C0FFEE
					v = value.Substr(1).ToNumber<unsigned int>(Format::Hex());
				}
				else
				{
					v = value.ToNumber<unsigned int>();
				}
				if constexpr (DEBUG)
				{
					std::cout << "Got uint value " << v << std::endl;
				}
				newConfiguration.propValue = v;
				break;
			}
			case StructProperty::Float:
			{
				if (properties[property->GetOption().second].Name == "temp")
					newConfiguration.propValue = format::StringToTemperature(value, tool->gameModel.GetTemperatureScale());
				else
					newConfiguration.propValue = value.ToNumber<float>();
			}
				break;
			default:
				return;
		}
		newConfiguration.prop = properties[property->GetOption().second];
		newConfiguration.propertyIndex = property->GetOption().second;
		newConfiguration.propertyValueStr = value;
		newConfiguration.changeType = properties[property->GetOption().second].Name == "type";
	}
	catch (const std::exception& ex)
	{
		return;
	}
	configuration = newConfiguration;
}

void PropertyTool::SetConfiguration(std::optional<Configuration> newConfiguration)
{
	configuration = newConfiguration;
	if (configuration)
	{
		auto &prefs = GlobalPrefs::Ref();
		Prefs::DeferWrite dw(prefs);
		prefs.Set("Prop.Type", configuration->propertyIndex);
		prefs.Set("Prop.Value", configuration->propertyValueStr);
	}
}

void PropertyWindow::SetProperty()
{
	tool->SetConfiguration(configuration);
}

void PropertyWindow::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}

void PropertyWindow::OnDraw()
{
	Graphics * g = GetGraphics();

	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xC8C8C8_rgb);
}

void PropertyWindow::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (key == SDLK_UP)
	{
		property->SetOption(property->GetOption().second-1);
		HandlePropertyChange();
	}
	else if (key == SDLK_DOWN)
	{
		property->SetOption(property->GetOption().second+1);
		HandlePropertyChange();
	}
}

void PropertyTool::OpenWindow(Simulation *sim, const Particle *takePropertyFrom)
{
	new PropertyWindow(this, sim, takePropertyFrom);
}

void PropertyTool::SetProperty(Simulation *sim, ui::Point position)
{
	if(position.X<0 || position.X>XRES || position.Y<0 || position.Y>YRES || !configuration)
		return;
	int i = sim->pmap[position.Y][position.X];
	if(!i)
		i = sim->photons[position.Y][position.X];
	if(!i)
		return;

	if (configuration->changeType)
	{
		sim->part_change_type(ID(i), int(sim->parts[ID(i)].x+0.5f), int(sim->parts[ID(i)].y+0.5f), std::get<int>(configuration->propValue));
		return;
	}

	switch (configuration->prop.Type)
	{
		case StructProperty::Float:
			*((float*)(((char*)&sim->parts[ID(i)])+configuration->prop.Offset)) = std::get<float>(configuration->propValue);
			break;
		case StructProperty::ParticleType:
		case StructProperty::Integer:
			*((int*)(((char*)&sim->parts[ID(i)])+configuration->prop.Offset)) = std::get<int>(configuration->propValue);
			break;
		case StructProperty::UInteger:
			*((unsigned int*)(((char*)&sim->parts[ID(i)])+configuration->prop.Offset)) = std::get<unsigned int>(configuration->propValue);
			break;
		default:
			break;
	}
}

void PropertyTool::Draw(Simulation *sim, Brush const &cBrush, ui::Point position)
{
	for (ui::Point off : cBrush)
	{
		ui::Point coords = position + off;
		if (coords.X >= 0 && coords.Y >= 0 && coords.X < XRES && coords.Y < YRES)
			SetProperty(sim, coords);
	}
}

void PropertyTool::DrawLine(Simulation *sim, Brush const &cBrush, ui::Point position, ui::Point position2, bool dragging)
{
	int x1 = position.X, y1 = position.Y, x2 = position2.X, y2 = position2.Y;
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy, rx = cBrush.GetRadius().X, ry = cBrush.GetRadius().Y;
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

void PropertyTool::DrawRect(Simulation *sim, Brush const &cBrush, ui::Point position, ui::Point position2)
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

void PropertyTool::DrawFill(Simulation *sim, Brush const &cBrush, ui::Point position)
{
	if (configuration)
		sim->flood_prop(position.X, position.Y, configuration->prop, configuration->propValue);
}
