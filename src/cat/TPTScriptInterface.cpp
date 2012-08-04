/*
 * TPTScriptInterface.cpp
 *
 *  Created on: Feb 5, 2012
 *      Author: Simon
 */

#include <stack>
#include <iostream>
#include <string>
#include <deque>
#include <string.h>
#include <stdlib.h>
#include "TPTScriptInterface.h"
#include "game/GameModel.h"

TPTScriptInterface::TPTScriptInterface(GameModel * m): CommandInterface(m)
{
}

int TPTScriptInterface::Command(std::string command)
{
	lastError = "";
	std::deque<std::string> words;
	std::deque<AnyType> commandWords;
	int retCode;

	//Split command into words, put them on the stack
	char * rawCommand;
	rawCommand = (char*)calloc(command.length()+1, 1);
	memcpy(rawCommand, (char*)command.c_str(), command.length());
	char * currentWord = rawCommand;
	char * currentCommand = rawCommand;
	while((currentCommand = strchr(currentCommand, ' ')))
	{
		currentCommand[0] = 0;
		words.push_back(std::string(currentWord));
		currentWord = ++currentCommand;
	}
	words.push_back(std::string(currentWord));
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
	free(rawCommand);
	if(commandWords.size())
	{
		retCode = 0;
		lastError = ((StringType)commandWords.front()).Value();
	}

	//Evaluate
	return 0;
}

ValueType TPTScriptInterface::testType(std::string word)
{
	int i = 0;
	char * rawWord = (char *)word.c_str();
	//Function
	if(word == "set")
		return TypeFunction;
	else if(word == "create")
		return TypeFunction;
	else if(word == "delete")
		return TypeFunction;
	else if(word == "kill")
		return TypeFunction;
	else if(word == "load")
		return TypeFunction;
	else if(word == "bubble")
		return TypeFunction;
	//Basic type
	parseNumber:
			for(i = 0; i < word.length(); i++)
				if(!(rawWord[i] >= '0' && rawWord[i] <= '9'))
				{
					if(rawWord[i] == ',' && rawWord[i+1] >= '0' && rawWord[i+1] <= '9')
						goto parsePoint;
					else
						goto parseString;
				}
			return TypeNumber;
	parsePoint:
			i++;
			for(; i < word.length(); i++)
				if(!(rawWord[i] >= '0' && rawWord[i] <= '9'))
				{
					goto parseString;
				}
			return TypePoint;
	parseString:
			return TypeString;
}

AnyType TPTScriptInterface::eval(std::deque<std::string> * words)
{
	if(words->size() < 1)
		return AnyType(TypeNull, NULL);
	std::string word = words->front(); words->pop_front();
	char * rawWord = (char *)word.c_str();
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
		break;
	case TypeNumber:
		return NumberType(atoi(rawWord));
	case TypePoint:
	{
		int pointX, pointY;
		sscanf(rawWord, "%d,%d", &pointX, &pointY);
		return PointType(pointX, pointY);
	}
	case TypeString:
		return StringType(word);
	}
}

std::string TPTScriptInterface::FormatCommand(std::string command)
{
	std::deque<std::string> words;
	std::deque<AnyType> commandWords;
	std::string outputData;

	//Split command into words, put them on the stack
	char * rawCommand;
	rawCommand = (char*)calloc(command.length()+1, 1);
	memcpy(rawCommand, (char*)command.c_str(), command.length());
	char * currentWord = rawCommand;
	char * currentCommand = rawCommand;
	while((currentCommand = strchr(currentCommand, ' ')))
	{
		currentCommand[0] = 0;
		words.push_back(std::string(currentWord));
		currentWord = ++currentCommand;
	}
	words.push_back(std::string(currentWord));
	free(rawCommand);
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

AnyType TPTScriptInterface::tptS_set(std::deque<std::string> * words)
{
	//Arguments from stack
	StringType property = eval(words);
	AnyType selector = eval(words);
	AnyType value = eval(words);

	Simulation * sim = m->GetSimulation();

	int returnValue = 0;

	FormatType propertyFormat;
	int propertyOffset = GetPropertyOffset(property.Value(), propertyFormat);

	if(propertyOffset==-1)
		throw GeneralException("Invalid property");

	//Selector
	int newValue;
	if(selector.GetType() == TypeNumber)
		newValue = ((NumberType)selector).Value();
	else if(selector.GetType() == TypeString)
		newValue = GetParticleType(((StringType)selector).Value());
	else
		throw GeneralException("Invalid value for assignment");

	if(selector.GetType() == TypePoint || selector.GetType() == TypeNumber)
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
		if(partIndex<0 || partIndex>NPART || sim->parts[partIndex].type==0)
			throw GeneralException("Invalid particle");

		switch(propertyFormat)
		{
		case FormatInt:
			*((int*)(((unsigned char*)&sim->parts[partIndex])+propertyOffset)) = newValue;
			break;
		case FormatFloat:
			*((float*)(((unsigned char*)&sim->parts[partIndex])+propertyOffset)) = newValue;
			break;
		}
		returnValue = 1;
	}
	else if(selector.GetType() == TypeString && ((StringType)selector).Value() == "all")
	{
		switch(propertyFormat)
		{
		case FormatInt:
			{
				for(int j = 0; j < NPART; j++)
					if(sim->parts[j].type)
					{
						returnValue++;
						*((int*)(((unsigned char*)&sim->parts[j])+propertyOffset)) = newValue;
					}
			}
			break;
		case FormatFloat:
			{
				for(int j = 0; j < NPART; j++)
					if(sim->parts[j].type)
					{
						returnValue++;
						*((float*)(((unsigned char*)&sim->parts[j])+propertyOffset)) = newValue;
					}
			}
			break;
		}
	}
	else if(selector.GetType() == TypeString || selector.GetType() == TypeNumber)
	{
		int type;
		if(selector.GetType() == TypeNumber)
			type = ((NumberType)selector).Value();
		else if(selector.GetType() == TypeString)
			type = GetParticleType(((StringType)selector).Value());

		if(type<0 || type>=PT_NUM)
			throw GeneralException("Invalid particle type");
		switch(propertyFormat)
		{
		case FormatInt:
			{
				for(int j = 0; j < NPART; j++)
					if(sim->parts[j].type == type)
					{
						returnValue++;
						*((int*)(((unsigned char*)&sim->parts[j])+propertyOffset)) = newValue;
					}
			}
			break;
		case FormatFloat:
			{
				for(int j = 0; j < NPART; j++)
					if(sim->parts[j].type == type)
					{
						returnValue++;
						*((float*)(((unsigned char*)&sim->parts[j])+propertyOffset)) = newValue;
					}
			}
			break;
		}
	}
	else
		throw GeneralException("Invalid selector");
	return NumberType(returnValue);
}

AnyType TPTScriptInterface::tptS_create(std::deque<std::string> * words)
{
	//Arguments from stack
	AnyType createType = eval(words);
	PointType position = eval(words);

	Simulation * sim = m->GetSimulation();

	int type;
	if(createType.GetType() == TypeNumber)
		type = ((NumberType)createType).Value();
	else if(createType.GetType() == TypeString)
		type = GetParticleType(((StringType)createType).Value());
	else
		throw GeneralException("Invalid type");

	if(type == -1)
		throw GeneralException("Invalid particle type");

	ui::Point tempPoint = position.Value();
	if(tempPoint.X<0 || tempPoint.Y<0 || tempPoint.Y >= YRES || tempPoint.X >= XRES)
				throw GeneralException("Invalid position");

	int returnValue = sim->create_part(-1, tempPoint.X, tempPoint.Y, type);

	return NumberType(returnValue);
}

AnyType TPTScriptInterface::tptS_delete(std::deque<std::string> * words)
{
	//Arguments from stack
	AnyType partRef = eval(words);

	Simulation * sim = m->GetSimulation();

	if(partRef.GetType() == TypePoint)
	{
		ui::Point deletePoint = ((PointType)partRef).Value();
		if(deletePoint.X<0 || deletePoint.Y<0 || deletePoint.Y >= YRES || deletePoint.X >= XRES)
			throw GeneralException("Invalid position");
		sim->delete_part(deletePoint.X, deletePoint.Y, 0);
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

AnyType TPTScriptInterface::tptS_load(std::deque<std::string> * words)
{
	//Arguments from stack
	NumberType saveID = eval(words);

	return NumberType(0);
}

TPTScriptInterface::~TPTScriptInterface() {
}

