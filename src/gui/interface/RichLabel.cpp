#include "RichLabel.h"

#include <vector>
#include <exception>

#include "Platform.h"

#include "gui/interface/Point.h"
#include "gui/interface/Component.h"

#include "graphics/Graphics.h"
#include "graphics/FontReader.h"

#include "Colour.h"

using namespace ui;

struct RichTextParseException: public std::exception {
	ByteString message;
public:
	RichTextParseException(String message = String("Parse error")): message(message.ToUtf8()) {}
	const char * what() const throw() override
	{
		return message.c_str();
	}
	~RichTextParseException() throw() {};
};

RichLabel::RichLabel(Point position, Point size, String labelText):
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
		String::value_type * currentData = new String::value_type[textSource.length()+1];
		std::fill(currentData, currentData+textSource.length()+1, 0);

		int finalTextPos = 0;
		String::value_type * finalText = new String::value_type[textSource.length()+1];
		std::fill(finalText, finalText+textSource.length()+1, 0);

		int originalTextPos = 0;
		String::value_type * originalText = new String::value_type[textSource.length()+1];
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
							regionsStack[stackPos].actionData = String(currentData);
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
			displayText = String(finalText);
		}
		catch (const RichTextParseException & e)
		{
			displayText = "\br[Parse exception: " + ByteString(e.what()).FromUtf8() + "]";
			regions.clear();
		}
		delete[] currentData;
		delete[] finalText;
		delete[] originalText;
		delete[] regionsStack;
	}
	TextPosition(displayText);
	displayTextWrapper.Update(displayText, false, 0);
}

void RichLabel::SetText(String text)
{
	textSource = text;
	updateRichText();
}

String RichLabel::GetDisplayText()
{
	return displayText;
}

String RichLabel::GetText()
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
	int cursorPosition = displayTextWrapper.Point2Index(x - textPosition.X, y - textPosition.Y).raw_index;
	for (auto const &region : regions)
	{
		if (region.start <= cursorPosition && region.finish >= cursorPosition)
		{
			switch (region.action)
			{
			case 'a':
				Platform::OpenURI(region.actionData.ToUtf8());
				break;
			}
		}
	}
}
