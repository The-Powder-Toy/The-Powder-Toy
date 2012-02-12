/*
 * Kitty.h
 *
 *  Created on: Feb 2, 2012
 *      Author: Simon
 */

#ifndef KITTY_H_
#define KITTY_H_

#include <string>
//#include "game/GameModel.h"

class GameModel;
class CommandInterface {
protected:
	std::string lastError;
	GameModel * m;
public:
	enum FormatType { FormatInt, FormatString, FormatChar, FormatFloat };
	CommandInterface(GameModel * m);
	int GetPropertyOffset(std::string key_, FormatType & format);
	int GetParticleType(std::string type);
	//void AttachGameModel(GameModel * m);
	virtual void Tick() {}
	virtual int Command(std::string command);
	virtual std::string FormatCommand(std::string command);
	std::string GetLastError();
	virtual ~CommandInterface();
};

#endif /* KITTY_H_ */
