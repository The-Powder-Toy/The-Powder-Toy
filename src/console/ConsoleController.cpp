/*
 * ConsoleController.cpp
 *
 *  Created on: Jan 31, 2012
 *      Author: Simon
 */

#include <stack>
#include "ConsoleController.h"

ConsoleController::ConsoleController(ControllerCallback * callback, CommandInterface * commandInterface):
	HasDone(false)
{
	consoleModel = new ConsoleModel();
	consoleView = new ConsoleView();
	consoleView->AttachController(this);
	consoleModel->AddObserver(consoleView);

	this->callback = callback;
	this->commandInterface = commandInterface;
}

void ConsoleController::EvaluateCommand(std::string command)
{
	if (command.substr(0, 5) == "!load ")
		CloseConsole();
	int returnCode = commandInterface->Command(command);
	if(command.length())
		consoleModel->AddLastCommand(ConsoleCommand(command, returnCode, commandInterface->GetLastError()));
	else
		CloseConsole();
}

void ConsoleController::CloseConsole()
{
	if(ui::Engine::Ref().GetWindow() == consoleView)
		ui::Engine::Ref().CloseWindow();
}

std::string ConsoleController::FormatCommand(std::string command)
{
	return commandInterface->FormatCommand(command);
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
	if(ui::Engine::Ref().GetWindow() == consoleView)
		ui::Engine::Ref().CloseWindow();
	if(callback)
		delete callback;
	delete consoleModel;
	delete consoleView;
}

