#include "CommandInterface.h"

CommandInterface *CommandInterface::Create(GameController * c, GameModel * m)
{
	return new CommandInterface(c, m);
}
