#pragma once
#include <memory>

class CommandInterface;
struct CommandInterfaceDeleter
{
	void operator ()(CommandInterface *ptr) const;
};
using CommandInterfacePtr = std::unique_ptr<CommandInterface, CommandInterfaceDeleter>;
