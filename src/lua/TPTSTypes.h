#ifndef TPTSTYPES_H_
#define TPTSTYPES_H_

#include <string>
#include <typeinfo>
#include "gui/interface/Point.h"

enum ValueType { TypeNumber, TypePoint, TypeString, TypeNull, TypeFunction };
typedef union { int num; std::string* str; ui::Point* pt; } ValueValue;

class GeneralException
{
protected:
	std::string exception;
public:
	GeneralException(std::string message){
		exception = message;
	}
	std::string GetExceptionMessage() {
		return exception;
	}
};


class NumberType;
class StringType;
class PointType;

class AnyType
{
protected:
	ValueType type;
	ValueValue value;
public:
	AnyType(ValueType type_, ValueValue value_);
	AnyType(const AnyType & v);
	operator NumberType();
	operator StringType();
	operator PointType();
	ValueType GetType();
	std::string TypeName()
	{
		switch(type)
		{
		case TypeNumber:
			return "Number";
		case TypePoint:
			return "Point";
		case TypeString:
			return "String";
		case TypeNull:
			return "Null";
		case TypeFunction:
			return "Function";
		default:
			return "Unknown";
		}
	}
	static std::string TypeName(ValueType type)
	{
		switch(type)
		{
		case TypeNumber:
			return "Number";
		case TypePoint:
			return "Point";
		case TypeString:
			return "String";
		case TypeNull:
			return "Null";
		case TypeFunction:
			return "Function";
		default:
			return "Unknown";
		}
	}
	~AnyType();
};

class InvalidConversionException: public GeneralException
{
private:
	ValueType from;
	ValueType to;
public:
	InvalidConversionException(ValueType from_, ValueType to_):
	GeneralException("Invalid conversion from " + AnyType::TypeName(from_) + " to " + AnyType::TypeName(to_)), from(from_), to(to_) {
	}
};

class NumberType: public AnyType
{
public:
	NumberType(int number);
	int Value();
};

class StringType: public AnyType
{
public:
	StringType(std::string string);
	std::string Value();
};

class PointType: public AnyType
{
public:
	PointType(ui::Point point);
	PointType(int pointX, int pointY);
	ui::Point Value();
};

#endif /* TPTSTYPES_H_ */
