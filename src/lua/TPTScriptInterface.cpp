#include "TPTScriptInterface.h"

#include <deque>
#ifdef MACOSX
#include <strings.h>
#endif
#include <cstdlib>
#include <cmath>

#include "Config.h"

#include "simulation/Simulation.h"
#include "simulation/Air.h"
#include "simulation/ElementClasses.h"

#include "gui/game/GameController.h"
#include "gui/game/GameModel.h"

#include "gui/interface/Engine.h"

#include "common/tpt-compat.h"

TPTScriptInterface::TPTScriptInterface(GameController * c, GameModel * m): CommandInterface(c, m)
{
}

int TPTScriptInterface::Command(String command)
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

ValueType TPTScriptInterface::testType(String word)
{
	size_t i = 0;
	String::value_type const *rawWord = word.c_str();
	//Function
	if (word == "set")
		return TypeFunction;
	else if (word == "create")
		return TypeFunction;
	else if (word == "delete")
		return TypeFunction;
	else if (word == "kill")
		return TypeFunction;
	else if (word == "load")
		return TypeFunction;
	else if (word == "reset")
		return TypeFunction;
	else if (word == "bubble")
		return TypeFunction;
	else if (word == "quit")
		return TypeFunction;

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

int TPTScriptInterface::parseNumber(String str)
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

AnyType TPTScriptInterface::eval(std::deque<String> * words)
{
	if(words->size() < 1)
		return AnyType(TypeNull, ValueValue());
	String word = words->front(); words->pop_front();
	ValueType wordType = testType(word);
	switch(wordType)
	{
	case TypeFunction:
		if(word == "set")
			return tptS_set(words);
		else if(word == "create")
			return tptS_create(words);
		else if(word == "delete" || word == "kill")
			return tptS_delete(words);
		else if(word == "load")
			return tptS_load(words);
		else if(word == "reset")
			return tptS_reset(words);
		else if(word == "bubble")
			return tptS_bubble(words);
		else if(word == "quit")
			return tptS_quit(words);
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

String TPTScriptInterface::FormatCommand(String command)
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

AnyType TPTScriptInterface::tptS_set(std::deque<String> * words)
{
	//Arguments from stack
	StringType property = eval(words);
	AnyType selector = eval(words);
	AnyType value = eval(words);

	Simulation * sim = m->GetSimulation();
	unsigned char * partsBlock = (unsigned char*)&sim->parts[0];

	int returnValue = 0;

	FormatType propertyFormat;
	int propertyOffset = GetPropertyOffset(property.Value().ToUtf8(), propertyFormat);
	if (propertyOffset == -1)
		throw GeneralException("Invalid property");

	//Selector
	int newValue = 0;
	float newValuef = 0.0f;
	if (value.GetType() == TypeNumber)
	{
		newValuef = newValue = ((NumberType)value).Value();
	}
	else if (value.GetType() == TypeFloat)
	{
		newValue = newValuef = ((FloatType)value).Value();
	}
	else if(value.GetType() == TypeString)
	{
		if (property.Value() == "temp")
		{
			String newString = ((StringType)value).Value();
			if (newString.at(newString.length()-1) == 'C')
				newValuef = atof(newString.SubstrFromEnd(1).ToUtf8().c_str())+273.15;
			else if (newString.at(newString.length()-1) == 'F')
				newValuef = (atof(newString.SubstrFromEnd(1).ToUtf8().c_str())-32.0f)*5/9+273.15f;
			else
				throw GeneralException("Invalid value for assignment");
		}
		else
		{
			newValue = m->GetSimulation()->GetParticleType(((StringType)value).Value().ToUtf8());
			if (newValue < 0 || newValue >= PT_NUM)
			{
				// TODO: add element CAKE to invalidate this
				if (!strcasecmp(((StringType)value).Value().ToUtf8().c_str(),"cake"))
					throw GeneralException("Cake is a lie, not an element");
				throw GeneralException("Invalid element");
			}
		}
	}
	else
		throw GeneralException("Invalid value for assignment");
	if (property.Value() == "type" && (newValue < 0 || newValue >= PT_NUM || !sim->elements[newValue].Enabled))
		throw GeneralException("Invalid element");

	if (selector.GetType() == TypePoint || selector.GetType() == TypeNumber)
	{
		int partIndex = -1;
		if(selector.GetType() == TypePoint)
		{
			ui::Point tempPoint = ((PointType)selector).Value();
			if(tempPoint.X<0 || tempPoint.Y<0 || tempPoint.Y >= YRES || tempPoint.X >= XRES)
				throw GeneralException("Invalid position");

		}
		else
			partIndex = ((NumberType)selector).Value();
		if(partIndex<0 || partIndex>=NPART || sim->parts[partIndex].type==0)
			throw GeneralException("Invalid particle");

		switch(propertyFormat)
		{
		case FormatInt:
			*((int*)(partsBlock+(partIndex*sizeof(Particle))+propertyOffset)) = newValue;
			break;
		case FormatFloat:
			*((float*)(partsBlock+(partIndex*sizeof(Particle))+propertyOffset)) = newValuef;
			break;
		case FormatElement:
			sim->part_change_type(partIndex, sim->parts[partIndex].x, sim->parts[partIndex].y, newValue);
			break;
		default:
			break;
		}
		returnValue = 1;
	}
	else if (selector.GetType() == TypeString && ((StringType)selector).Value() == "all")
	{
		switch(propertyFormat)
		{
		case FormatInt:
			{
				for(int j = 0; j < NPART; j++)
					if(sim->parts[j].type)
					{
						returnValue++;
						*((int*)(partsBlock+(j*sizeof(Particle))+propertyOffset)) = newValue;
					}
			}
			break;
		case FormatFloat:
			{
				for(int j = 0; j < NPART; j++)
					if(sim->parts[j].type)
					{
						returnValue++;
						*((float*)(partsBlock+(j*sizeof(Particle))+propertyOffset)) = newValuef;
					}
			}
			break;
		case FormatElement:
			{
				for (int j = 0; j < NPART; j++)
					if (sim->parts[j].type)
					{
						returnValue++;
						sim->part_change_type(j, sim->parts[j].x, sim->parts[j].y, newValue);
					}
			}
			break;
		default:
			break;
		}
	}
	else if(selector.GetType() == TypeString || selector.GetType() == TypeNumber)
	{
		int type = 0;
		if (selector.GetType() == TypeNumber)
			type = ((NumberType)selector).Value();
		else if (selector.GetType() == TypeString)
			type = m->GetSimulation()->GetParticleType(((StringType)selector).Value().ToUtf8());

		if (type<0 || type>=PT_NUM)
			throw GeneralException("Invalid particle type");
		if (type==0)
			throw GeneralException("Cannot set properties of particles that do not exist");
		switch(propertyFormat)
		{
		case FormatInt:
			{
				for (int j = 0; j < NPART; j++)
					if (sim->parts[j].type == type)
					{
						returnValue++;
						*((int*)(partsBlock+(j*sizeof(Particle))+propertyOffset)) = newValue;
					}
			}
			break;
		case FormatFloat:
			{
				for (int j = 0; j < NPART; j++)
					if (sim->parts[j].type == type)
					{
						returnValue++;
						*((float*)(partsBlock+(j*sizeof(Particle))+propertyOffset)) = newValuef;
					}
			}
			break;
		case FormatElement:
			{
				for (int j = 0; j < NPART; j++)
					if (sim->parts[j].type == type)
					{
						returnValue++;
						sim->part_change_type(j, sim->parts[j].x, sim->parts[j].y, newValue);
					}
			}
			break;
		default:
			break;
		}
	}
	else
		throw GeneralException("Invalid selector");
	return NumberType(returnValue);
}

AnyType TPTScriptInterface::tptS_create(std::deque<String> * words)
{
	//Arguments from stack
	AnyType createType = eval(words);
	PointType position = eval(words);

	Simulation * sim = m->GetSimulation();

	int type;
	if(createType.GetType() == TypeNumber)
		type = ((NumberType)createType).Value();
	else if(createType.GetType() == TypeString)
		type = m->GetSimulation()->GetParticleType(((StringType)createType).Value().ToUtf8());
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

AnyType TPTScriptInterface::tptS_delete(std::deque<String> * words)
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

AnyType TPTScriptInterface::tptS_load(std::deque<String> * words)
{
	//Arguments from stack
	NumberType saveID = eval(words);

	if (saveID.Value() > 0)
	{
		c->OpenSavePreview(saveID.Value(), 0, false);
		return NumberType(0);
	}
	else
		throw GeneralException("Invalid save ID");
}

AnyType TPTScriptInterface::tptS_bubble(std::deque<String> * words)
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
		rem2 = sim->create_part(-1, bubblePos.X+18*cosf(i/5.0), bubblePos.Y+18*sinf(i/5.0), PT_SOAP);

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

AnyType TPTScriptInterface::tptS_reset(std::deque<String> * words)
{
	//Arguments from stack
	StringType reset = eval(words);
	String resetStr = reset.Value();

	Simulation * sim = m->GetSimulation();

	if (resetStr == "pressure")
	{
		for (int nx = 0; nx < XRES/CELL; nx++)
			for (int ny = 0; ny < YRES/CELL; ny++)
			{
				sim->air->pv[ny][nx] = 0;
			}
	}
	else if (resetStr == "velocity")
	{
		for (int nx = 0; nx < XRES/CELL; nx++)
			for (int ny = 0; ny < YRES/CELL; ny++)
			{
				sim->air->vx[ny][nx] = 0;
				sim->air->vy[ny][nx] = 0;
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
				sim->parts[i].temp = sim->elements[sim->parts[i].type].DefaultProperties.temp;
			}
		}
	}
	else
	{
		throw GeneralException("Unknown reset command");
	}

	return NumberType(0);
}

AnyType TPTScriptInterface::tptS_quit(std::deque<String> * words)
{
	ui::Engine::Ref().Exit();

	return NumberType(0);
}

TPTScriptInterface::~TPTScriptInterface() {
}

