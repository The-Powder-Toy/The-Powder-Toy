#pragma once
#include <string>
#include <vector>
#include <sstream>
namespace pim
{
	struct DataType
	{
		public:
		enum
		{
			Integer,
			Float
		};
	};
	static std::string TypeName(int type) 
	{
		if(type == DataType::Float)
			return "decimal";
		if(type == DataType::Integer)
			return "integer";
	}
	static std::string ArgsName(std::vector<int> args)
	{
		std::stringstream newStream("");
		for(std::vector<int>::iterator iter = args.begin(), end = args.end(); iter != end; iter++)
		{
			if(*iter == DataType::Float)
				newStream << "D";
			else if(*iter == DataType::Integer)
				newStream << "I";
			else
				newStream << "V";
		}
		return newStream.str();
	}
}