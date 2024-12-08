#include "AccessProperty.h"
#include "simulation/ElementClasses.h"
#include "simulation/GOLString.h"
#include "simulation/SimulationData.h"
#include "Config.h"
#include "Format.h"
#include "gui/game/GameController.h"
#include <iostream>

AccessProperty AccessProperty::Parse(int prop, String value)
{
	AccessProperty changeProperty;
	auto &sd = SimulationData::CRef();
	auto &properties = Particle::GetProperties();
	auto &elements = sd.elements;
	if (prop < 0 || prop >= int(properties.size()) || !value.size())
	{
		throw ParseError("invalid property index");
	}
	value = value.ToUpper();
	try
	{
		switch(properties[prop].Type)
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
				if (v == -1 && properties[prop].Name == "ctype")
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
						auto &builtinGol = SimulationData::builtinGol;
						auto &customGol = sd.GetCustomGol();
						if (v == -1)
						{
							for (int i = 0; i < int(builtinGol.size()); ++i)
							{
								if (builtinGol[i].name == value)
								{
									v = PMAP(i, PT_LIFE);
									break;
								}
							}
						}
						if (v == -1)
						{
							for (int i = 0; i < int(customGol.size()); ++i)
							{
								if (customGol[i].nameString == value)
								{
									v = PMAP(customGol[i].rule, PT_LIFE);
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

			if (properties[prop].Name == "type" && (v < 0 || v >= PT_NUM || !elements[v].Enabled))
			{
				throw ParseError("invalid element");
			}
			if constexpr (DEBUG)
			{
				std::cout << "Got int value " << v << std::endl;
			}
			changeProperty.propertyValue = v;
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
			changeProperty.propertyValue = v;
			break;
		}
		case StructProperty::Float:
		{
			if (properties[prop].Name == "temp")
				changeProperty.propertyValue = format::StringToTemperature(value, GameController::Ref().GetTemperatureScale());
			else
				changeProperty.propertyValue = value.ToNumber<float>();
		}
			break;
		default:
			throw ParseError("invalid property value");
		}
		changeProperty.propertyIndex = prop;
	}
	catch (const std::exception& ex)
	{
		throw ParseError("invalid property value");
	}
	return changeProperty;
}
