/*
 * ConsoleController.cpp
 *
 *  Created on: Jan 31, 2012
 *      Author: Simon
 */

#include <stack>
#include "ConsoleController.h"

ConsoleController::ConsoleController(ControllerCallback * callback):
	HasDone(false)
{
	consoleModel = new ConsoleModel();
	consoleView = new ConsoleView();
	consoleView->AttachController(this);
	consoleModel->AddObserver(consoleView);

	this->callback = callback;
}

void ConsoleController::EvaluateCommand(std::string command)
{
	if(command.length())
		consoleModel->AddLastCommand(ConsoleCommand(command, -1, "Syntax error"));
	else
		if(ui::Engine::Ref().GetWindow() == consoleView)
			ui::Engine::Ref().CloseWindow();
}

std::string ConsoleController::FormatCommand(std::string command)
{
	char * rawText = (char*)command.c_str();
	char * outputData = (char *)calloc(command.length()*6, 1);
	int rawTextLoc = 0;
	int outputDataLoc = 0;
	std::stack<char> pstack;
	while(rawText[rawTextLoc])
	{
		switch(rawText[rawTextLoc])
		{
		case '\\':
			outputData[outputDataLoc++] = rawText[rawTextLoc++];
			if(rawText[rawTextLoc])
				outputData[outputDataLoc++] = rawText[rawTextLoc++];
			break;
		case '"':
			if(pstack.size() && pstack.top() == '"')
			{
				pstack.pop();
				outputData[outputDataLoc++] = rawText[rawTextLoc++];
				outputData[outputDataLoc++] = '\b';
				outputData[outputDataLoc++] = 'w';
			}
			else
			{
				pstack.push('"');
				outputData[outputDataLoc++] = '\b';
				outputData[outputDataLoc++] = 'o';
				outputData[outputDataLoc++] = rawText[rawTextLoc++];
			}
			break;
		default:
			outputData[outputDataLoc++] = rawText[rawTextLoc++];
			break;
		}
	}
	return outputData;
}

void ConsoleController::NextCommand()
{
	int cIndex = consoleModel->GetCurrentCommandIndex();
	if(cIndex < consoleModel->GetPreviousCommands().size())
		consoleModel->SetCurrentCommandIndex(cIndex+1);
}

void ConsoleController::PreviousCommand()
{
	int cIndex = consoleModel->GetCurrentCommandIndex();
	if(cIndex > 0)
		consoleModel->SetCurrentCommandIndex(cIndex-1);
}

void ConsoleController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == consoleView)
		ui::Engine::Ref().CloseWindow();
	if(callback)
		callback->ControllerExit();
	HasDone = true;
}

ConsoleView * ConsoleController::GetView()
{
	return consoleView;
}

ConsoleController::~ConsoleController() {
	// TODO Auto-generated destructor stub
}

