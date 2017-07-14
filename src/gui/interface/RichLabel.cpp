#include <vector>
#include <exception>

#include "RichLabel.h"
#include "Platform.h"
#include "gui/interface/Point.h"
#include "gui/interface/Component.h"
#include "graphics/Graphics.h"

using namespace ui;

struct RichTextParseException: public std::exception {
	std::string message;
public:
	RichTextParseException(std::string message_ = "Parse error"): message(message_) {}
	const char * what() const throw()
	{
		return message.c_str();
	}
	~RichTextParseException() throw() {};
};

RichLabel::RichLabel(Point position, Point size, std::string labelText):
	Component(position, size),
	textSource(labelText),
	displayText("")
{
	updateRichText();
}

RichLabel::~RichLabel()
{

}

void RichLabel::updateRichText()
{
	regions.clear();
	displayText = "";

	if(textSource.length())
	{

		enum State { ReadText, ReadData, ReadRegion, ReadDataStart };
		State state = ReadText;

		int currentDataPos = 0;
		char * currentData = new char[textSource.length()+1];
		std::fill(currentData, currentData+textSource.length()+1, 0);

		int finalTextPos = 0;
		char * finalText = new char[textSource.length()+1];
		std::fill(finalText, finalText+textSource.length()+1, 0);

		int originalTextPos = 0;
		char * originalText = new char[textSource.length()+1];
		std::copy(textSource.begin(), textSource.end(), originalText);
		originalText[textSource.length()] = 0;

		int stackPos = -1;
		RichTextRegion * regionsStack = new RichTextRegion[256];

		try
		{
			while(originalText[originalTextPos])
			{
				char current = originalText[originalTextPos];

				if(state == ReadText)
				{
					if(current == '{')
					{
						if(stackPos > 255)
							throw RichTextParseException("Too many nested regions");
						stackPos++;
						regionsStack[stackPos].start = finalTextPos;
						regionsStack[stackPos].finish = finalTextPos;
						state = ReadRegion;
					}
					else if(current == '}')
					{
						if(stackPos >= 0)
						{
							currentData[currentDataPos] = 0;
							regionsStack[stackPos].actionData = std::string(currentData);
							regions.push_back(regionsStack[stackPos]);
							stackPos--;
						}
						else
						{
							throw RichTextParseException("Unexpected '}'");
						}
					}
					else
					{
						finalText[finalTextPos++] = current;
						finalText[finalTextPos] = 0;
						if(stackPos >= 0)
						{
							regionsStack[stackPos].finish = finalTextPos;
						}
					}
				}
				else if(state == ReadData)
				{
					if(current == '|')
					{
						state = ReadText;
					}
					else
					{
						currentData[currentDataPos++] = current;
						currentData[currentDataPos] = 0;
					}
				}
				else if(state == ReadDataStart)
				{
					if(current != ':')
					{
						throw RichTextParseException("Expected ':'");
					}
					state = ReadData;
					currentDataPos = 0;
				}
				else if(state == ReadRegion)
				{
					if(stackPos >= 0)
					{
						regionsStack[stackPos].action = current;
						state = ReadDataStart;
					}
					else
					{
						throw RichTextParseException();
					}
				}

				originalTextPos++;
			}

			if(stackPos != -1)
				throw RichTextParseException("Unclosed region");

			finalText[finalTextPos] = 0;
			displayText = std::string(finalText);
		}
		catch (const RichTextParseException & e)
		{
			displayText = "\br[Parse exception: " + std::string(e.what()) + "]";
			regions.clear();
		}
		delete[] currentData;
		delete[] finalText;
		delete[] originalText;
		delete[] regionsStack;
	}
	TextPosition(displayText);
}

void RichLabel::SetText(std::string text)
{
	textSource = text;
	updateRichText();
}

std::string RichLabel::GetDisplayText()
{
	return displayText;
}

std::string RichLabel::GetText()
{
	return textSource;
}

void RichLabel::Draw(const Point& screenPos)
{
	Graphics * g = GetGraphics();
	ui::Colour textColour = Appearance.TextInactive;
	g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, displayText, textColour.Red, textColour.Green, textColour.Blue, 255);
}

void RichLabel::OnMouseClick(int x, int y, unsigned button)
{
	int cursorPosition = Graphics::CharIndexAtPosition((char*)displayText.c_str(), x-textPosition.X, y-textPosition.Y);
	for(std::vector<RichTextRegion>::iterator iter = regions.begin(), end = regions.end(); iter != end; ++iter)
	{
		if((*iter).start <= cursorPosition && (*iter).finish >= cursorPosition)
		{
			switch((*iter).action)
			{
				case 'a':
					Platform::OpenURI((*iter).actionData);
				break;
			}
		}
	}
}
