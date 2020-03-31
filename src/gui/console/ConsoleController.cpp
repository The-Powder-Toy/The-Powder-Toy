#include "ConsoleController.h"

#include "Controller.h"
#include "ConsoleView.h"
#include "ConsoleModel.h"
#include "ConsoleCommand.h"

#include "lua/CommandInterface.h"

ConsoleController::ConsoleController(std::function<void ()> onDone_, CommandInterface * commandInterface):
	HasDone(false)
{
	consoleModel = new ConsoleModel();
	consoleView = new ConsoleView();
	consoleView->AttachController(this);
	consoleModel->AddObserver(consoleView);

	onDone = onDone_;
	this->commandInterface = commandInterface;
}

void ConsoleController::EvaluateCommand(String command)
{
	if(command.length())
	{
		if (command.BeginsWith("!load "))
			CloseConsole();
		int returnCode = commandInterface->Command(command);
		consoleModel->AddLastCommand(ConsoleCommand(command, returnCode, commandInterface->GetLastError()));
	}
	else
		CloseConsole();
}

void ConsoleController::CloseConsole()
{
	consoleView->CloseActiveWindow();
}

String ConsoleController::FormatCommand(String command)
{
	return commandInterface->FormatCommand(command);
}

void ConsoleController::NextCommand()
{
	size_t cIndex = consoleModel->GetCurrentCommandIndex();
	if (cIndex < consoleModel->GetPreviousCommands().size())
		consoleModel->SetCurrentCommandIndex(cIndex+1);
}

void ConsoleController::PreviousCommand()
{
	size_t cIndex = consoleModel->GetCurrentCommandIndex();
	if(cIndex > 0)
		consoleModel->SetCurrentCommandIndex(cIndex-1);
}

void ConsoleController::Exit()
{
	consoleView->CloseActiveWindow();
	if (onDone)
		onDone();
	HasDone = true;
}

ConsoleView * ConsoleController::GetView()
{
	return consoleView;
}

ConsoleController::~ConsoleController()
{
	consoleView->CloseActiveWindow();
	delete consoleModel;
	delete consoleView;
}

