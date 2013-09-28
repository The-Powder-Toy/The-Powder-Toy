#include <algorithm>
#include <cstdio>
#include "client/Client.h"
#include "Misc.h"
#include "ConsoleModel.h"

ConsoleModel::ConsoleModel() {
	std::vector<std::string> previousHistory = Client::Ref().GetPrefStringArray("Console.History");
	for(std::vector<std::string>::iterator iter = previousHistory.begin(), end = previousHistory.end(); iter != end; ++iter)
	{
		if(previousCommands.size()<25)
		{
			previousCommands.push_back(ConsoleCommand(*iter, 0, ""));
		}
		else
			break;
	}
	currentCommandIndex=-1;
}

void ConsoleModel::AddObserver(ConsoleView * observer)
{
	observers.push_back(observer);
}

void ConsoleModel::ProcessResult(std::string cmd, std::string highlighted, CommandInterface::EvalResult * result)
{
	int newlines;
	switch(result->status)
	{
		case CommandInterface::EvalFail:
		case CommandInterface::EvalSuccess:
			{
				std::string buffer = wordwrap(result->buffer, XRES-20);
				int ln,inverted=0;
				previousCommands.push_front(cmd);
				cmd = wordwrap(cmd, XRES-20);
				newlines = std::count(cmd.begin(), cmd.end(), '\n');
				history += '\n' + highlighted;
				promptHistory += "\n>";
				for(; newlines>0; newlines--)
					promptHistory += "\n>>";
				if(buffer.size())
				{
					newlines = 1;
					for(std::string::iterator it = buffer.begin(), end = buffer.end(); it != end; it++)
					{
						if(*it=='\n')
							newlines++;
						if(*it=='\x01')
							inverted=!inverted;
						if(*it=='\x0F')
						{
							if(++it==end)
								continue;
							if(*it=='\n' || *it=='\x00')
								*it+=1;
							if(++it==end)
								continue;
							if(*it=='\n' || *it=='\x00')
								*it+=1;
							if(++it==end)
								continue;
							if(*it=='\n' || *it=='\x00')
								*it+=1;
						}
					}
					history += "\n\bo" + buffer + "\bw";
					if(inverted)
						history += '\x01';
					for(; newlines>0; newlines--)
						promptHistory += '\n';
				}
				ln = 512;
				for(std::string::iterator it = history.end(), begin = history.begin(); it != begin; it--)
					if(*it=='\n')
						if(!--ln)
						{
							history = history.substr(it-begin);
							break;
						}
				ln = 512;
				for(std::string::iterator it = promptHistory.end(), begin = promptHistory.begin(); it != begin; it--)
					if(*it=='\n')
						if(!--ln)
						{
							promptHistory = promptHistory.substr(it-begin);
							break;
						}
				command = "";
				currentCommandIndex = -1;
			}
			break;
		case CommandInterface::EvalMore:
			command = cmd + '\n';
			newlines = 1 + std::count(cmd.begin(), cmd.end(), '\n');
			break;
	}
	delete result;
	notifyHistoryChanged();
}

void ConsoleModel::notifyHistoryChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyHistoryChanged(this, command, promptHistory, history);
	}
}

void ConsoleModel::NextCommand()
{
	int newlines;
	if(currentCommandIndex>-1)
	{
		currentCommandIndex--;
		if(currentCommandIndex>-1)
			command=previousCommands.at(currentCommandIndex);
		else
			command="";
		newlines = std::count(command.begin(), command.end(), '\n');
		notifyHistoryChanged();
	}
}

void ConsoleModel::PreviousCommand()
{
	int newlines;
	if(currentCommandIndex+1<previousCommands.size())
	{
		currentCommandIndex++;
		command=previousCommands.at(currentCommandIndex);
		newlines = std::count(command.begin(), command.end(), '\n');
		notifyHistoryChanged();
	}
}

ConsoleModel::~ConsoleModel() {
	Client::Ref().SetPref("Console.History", std::vector<std::string>(previousCommands.begin(), previousCommands.end()));
}

