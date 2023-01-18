#include "CommandInterface.h"

#include <cstring>
#include <cstddef>
#include <cassert>

#include "Misc.h"
#include "gui/game/GameModel.h"
#include "simulation/Particle.h"

CommandInterface *commandInterface = nullptr;

CommandInterface::CommandInterface(GameController * c, GameModel * m)
{
	assert(!commandInterface);
	commandInterface = this;
	this->m = m;
	this->c = c;
}

CommandInterface::~CommandInterface()
{
	commandInterface = nullptr;
}

int CommandInterface::Command(String command)
{
	lastError = "No interpreter";
	return -1;
}

String CommandInterface::FormatCommand(String command)
{
	return command;
}

void CommandInterface::Log(LogType type, String message)
{
	m->Log(message, type == LogError || type == LogNotice);
}

int CommandInterface::GetPropertyOffset(ByteString key, FormatType & format)
{
	int offset = -1;
	for (auto &alias : Particle::GetPropertyAliases())
	{
		if (key == alias.from)
		{
			key = alias.to;
		}
	}
	for (auto &prop : Particle::GetProperties())
	{
		if (key == prop.Name)
		{
			offset = prop.Offset;
			switch (prop.Type)
			{
			case StructProperty::ParticleType:
				format = byteStringEqualsLiteral(key, "type") ? FormatElement : FormatInt; // FormatElement is tightly coupled with "type"
				break;

			case StructProperty::Integer:
			case StructProperty::UInteger:
				format = FormatInt;
				break;

			case StructProperty::Float:
				format = FormatFloat;
				break;

			default:
				break;
			}
		}
	}
	return offset;
}

String CommandInterface::GetLastError()
{
	return lastError;
}
