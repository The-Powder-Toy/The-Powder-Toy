#include "CommandInterface.h"
#include "TPTScriptInterface.h"

CommandInterface *CommandInterface::Create(GameController * c, GameModel * m)
{
	return new TPTScriptInterface(c, m);
}
