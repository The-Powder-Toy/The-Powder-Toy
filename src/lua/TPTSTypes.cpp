#include <iostream>
#include <stdint.h>
#include "TPTSTypes.h"

AnyType::AnyType(ValueType type_, ValueValue value_):
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
		value.str = new String(*(value.str));
	else if(type == TypePoint)
		value.pt = new ui::Point(*(value.pt));
}

AnyType::operator NumberType()
{
	if (type == TypeNumber)
		return NumberType(value.num);
	else if (type == TypeFloat)
		return NumberType(value.numf);
	else
		throw InvalidConversionException(type, TypeNumber);
}

AnyType::operator FloatType()
{
	if (type == TypeNumber)
		return FloatType(value.num);
	else if (type == TypeFloat)
		return FloatType(value.numf);
	else
		throw InvalidConversionException(type, TypeFloat);
}

AnyType::operator StringType()
{
	if(type == TypeNumber)
	{
		return StringType(String::Build(((NumberType *)this)->Value()));
	}
	else if(type == TypeString && value.str)
	{
		return StringType(*(value.str));
	}
	else if (type == TypePoint && value.pt)
	{
		ui::Point thisPoint = *(value.pt);
		return StringType(String::Build(thisPoint.X, ",", thisPoint.Y));
	}
	else
		throw InvalidConversionException(type, TypeString);

}

AnyType::operator PointType()
{
	if(type == TypePoint)
	{
		return PointType(*(value.pt));
	}
	else if(type == TypeString)
	{
		int x, y;
		if(String::Split comma = (*value.str).SplitNumber(x))
			if(comma.After().BeginsWith(","))
				if(String::Split end = comma.After().Substr(1).SplitNumber(y))
					if(!end.After().size())
						return PointType(x, y);
		throw InvalidConversionException(type, TypePoint);
	}
	else
		throw InvalidConversionException(type, TypePoint);
}

AnyType::~AnyType()
{
	if(type == TypeString)
		delete value.str;
	else if(type == TypePoint)
		delete value.pt;
}

//Number Type

NumberType::NumberType(int number): AnyType(TypeNumber, ValueValue())
{
	value.num = number;
}

int NumberType::Value()
{
	return value.num;
}

//Float Type

FloatType::FloatType(float number): AnyType(TypeFloat, ValueValue())
{
	value.numf = number;
}

float FloatType::Value()
{
	return value.numf;
}

//String type

StringType::StringType(String string):	AnyType(TypeString, ValueValue())
{
	value.str = new String(string);
}

String StringType::Value()
{
	return *value.str;
}

//Point type

PointType::PointType(ui::Point point): AnyType(TypePoint, ValueValue())
{
	value.pt = new ui::Point(point);
}

PointType::PointType(int pointX, int pointY): AnyType(TypePoint, ValueValue())
{
	value.pt = new ui::Point(pointX, pointY);
}

ui::Point PointType::Value()
{
	return *value.pt;
}
