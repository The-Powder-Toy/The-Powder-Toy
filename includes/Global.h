#ifndef GAMESESSION_H
#define GAMESESSION_H

#include "Singleton.h"
#include "Graphics.h"

class Global : public Singleton<Global>
{
public:
	Graphics * g;
	Global();
};

#endif // GAMESESSION_H
