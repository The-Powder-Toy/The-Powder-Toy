/*
 * Kitty.cpp
 *
 *  Created on: Feb 2, 2012
 *      Author: Simon
 */

#include <iostream>
#include <string>
#include <string.h>
#include <strings.h>
#include "CommandInterface.h"
#include "game/GameModel.h"

CommandInterface::CommandInterface(GameModel * m) {
	this->m = m;
}

/*void CommandInterface::AttachGameModel(GameModel * m)
{
	this->m = m;
}*/

int CommandInterface::Command(std::string command)
{
	lastError = "No interpreter";
	return -1;
}

std::string CommandInterface::FormatCommand(std::string command)
{
	return command;
}

void CommandInterface::Log(LogType type, std::string message)
{
	m->Log(message);
}

int CommandInterface::GetPropertyOffset(std::string key_, FormatType & format)
{
	char * key = (char *)key_.c_str();
	int offset;
	if (strcmp(key, "type")==0){
		offset = offsetof(Particle, type);
		format = FormatInt;
	} else if (strcmp(key, "life")==0){
		offset = offsetof(Particle, life);
		format = FormatInt;
	} else if (strcmp(key, "ctype")==0){
		offset = offsetof(Particle, ctype);
		format = FormatInt;
	} else if (strcmp(key, "temp")==0){
		offset = offsetof(Particle, temp);
		format = FormatFloat;
	} else if (strcmp(key, "tmp2")==0){
		offset = offsetof(Particle, tmp2);
		format = FormatInt;
	} else if (strcmp(key, "tmp")==0){
		offset = offsetof(Particle, tmp);
		format = FormatInt;
	} else if (strcmp(key, "vy")==0){
		offset = offsetof(Particle, vy);
		format = FormatFloat;
	} else if (strcmp(key, "vx")==0){
		offset = offsetof(Particle, vx);
		format = FormatFloat;
	} else if (strcmp(key, "x")==0){
		offset = offsetof(Particle, x);
		format = FormatFloat;
	} else if (strcmp(key, "y")==0){
		offset = offsetof(Particle, y);
		format = FormatFloat;
	} else if (strcmp(key, "dcolour")==0){
		offset = offsetof(Particle, dcolour);
		format = FormatInt;
	} else if (strcmp(key, "dcolor")==0){
		offset = offsetof(Particle, dcolour);
		format = FormatInt;
	} else {
		offset = -1;
	}
	return offset;
}

int CommandInterface::GetParticleType(std::string type)
{
	int i = -1;
	char * txt = (char*)type.c_str();

	//Scope
	Element * elements = m->GetSimulation()->elements;

	// alternative names for some elements
	if (strcasecmp(txt,"C4")==0) i = PT_PLEX;
	else if (strcasecmp(txt,"C5")==0) i = PT_C5;
	else if (strcasecmp(txt,"NONE")==0) i = PT_NONE;
	for (i=1; i<PT_NUM; i++) {
		if (strcasecmp(txt, elements[i].Name)==0 && elements[i].Enabled)
		{
			return i;
		}
	}
	return i;
}

std::string CommandInterface::GetLastError()
{
	return lastError;
}

CommandInterface::~CommandInterface() {
}

