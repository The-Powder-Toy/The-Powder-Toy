#pragma once
#include <stdexcept>
#include <cstring>
#include "Format.h"

namespace vm
{
	class RuntimeException: public std::exception
	{
		char * error;
	public:
		RuntimeException() : error(NULL) {}
		RuntimeException(char * message) : error(strdup(message)) {}
		const char * what() const throw()
		{
			if(error)
				return error;
			else
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

	class JITException: public RuntimeException
	{
		char * _what;
	public:
		JITException(const char * what2)
		{
			_what = strdup(what2);
		}
		const char * what() const throw()
		{
			return _what;
		}
		~JITException() throw() {};
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

	class InvalidProgramException: public RuntimeException
	{
	public:
		InvalidProgramException() {}
		const char * what() const throw()
		{
			return "Could not load program";
		}
		~InvalidProgramException() throw() {};
	};
}
