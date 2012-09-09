#pragma once
#include <stdexcept>
#include <cstring>
#include "Format.h"

namespace vm
{
	class RuntimeException: public std::exception
	{
	public:
		RuntimeException() {}
		const char * what() const throw()
		{
			return "VirtualMachine runtime exception";
		}
		~RuntimeException() throw() {};
	};

	class StackOverflowException: public RuntimeException
	{
	public:
		StackOverflowException() {}
		const char * what() const throw()
		{
			return "VirtualMachine Stack overflow";
		}
		~StackOverflowException() throw() {};
	};

	class StackUnderflowException: public RuntimeException
	{
	public:
		StackUnderflowException() {}
		const char * what() const throw()
		{
			return "VirtualMachine Stack underflow";
		}
		~StackUnderflowException() throw() {};
	};

	class AccessViolationException: public RuntimeException
	{
		int address;
		char * _what;
	public:
		AccessViolationException(int address = 0) : address(address)
		{
			_what = strdup(std::string("VirtualMachine Access violation at "+format::NumberToString<int>(address)).c_str());
		}
		const char * what() const throw()
		{
			if(address)
				return _what;
			return "VirtualMachine Access violation";
		}
		~AccessViolationException() throw() {};
	};

	class OutOfMemoryException: public RuntimeException
	{
	public:
		OutOfMemoryException() {}
		const char * what() const throw()
		{
			return "VirtualMachine Out of memory";
		}
		~OutOfMemoryException() throw() {};
	};
}