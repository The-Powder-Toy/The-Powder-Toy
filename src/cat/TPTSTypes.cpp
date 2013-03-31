#include <iostream>
#include <sstream>
#include <stdint.h>
#include "TPTSTypes.h"

AnyType::AnyType(ValueType type_, void * value_):
	type(type_),
	value(value_)
{
}

ValueType AnyType::GetType()
{
	return type;
}

AnyType::AnyType(const AnyType & v):
	type(v.type),
	value(v.value)
{
	if(type == TypeString)
	{
		value = new std::string(*((std::string*)value));
	}
	else if(type == TypePoint)
	{
		value = new ui::Point(*((ui::Point*)value));
	}
}

AnyType::operator NumberType()
{
	if(type != TypeNumber)
		throw InvalidConversionException(type, TypeNumber);
	else
		return NumberType((intptr_t)value);
}

AnyType::operator StringType()
{
	if(type == TypeNumber)
	{
		std::stringstream numberStream;
		numberStream << ((NumberType*)this)->Value();
		return StringType(numberStream.str());
	}
	else if(type == TypeString && value)
	{
		return StringType(*((std::string*)value));
	}
	else
		throw InvalidConversionException(type, TypeString);

}

AnyType::operator PointType()
{
	if(type == TypePoint)
	{
		return PointType(*((ui::Point*)value));
	}
	else if(type == TypeString)
	{
		ui::Point thisPoint = *((ui::Point*)value);
		std::stringstream pointStream;
		pointStream << thisPoint.X << "," << thisPoint.Y;
		return StringType(pointStream.str());
	}
	else
		throw InvalidConversionException(type, TypePoint);
}

AnyType::~AnyType()
{
	if(type == TypeString || type == TypePoint)
		delete value;
}

//Number Type

NumberType::NumberType(int number):	AnyType(TypeNumber, (void*)number) { }

int NumberType::Value()
{
	return (intptr_t)value;
}

//String type

StringType::StringType(std::string string):	AnyType(TypeString, new std::string(string)) { }

std::string StringType::Value()
{
	return std::string(*((std::string*)value));
}

//Point type

PointType::PointType(ui::Point point): AnyType(TypePoint, new ui::Point(point)) { }

PointType::PointType(int pointX, int pointY): AnyType(TypePoint, new ui::Point(pointX, pointY)) { }

ui::Point PointType::Value()
{
	return ui::Point(*((ui::Point*)value));
}
