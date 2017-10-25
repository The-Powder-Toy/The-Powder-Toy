#include "ConsoleView.h"
#include "gui/interface/Keys.h"
#include "math.h"

ConsoleView::ConsoleView():
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, 150)),
	commandField(NULL)
{
	class CommandHighlighter: public ui::TextboxAction
	{
		ConsoleView * v;
	public:
		CommandHighlighter(ConsoleView * v_) { v = v_; }
		virtual void TextChangedCallback(ui::Textbox * sender)
		{
			sender->SetDisplayText(v->c->FormatCommand(sender->GetText()));
		}
	};
	commandField = new ui::Textbox(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "");
	commandField->SetMultiline(true);
	commandField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	commandField->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	commandField->SetActionCallback(new CommandHighlighter(this));
	AddComponent(commandField);
	FocusComponent(commandField);
	commandField->SetBorder(false);
}

/*Console Event Listener for various Key Command Presses*/
void ConsoleView::DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	switch(key)
	{
	case SDLK_ESCAPE:
	case '`':
		if (character != '~')
			c->CloseConsole();
		else
			Window::DoKeyPress(key, character, shift, ctrl, alt);
		break;
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		c->EvaluateCommand(commandField->GetText());
		commandField->SetText("");
		commandField->SetDisplayText("");
		break;
	case SDLK_DOWN:
		c->NextCommand();
		break;
	case SDLK_UP:
		c->PreviousCommand();
		break;
	default:
		Window::DoKeyPress(key, character, shift, ctrl, alt);
		break;
	}
}

void ConsoleView::NotifyPreviousCommandsChanged(ConsoleModel * sender)
{
	for (size_t i = 0; i < commandList.size(); i++)
	{
		RemoveComponent(commandList[i]);
		delete commandList[i];
	}
	commandList.clear();
	std::deque<ConsoleCommand> commands = sender->GetPreviousCommands();
	
	int currentY = Size.Y - 16;
	int numLines = 0;

	if(commands.size())
		for(int i = commands.size()-1; i >= 0; i--)
		{
			/*account for multiline*/
			double totalY = 5 * commands[i].Command.size(); //width of a single char = 5 pixels. Multiply this by total num of characters in the string to get total length of the string in pixels
			numLines = ceil(totalY / (Size.X / 2)); // number of lines = total length of string (in pixels) / pixel width of the window
			currentY -= 16 * numLines;
			/*end account for multiline*/

			if(currentY <= 0) //i.e. we're off screen now (hence done and nothing left to print)
				break;
			ui::Label * tempLabel = new ui::Label(ui::Point(Size.X/2, currentY), ui::Point(Size.X/2, 16), commands[i].ReturnValue); //remove the '/2' to get rid of console feedback display
			tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
			tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
			commandList.push_back(tempLabel);
			AddComponent(tempLabel);
			tempLabel = new ui::Label(ui::Point(0, currentY), ui::Point(Size.X/2, 16), commands[i].Command);
			tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
			tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
			tempLabel->SetMultiline(true);
			commandList.push_back(tempLabel);
			AddComponent(tempLabel);
		}
}

void ConsoleView::NotifyCurrentCommandChanged(ConsoleModel * sender)
{
	commandField->SetText(sender->GetCurrentCommand().Command);
	commandField->SetDisplayText(c->FormatCommand(commandField->GetText()));
}


void ConsoleView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->fillrect(Position.X, Position.Y, Size.X, Size.Y, 0, 0, 0, 110);
	g->draw_line(Position.X, Position.Y+Size.Y-16, Position.X+Size.X, Position.Y+Size.Y-16, 255, 255, 255, 160);

	if(commandField->GetMultilineStatus() == 0){
		g->draw_line(Position.X, Position.Y+Size.Y, Position.X+Size.X, Position.Y+Size.Y, 255, 255, 255, 200);
	}
	else{
		/*Redraw bottom line of console text box to conform to multiline*/
		//double totalY = 4.04 * commandField->GetText().size();
		//int numSpace = 16*(int(totalY) / Size.X);
		int lineWidth = GetPixelWidth(commandField->GetText());
		int numSpace = 12 * (lineWidth / Size.X);
		g->draw_line(Position.X, Position.Y + Size.Y + numSpace, Position.X + Size.X, Position.Y + Size.Y + numSpace, 255, 255, 255, 200);
	}
}

int ConsoleView::GetPixelWidth(std::string str)
{
	int lineWidth = 0;
	for (int i = 0; i < str.size(); i++)
	{
		lineWidth += Graphics::CharWidth(str[i]);
	}
	return lineWidth;
}

ConsoleView::~ConsoleView() {
}

