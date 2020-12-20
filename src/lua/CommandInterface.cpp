#include "CommandInterface.h"

#include <cstring>
#include <cstddef>
#if !defined(WIN) || defined(__GNUC__)
#include <strings.h>
#endif

#include "gui/game/GameModel.h"

#include "simulation/Particle.h"

CommandInterface::CommandInterface(GameController * c, GameModel * m) {
	this->m = m;
	this->c = c;
}

/*void CommandInterface::AttachGameModel(GameModel * m)
{
	this->m = m;
}*/

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
	if (!key.compare("type"))
	{
		offset = offsetof(Particle, type);
		format = FormatElement;
	}
	else if (!key.compare("life"))
	{
		offset = offsetof(Particle, life);
		format = FormatInt;
	}
	else if (!key.compare("ctype"))
	{
		offset = offsetof(Particle, ctype);
		format = FormatInt;
	}
	else if (!key.compare("temp"))
	{
		offset = offsetof(Particle, temp);
		format = FormatFloat;
	}
	else if (!key.compare("tmp2"))
	{
		offset = offsetof(Particle, tmp2);
		format = FormatInt;
	}
	else if (!key.compare("tmp"))
	{
		offset = offsetof(Particle, tmp);
		format = FormatInt;
	}
	else if (!key.compare("vy"))
	{
		offset = offsetof(Particle, vy);
		format = FormatFloat;
	}
	else if (!key.compare("vx"))
	{
		offset = offsetof(Particle, vx);
		format = FormatFloat;
	}
	else if (!key.compare("x"))
	{
		offset = offsetof(Particle, x);
		format = FormatFloat;
	}
	else if (!key.compare("y"))
	{
		offset = offsetof(Particle, y);
		format = FormatFloat;
	}
	else if (!key.compare("dcolor") || !key.compare("dcolour"))
	{
		offset = offsetof(Particle, dcolour);
		format = FormatInt;
	}
	else if (!key.compare("pavg0"))
	{
		offset = offsetof(Particle, pavg[0]);
		format = FormatFloat;
	}
	else if (!key.compare("pavg1"))
	{
		offset = offsetof(Particle, pavg[1]);
		format = FormatFloat;
	}
	return offset;
}

String CommandInterface::GetLastError()
{
	return lastError;
}

CommandInterface::~CommandInterface() {
}

