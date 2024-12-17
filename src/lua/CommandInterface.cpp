#include "CommandInterface.h"
#include "Misc.h"
#include "gui/game/GameModel.h"
#include "simulation/Particle.h"
#include "Format.h"
#include "simulation/Simulation.h"
#include "simulation/Air.h"
#include "simulation/ElementClasses.h"
#include "gui/game/GameController.h"
#include "gui/game/GameModel.h"
#include "gui/interface/Engine.h"
#include "common/tpt-compat.h"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <deque>

CommandInterface::CommandInterface(GameController *newGameController, GameModel *newGameModel)
{
	this->m = newGameModel;
	this->c = newGameController;
}

void CommandInterface::Log(LogType type, String message)
{
	m->Log(message, type == LogError || type == LogNotice);
}

static std::optional<int> GetPropertyOffset(ByteString key)
{
	for (auto &alias : Particle::GetPropertyAliases())
	{
		if (key == alias.from)
		{
			key = alias.to;
		}
	}
	auto &properties = Particle::GetProperties();
	for (int i = 0; i < int(properties.size()); ++i)
	{
		auto &prop = properties[i];
		if (key == prop.Name)
		{
			return i;
		}
	}
	return std::nullopt;
}

String CommandInterface::GetLastError()
{
	return lastError;
}

int CommandInterface::PlainCommand(String command)
{
	lastError = "";
	std::deque<String> words;
	std::deque<AnyType> commandWords;
	int retCode = -1;

	//Split command into words, put them on the stack
	for(String word : command.PartitionBy(' '))
		words.push_back(word);
	while(!words.empty())
	{
		try
		{
			commandWords.push_back(eval(&words));
		}
		catch (GeneralException & e)
		{
			retCode = -1;
			lastError = e.GetExceptionMessage();
			break;
		}
	}
	if(commandWords.size())
	{
		retCode = 0;
		lastError = ((StringType)commandWords.front()).Value();
	}

	//Evaluate
	return retCode;
}

struct Function
{
	const char32_t *name;
	AnyType (CommandInterface::*member)(std::deque<String> *);
};
static const std::vector<Function> functions = {
	{ U"set"   , &CommandInterface::tptS_set    },
	{ U"get"   , &CommandInterface::tptS_get    },
	{ U"create", &CommandInterface::tptS_create },
	{ U"delete", &CommandInterface::tptS_delete },
	{ U"kill"  , &CommandInterface::tptS_delete },
	{ U"load"  , &CommandInterface::tptS_load   },
	{ U"reset" , &CommandInterface::tptS_reset  },
	{ U"bubble", &CommandInterface::tptS_bubble },
	{ U"quit"  , &CommandInterface::tptS_quit   },
};

ValueType CommandInterface::testType(String word)
{
	size_t i = 0;
	String::value_type const *rawWord = word.c_str();
	//Function
	for (auto &function : functions)
	{
		if (word == function.name)
		{
			return TypeFunction;
		}
	}

	//Basic type
	for (i = 0; i < word.length(); i++)
	{
		if (!(rawWord[i] >= '0' && rawWord[i] <= '9') && !(rawWord[i] == '-' && !i))
		{
			if (rawWord[i] == '.' && rawWord[i+1])
				goto parseFloat;
			else if (rawWord[i] == ',' && rawWord[i+1] >= '0' && rawWord[i+1] <= '9')
				goto parsePoint;
			else if ((rawWord[i] == '#' || (i && rawWord[i-1] == '0' && rawWord[i] == 'x')) &&
				((rawWord[i+1] >= '0' && rawWord[i+1] <= '9')
				|| (rawWord[i+1] >= 'a' && rawWord[i+1] <= 'f')
				|| (rawWord[i+1] >= 'A' && rawWord[i+1] <= 'F')))
				goto parseNumberHex;
			else
				goto parseString;
		}
	}
	return TypeNumber;

parseFloat:
	for (i++; i < word.length(); i++)
		if (!((rawWord[i] >= '0' && rawWord[i] <= '9')))
		{
			goto parseString;
		}
	return TypeFloat;

parseNumberHex:
	for (i++; i < word.length(); i++)
		if (!((rawWord[i] >= '0' && rawWord[i] <= '9') || (rawWord[i] >= 'a' && rawWord[i] <= 'f') || (rawWord[i] >= 'A' && rawWord[i] <= 'F')))
		{
			goto parseString;
		}
	return TypeNumber;

parsePoint:
	for (i++; i < word.length(); i++)
		if (!(rawWord[i] >= '0' && rawWord[i] <= '9'))
		{
			goto parseString;
		}
	return TypePoint;

parseString:
	return TypeString;
}

int CommandInterface::parseNumber(String str)
{
	String::value_type const *stringData = str.c_str();
	char cc;
	int base = 10;
	int currentNumber = 0;
	if (stringData[0] == '#')
	{
		stringData++;
		base = 16;
	}
	else if (stringData[0] == '0' && stringData[1] == 'x')
	{
		stringData+=2;
		base = 16;
	}
	if (base == 16)
	{
		while ((cc = *(stringData++)))
		{
			currentNumber *= base;
			if (cc >= '0' && cc <= '9')
				currentNumber += cc - '0';
			else if (cc >= 'a' && cc <= 'f')
				currentNumber += (cc - 'a') + 10;
			else if (cc >= 'A' && cc <= 'F')
				currentNumber += (cc - 'A') + 10;
			else
				break;
		}
	}
	else
	{
		try
		{
			return str.ToNumber<int>();
		}
		catch (std::exception & e)
		{
			throw GeneralException(ByteString(e.what()).FromUtf8());
		}
	}
	return currentNumber;
}

AnyType CommandInterface::eval(std::deque<String> * words)
{
	if(words->size() < 1)
		return AnyType(TypeNull, ValueValue());
	String word = words->front(); words->pop_front();
	ValueType wordType = testType(word);
	switch(wordType)
	{
	case TypeFunction:
		{
			auto it = std::find_if(functions.begin(), functions.end(), [&word](auto &func) {
				return func.name == word;
			});
			return (this->*(it->member))(words);
		}
		break;
	case TypeNumber:
		return NumberType(parseNumber(word));
	case TypeFloat:
		return FloatType(atof(word.ToUtf8().c_str()));
	case TypePoint:
	{
		int x, y;
		if(String::Split comma = word.SplitNumber(x))
			if(comma.After().BeginsWith(","))
				if(comma.After().Substr(1).SplitNumber(y))
					return PointType(x, y);
		return PointType(0, 0);
	}
	case TypeString:
		return StringType(word);
	default:
		break;
	}
	return StringType(word);
}

String CommandInterface::PlainFormatCommand(String command)
{
	std::deque<String> words;
	std::deque<AnyType> commandWords;
	String outputData;

	//Split command into words, put them on the stack
	for(String word : command.PartitionBy(' ', true))
		words.push_back(word);
	while(!words.empty())
	{
		ValueType cType = testType(words.front());
		switch(cType)
		{
		case TypeFunction:
			outputData += "\bt";
			break;
		case TypeNumber:
		case TypePoint:
			outputData += "\bo";
			break;
		case TypeString:
			outputData += "\bg";
			break;
		default:
			outputData += "\bw";
			break;
		}
		outputData += words.front() + " ";
		words.pop_front();
	}
	return outputData;
}

static std::vector<int> EvaluateSelector(Simulation *sim, AnyType selector)
{
	auto &sd = SimulationData::CRef();
	std::vector<int> indices;
	if (selector.GetType() == TypePoint || selector.GetType() == TypeNumber)
	{
		int partIndex = -1;
		if(selector.GetType() == TypePoint)
		{
			ui::Point tempPoint = ((PointType)selector).Value();
			if(tempPoint.X<0 || tempPoint.Y<0 || tempPoint.Y >= YRES || tempPoint.X >= XRES)
				throw GeneralException("Invalid position");

			auto r = sim->pmap[tempPoint.Y][tempPoint.X];
			if (!r)
			{
				r = sim->photons[tempPoint.Y][tempPoint.X];
			}
			if (r)
			{
				partIndex = ID(r);
			}
		}
		else
			partIndex = ((NumberType)selector).Value();
		if(partIndex<0 || partIndex>=NPART || sim->parts[partIndex].type==0)
			throw GeneralException("Invalid particle");

		indices.push_back(partIndex);
	}
	else if (selector.GetType() == TypeString && ((StringType)selector).Value() == "all")
	{
		for(int j = 0; j < NPART; j++)
		{
			if(sim->parts[j].type)
			{
				indices.push_back(j);
			}
		}
	}
	else if(selector.GetType() == TypeString || selector.GetType() == TypeNumber)
	{
		int type = 0;
		if (selector.GetType() == TypeNumber)
			type = ((NumberType)selector).Value();
		else if (selector.GetType() == TypeString)
			type = sd.GetParticleType(((StringType)selector).Value().ToUtf8());

		if (type<0 || type>=PT_NUM)
			throw GeneralException("Invalid particle type");
		if (type==0)
			throw GeneralException("Cannot access properties of particles that do not exist");
		for (int j = 0; j < NPART; j++)
		{
			if (sim->parts[j].type == type)
			{
				indices.push_back(j);
			}
		}
	}
	else
		throw GeneralException("Invalid selector");
	return indices;
}

AnyType CommandInterface::tptS_set(std::deque<String> * words)
{
	//Arguments from stack
	StringType property = eval(words);
	AnyType selector = eval(words);
	AnyType value = eval(words);

	Simulation * sim = m->GetSimulation();
	auto prop = GetPropertyOffset(property.Value().ToUtf8());
	if (!prop)
	{
		throw GeneralException("Invalid property");
	}
	auto &propInfo = Particle::GetProperties()[*prop];

	// assume that value can be anything
	if (value.GetType() == TypeNumber && propInfo.Type == StructProperty::Float)
	{
		value = FloatType(float(NumberType(value).Value()));
	}
	if (value.GetType() == TypeFloat && propInfo.Type != StructProperty::Float)
	{
		value = NumberType(int(FloatType(value).Value()));
	}
	// value can still be almost anything, but if it was NumberType or FloatType,
	// at least it now matches the float-ness, if not the signedness, of prop
	AccessProperty changeProperty;
	try
	{
		switch (value.GetType())
		{
		case TypeNumber:
			// get a number (an int) => take an int
			changeProperty.propertyIndex = *prop;
			changeProperty.propertyValue = NumberType(value).Value();
			if (propInfo.Type == StructProperty::UInteger)
			{
				// actually want an unsigned int => convert the int taken to one
				changeProperty.propertyValue = static_cast<unsigned int>(std::get<int>(changeProperty.propertyValue));
			}
			break;

		case TypeFloat:
			// get a float => take a float
			changeProperty.propertyIndex = *prop;
			changeProperty.propertyValue = FloatType(value).Value();
			break;

		case TypeString:
			// AccessProperty::Parse returns the appropriate variant
			changeProperty = AccessProperty::Parse(*prop, StringType(value).Value());
			break;

		default:
			// get something else => bail
			throw GeneralException("Invalid property value");
		}
	}
	catch (const AccessProperty::ParseError &ex)
	{
		// TODO: add element CAKE to invalidate this
		if (value.GetType() == TypeString && StringType(value).Value().ToUpper() == "CAKE")
		{
			throw GeneralException("Cake is a lie, not an element");
		}
		throw GeneralException(ByteString(ex.what()).FromUtf8());
	}

	int returnValue = 0;
	for (auto index : EvaluateSelector(sim, selector))
	{
		returnValue++;
		changeProperty.Set(sim, index);
	}
	return NumberType(returnValue);
}

AnyType CommandInterface::tptS_get(std::deque<String> * words)
{
	StringType property = eval(words);
	AnyType selector = eval(words);

	Simulation *sim = m->GetSimulation();
	auto prop = GetPropertyOffset(property.Value().ToUtf8());
	if (!prop)
	{
		throw GeneralException("Invalid property");
	}
	auto &propInfo = Particle::GetProperties()[*prop];
	AccessProperty accessProperty{ *prop };

	auto indices = EvaluateSelector(sim, selector);
	if (indices.size() > 1)
	{
		throw GeneralException("Multiple matching particles");
	}
	if (indices.size() < 1)
	{
		throw GeneralException("No matching particles");
	}

	auto value = accessProperty.Get(sim, indices[0]);
	switch (propInfo.Type)
	{
	case StructProperty::Float:
		return FloatType(std::get<float>(value));

	case StructProperty::UInteger:
		return NumberType(std::get<unsigned int>(value));

	default:
		break;
	}
	return NumberType(std::get<int>(value));
}

AnyType CommandInterface::tptS_create(std::deque<String> * words)
{
	auto &sd = SimulationData::CRef();
	//Arguments from stack
	AnyType createType = eval(words);
	PointType position = eval(words);

	Simulation * sim = m->GetSimulation();

	int type;
	if(createType.GetType() == TypeNumber)
		type = ((NumberType)createType).Value();
	else if(createType.GetType() == TypeString)
		type = sd.GetParticleType(((StringType)createType).Value().ToUtf8());
	else
		throw GeneralException("Invalid type");

	if(type == -1)
		throw GeneralException("Invalid particle type");

	ui::Point tempPoint = position.Value();
	if(tempPoint.X<0 || tempPoint.Y<0 || tempPoint.Y >= YRES || tempPoint.X >= XRES)
				throw GeneralException("Invalid position");

	int v = -1;
	if (ID(type))
	{
		v = ID(type);
		type = TYP(type);
	}
	int returnValue = sim->create_part(-1, tempPoint.X, tempPoint.Y, type, v);

	return NumberType(returnValue);
}

AnyType CommandInterface::tptS_delete(std::deque<String> * words)
{
	//Arguments from stack
	AnyType partRef = eval(words);

	Simulation * sim = m->GetSimulation();

	if(partRef.GetType() == TypePoint)
	{
		ui::Point deletePoint = ((PointType)partRef).Value();
		if(deletePoint.X<0 || deletePoint.Y<0 || deletePoint.Y >= YRES || deletePoint.X >= XRES)
			throw GeneralException("Invalid position");
		sim->delete_part(deletePoint.X, deletePoint.Y);
	}
	else if(partRef.GetType() == TypeNumber)
	{
		int partIndex = ((NumberType)partRef).Value();
		if(partIndex < 0 || partIndex >= NPART)
			throw GeneralException("Invalid particle index");
		sim->kill_part(partIndex);
	}
	else
		throw GeneralException("Invalid particle reference");

	return NumberType(0);
}

AnyType CommandInterface::tptS_load(std::deque<String> * words)
{
	//Arguments from stack
	NumberType saveID = eval(words);

	if (saveID.Value() > 0)
	{
		c->OpenSavePreview(saveID.Value(), 0, savePreviewNormal);
		return NumberType(0);
	}
	else
		throw GeneralException("Invalid save ID");
}

AnyType CommandInterface::tptS_bubble(std::deque<String> * words)
{
	//Arguments from stack
	PointType bubblePosA = eval(words);
	ui::Point bubblePos = bubblePosA.Value();

	if(bubblePos.X<0 || bubblePos.Y<0 || bubblePos.Y >= YRES || bubblePos.X >= XRES)
			throw GeneralException("Invalid position");

	Simulation * sim = m->GetSimulation();

	int first, rem1, rem2;

	first = sim->create_part(-1, bubblePos.X+18, bubblePos.Y, PT_SOAP);
	rem1 = first;

	for (int i = 1; i<=30; i++)
	{
		rem2 = sim->create_part(-1, int(bubblePos.X+18*cosf(i/5.0)+0.5f), int(bubblePos.Y+18*sinf(i/5.0)+0.5f), PT_SOAP);

		sim->parts[rem1].ctype = 7;
		sim->parts[rem1].tmp = rem2;
		sim->parts[rem2].tmp2 = rem1;

		rem1 = rem2;
	}

	sim->parts[rem1].ctype = 7;
	sim->parts[rem1].tmp = first;
	sim->parts[first].tmp2 = rem1;
	sim->parts[first].ctype = 7;

	return NumberType(0);
}

AnyType CommandInterface::tptS_reset(std::deque<String> * words)
{
	auto &sd = SimulationData::CRef();
	//Arguments from stack
	StringType reset = eval(words);
	String resetStr = reset.Value();

	Simulation * sim = m->GetSimulation();

	if (resetStr == "pressure")
	{
		for (int nx = 0; nx < XCELLS; nx++)
			for (int ny = 0; ny < YCELLS; ny++)
			{
				sim->pv[ny][nx] = 0;
			}
	}
	else if (resetStr == "velocity")
	{
		for (int nx = 0; nx < XCELLS; nx++)
			for (int ny = 0; ny < YCELLS; ny++)
			{
				sim->vx[ny][nx] = 0;
				sim->vy[ny][nx] = 0;
			}
	}
	else if (resetStr == "sparks")
	{
		c->ResetSpark();
	}
	else if (resetStr == "temp")
	{
		for (int i = 0; i < NPART; i++)
		{
			if (sim->parts[i].type)
			{
				sim->parts[i].temp = sd.elements[sim->parts[i].type].DefaultProperties.temp;
			}
		}
	}
	else
	{
		throw GeneralException("Unknown reset command");
	}

	return NumberType(0);
}

AnyType CommandInterface::tptS_quit(std::deque<String> * words)
{
	ui::Engine::Ref().Exit();

	return NumberType(0);
}
