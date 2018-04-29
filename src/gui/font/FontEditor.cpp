#include <stdexcept>
#include <sstream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <iostream>

#include "FontEditor.h"

#include "Config.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Point.h"
#include "gui/interface/Button.h"
#include "gui/interface/Mouse.h"
#include "gui/interface/Keys.h"
#include "graphics/Graphics.h"

#ifdef FONTEDITOR
unsigned char *font_data;
short *font_ptrs;

void FontEditor::ReadHeader(std::string header)
{
	std::fstream file;
	file.open(header, std::ios_base::in);
	if(!file)
		throw std::runtime_error("Could not open " + header);
	file >> std::skipws;

	std::string word;

	while(word != "font_data[]")
		file >> word;
	file >> word >> word;

	size_t startFontData = file.tellg();

	fontData.clear();
	do
	{
		unsigned int value;
		file >> std::hex >> value;
		if(!file.fail())
		{
			fontData.push_back(value);
			file >> word;
		}
	}
	while(!file.fail());
	file.clear();
	
	size_t endFontData = file.tellg();
	
	while(word != "font_ptrs[]")
		file >> word;
	file >> word >> word;
	
	size_t startFontPtrs = file.tellg();
	
	fontPtrs.clear();
	do
	{
		unsigned int value;
		file >> std::hex >> value;
		if(!file.fail())
		{
			fontPtrs.push_back(value);
			file >> word;
		}
	}
	while(!file.fail());
	file.clear();
	
	size_t endFontPtrs = file.tellg();

	do
	{
		file >> word;
	}
	while(!file.fail());
	file.clear();
	size_t eof = file.tellg();

	file.seekg(0);
	beforeFontData = std::string(startFontData, 0);
	file.read(&beforeFontData[0], startFontData);

	file.seekg(endFontData);
	afterFontData = std::string(startFontPtrs - endFontData, 0);
	file.read(&afterFontData[0], startFontPtrs - endFontData);

	file.seekg(endFontData);
	afterFontData = std::string(startFontPtrs - endFontData, 0);
	file.read(&afterFontData[0], startFontPtrs - endFontData);

	file.seekg(endFontPtrs);
	afterFontPtrs = std::string(eof - endFontPtrs, 0);
	file.read(&afterFontPtrs[0], eof - endFontPtrs);
	file.close();
}

void FontEditor::WriteHeader(std::string header, std::vector<unsigned char> const &fontData, std::vector<short> const &fontPtrs)
{
	std::fstream file;
	file.open(header, std::ios_base::out | std::ios_base::trunc);
	if(!file)
		throw std::runtime_error("Could not open " + header);

	file << std::setfill('0') << std::hex << std::uppercase;
	file << beforeFontData << std::endl;
	for(int ch = 0; ch < 256; ch++)
	{
		file << "    " << "0x" << std::setw(2) << (unsigned int)fontData[fontPtrs[ch]] << ",  ";
		for(int i = fontPtrs[ch] + 1; i < (int)(ch == (int)fontPtrs.size() - 1 ? fontData.size() : fontPtrs[ch + 1]); i++)
			file << " " << "0x" << std::setw(2) << (unsigned int)fontData[i] << ",";
		file << std::endl;
	}
	file << afterFontData;
	for(int ch = 0; ch < 256; ch++)
	{
		if(!(ch & 7))
			file << std::endl << "    ";
		else
			file << " ";
		file << "0x" << std::setw(4) << (unsigned int)fontPtrs[ch] << ",";
	}
	file << std::endl << afterFontPtrs;
	file.close();
}

void FontEditor::UnpackData(
		std::array<char, 256> &fontWidths,
		std::array<std::array<std::array<char, MAX_WIDTH>, FONT_H>, 256> &fontPixels,
		std::vector<unsigned char> const &fontData,
		std::vector<short> const &fontPtrs)
{
	for(int ch = 0; ch < 256; ch++)
	{
		unsigned char const *data = &fontData[fontPtrs[ch]];
		int bits = 0;
		int pixels = 0;
		int width = fontWidths[ch] = *(data++);
		for(int j = 0; j < FONT_H; j++)
			for(int i = 0; i < width; i++)
			{
				if(!bits)
				{
					pixels = *(data++);
					bits = 8;
				}
				fontPixels[ch][j][i] = pixels & 3;
				pixels >>= 2;
				bits -= 2;
			}
	}
}

void FontEditor::PackData(
		std::array<char, 256> const &fontWidths,
		std::array<std::array<std::array<char, MAX_WIDTH>, FONT_H>, 256> const &fontPixels,
		std::vector<unsigned char> &fontData,
		std::vector<short> &fontPtrs)
{
	fontPtrs.clear();
	fontData.clear();
	for(int ch = 0; ch < 256; ch++)
	{
		fontPtrs.push_back(fontData.size());
		fontData.push_back(fontWidths[ch]);

		int bits = 0;
		int pixels = 0;
		for(int j = 0; j < FONT_H; j++)
			for(int i = 0; i < fontWidths[ch]; i++)
			{
				if(bits == 8)
				{
					fontData.push_back(pixels);
					bits = 0;
					pixels = 0;
				}
				pixels >>= 2;
				pixels |= fontPixels[ch][j][i] << 6;
				bits += 2;
			}
		if(bits)
			fontData.push_back(pixels);
	}
}

#define FONT_SCALE 16
FontEditor::FontEditor(std::string _header):
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, WINDOWH)),
	header(_header),
	currentChar(0x80),
	fgR(255), fgG(255), fgB(255), bgR(0), bgG(0), bgB(0),
	grid(1),
	rulers(1)
{
	ReadHeader(header);
	UnpackData(fontWidths, fontPixels, fontData, fontPtrs);
	font_data = fontData.data();
	font_ptrs = fontPtrs.data();
	
	int baseline = 8 + FONT_H * FONT_SCALE + 4 + FONT_H + 4 + 1;
	int currentX = 1;

	class PrevCharAction : public ui::ButtonAction
	{
		FontEditor *v;
	public:
		PrevCharAction(FontEditor *_v): v(_v) {}
		void ActionCallback(ui::Button *)
		{
			v->PrevChar();
		}
	};
	ui::Button *prev = new ui::Button(ui::Point(currentX, baseline), ui::Point(17, 17), "\x96");
	currentX += 18;
	prev->SetActionCallback(new PrevCharAction(this));
	AddComponent(prev);

	class CharNumberAction : public ui::TextboxAction
	{
		FontEditor *v;
	public:
		CharNumberAction(FontEditor *_v): v(_v) {}
		void TextChangedCallback(ui::Textbox *)
		{
			unsigned int number;
			std::stringstream ss(v->currentCharTextbox->GetText());
			ss >> std::hex >> number;
			if(number < 256)
				v->currentChar = number;
		}
	};
	currentCharTextbox = new ui::Textbox(ui::Point(currentX, baseline), ui::Point(31, 17));
	currentX += 32;
	currentCharTextbox->SetActionCallback(new CharNumberAction(this));
	UpdateCharNumber();
	AddComponent(currentCharTextbox);

	class NextCharAction : public ui::ButtonAction
	{
		FontEditor *v;
	public:
		NextCharAction(FontEditor *_v): v(_v) {}
		void ActionCallback(ui::Button *)
		{
			v->NextChar();
		}
	};
	ui::Button *next = new ui::Button(ui::Point(currentX, baseline), ui::Point(17, 17), "\x95");
	currentX += 18;
	next->SetActionCallback(new NextCharAction(this));
	AddComponent(next);

	class ShrinkCharAction : public ui::ButtonAction
	{
		FontEditor *v;
	public:
		ShrinkCharAction(FontEditor *_v): v(_v) {}
		void ActionCallback(ui::Button *)
		{
			v->ShrinkChar();
		}
	};
	ui::Button *shrink = new ui::Button(ui::Point(currentX, baseline), ui::Point(17, 17), "><");
	currentX += 18;
	shrink->SetActionCallback(new ShrinkCharAction(this));
	AddComponent(shrink);

	class GrowCharAction : public ui::ButtonAction
	{
		FontEditor *v;
	public:
		GrowCharAction(FontEditor *_v): v(_v) {}
		void ActionCallback(ui::Button *)
		{
			v->GrowChar();
		}
	};
	ui::Button *grow = new ui::Button(ui::Point(currentX, baseline), ui::Point(17, 17), "<>");
	currentX += 18;
	grow->SetActionCallback(new GrowCharAction(this));
	AddComponent(grow);

	
	class ToggleAction : public ui::ButtonAction
	{
		int &toggle;
	public:
		ToggleAction(int &_toggle): toggle(_toggle) {}
		void ActionCallback(ui::Button *button)
		{
			toggle = button->GetToggleState();
		}
	};
	ui::Button *showGrid = new ui::Button(ui::Point(currentX, baseline), ui::Point(32, 17), "Grid");
	currentX += 33;
	showGrid->SetTogglable(true);
	showGrid->SetToggleState(grid);
	showGrid->SetActionCallback(new ToggleAction(grid));
	AddComponent(showGrid);
	
	ui::Button *showRulers = new ui::Button(ui::Point(currentX, baseline), ui::Point(32, 17), "Rulers");
	currentX += 33;
	showRulers->SetTogglable(true);
	showRulers->SetToggleState(grid);
	showRulers->SetActionCallback(new ToggleAction(rulers));
	AddComponent(showRulers);

	baseline += 18;
	currentX = 1;
	
	class ColorComponentAction : public ui::TextboxAction
	{
		int &color;
	public:
		ColorComponentAction(int &_color): color(_color) {}
		void TextChangedCallback(ui::Textbox *box)
		{
			std::stringstream ss(box->GetText());
			ss >> color;
		}
	};
	int *refs[6] = {&fgR, &fgG, &fgB, &bgR, &bgG, &bgB};
	for(int i = 0; i < 6; i++)
	{
		std::stringstream ss;
		ss << *refs[i];
		ui::Textbox *colorComponent = new ui::Textbox(ui::Point(currentX, baseline), ui::Point(27, 17), ss.str());
		currentX += 28;
		colorComponent->SetActionCallback(new ColorComponentAction(*refs[i]));
		AddComponent(colorComponent);
	}

	baseline += 18;
	currentX = 1;
	
	class RenderAction : public ui::ButtonAction
	{
		FontEditor *v;
	public:
		RenderAction(FontEditor *_v): v(_v) {}
		void ActionCallback(ui::Button *)
		{
			v->Render();
		}
	};
	ui::Button *render = new ui::Button(ui::Point(currentX, baseline), ui::Point(50, 17), "Render");
	currentX += 51;
	render->SetActionCallback(new RenderAction(this));
	AddComponent(render);
	
	class SaveAction : public ui::ButtonAction
	{
		FontEditor *v;
	public:
		SaveAction(FontEditor *_v): v(_v) {}
		void ActionCallback(ui::Button *)
		{
			v->Save();
		}
	};
	savedButton = new ui::Button(ui::Point(currentX, baseline), ui::Point(50, 17), "Save");
	currentX += 51;
	savedButton->SetTogglable(true);
	savedButton->SetToggleState(true);
	savedButton->SetActionCallback(new SaveAction(this));
	AddComponent(savedButton);

	baseline += 18;
	
	outputPreview = new ui::Label(ui::Point(0, baseline + (Size.Y - baseline) * 3 / 5), ui::Point(Size.X, (Size.Y - baseline) * 2 / 5), "");
	outputPreview->SetMultiline(true);
	outputPreview->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	outputPreview->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(outputPreview);
	
	class PreviewAction : public ui::TextboxAction
	{
		FontEditor *v;
	public:
		PreviewAction(FontEditor *_v): v(_v) {}
		void TextChangedCallback(ui::Textbox *box)
		{
			std::stringstream ss(box->GetText());
			std::string text;
			while(!ss.eof())
			{
				if(ss.peek() == '\n')
				{
					text.push_back('\n');
					ss.get();
				}
				unsigned int ch;
				ss >> std::hex >> ch;
				if(ss.fail())
				{
					ss.clear();
					char ch = ss.get();
					if(!ss.eof())
						text.push_back(ch);
					continue;
				}
				text.push_back((char)ch);
			}
			v->outputPreview->SetText(text);
		}
	};
	ui::Textbox *inputPreview = new ui::Textbox(ui::Point(0, baseline), ui::Point(Size.X, (Size.Y - baseline) * 3 / 5));
	inputPreview->SetMultiline(true);
	inputPreview->SetInputType(ui::Textbox::Multiline);
	inputPreview->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	inputPreview->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	inputPreview->SetActionCallback(new PreviewAction(this));

	std::stringstream input;
	for(unsigned int ch = 0x20; ch <= 0xFF; ch++)
	{
		if(!(ch & 0x3F))
			input << "20 ";
		input << std::hex << std::setw(2) << ch << " ";
	}
	inputPreview->SetText(input.str());
	PreviewAction(this).TextChangedCallback(inputPreview);
	AddComponent(inputPreview);
}

void FontEditor::OnDraw()
{
	Graphics *g = GetGraphics();
	
	int areaWidth = 8 + fontWidths[currentChar] * FONT_SCALE + 8;
	g->fillrect(0, 0, areaWidth, 8 + FONT_H * FONT_SCALE + 4 + FONT_H + 4, bgR, bgG, bgB, 255);
	for(int j = 0; j < FONT_H; j++)
		for(int i = 0; i < fontWidths[currentChar]; i++)
			g->fillrect(8 + i * FONT_SCALE, 8 + j * FONT_SCALE, FONT_SCALE - grid, FONT_SCALE - grid, fgR, fgG, fgB, fontPixels[currentChar][j][i] * 255 / 3);

	for(int j = 0; j < FONT_H; j++)
		for(int i = 0; i < fontWidths[currentChar]; i++)
			g->blendpixel(8 + i, 8 + FONT_H * FONT_SCALE + 4 + j, fgR, fgG, fgB, fontPixels[currentChar][j][i] * 255 / 3);


	if(rulers)
	{
		g->draw_line(0, 7 + 0 * FONT_SCALE , areaWidth - 1, 7 + 0 * FONT_SCALE, 128, 128, 128, 255);
		g->draw_line(0, 7 + 2 * FONT_SCALE , areaWidth - 1, 7 + 2 * FONT_SCALE, 128, 128, 128, 255);
		g->draw_line(0, 7 + 4 * FONT_SCALE , areaWidth - 1, 7 + 4 * FONT_SCALE, 128, 128, 128, 255);
		g->draw_line(0, 7 + 9 * FONT_SCALE , areaWidth - 1, 7 + 9 * FONT_SCALE, 128, 128, 128, 255);
		g->draw_line(0, 7 + 12 * FONT_SCALE , areaWidth - 1, 7 + 12 * FONT_SCALE, 128, 128, 128, 255);

		g->draw_line(7, 8, 7, 7 + FONT_H * FONT_SCALE, 128, 128, 128, 255);
		g->draw_line(7 + fontWidths[currentChar] * FONT_SCALE, 8, 7 + fontWidths[currentChar] * FONT_SCALE, 7 + FONT_H * FONT_SCALE, 128, 128, 128, 255);
	}
}

void FontEditor::OnMouseDown(int x, int y, unsigned button)
{
	x = (x - 8) / FONT_SCALE;
	y = (y - 8) / FONT_SCALE;
	if(x >= 0 && y >= 0 && x < fontWidths[currentChar] && y < FONT_H)
	{
		if(button == SDL_BUTTON_LEFT)
			fontPixels[currentChar][y][x] = (fontPixels[currentChar][y][x] + 1) % 4;
		else
			fontPixels[currentChar][y][x] = (fontPixels[currentChar][y][x] + 3) % 4;
		savedButton->SetToggleState(false);
	}
}

void FontEditor::OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(IsFocused(NULL))
	{
		switch(key)
		{
		case SDLK_LEFT:
			PrevChar(); break;
		case SDLK_RIGHT:
			PrevChar(); break;
		case SDLK_ESCAPE:
		case 'q':
			if(savedButton->GetToggleState())
				ui::Engine::Ref().Exit();
			else
				ui::Engine::Ref().ConfirmExit();
			break;
		}
	}
}

void FontEditor::UpdateCharNumber()
{
	std::stringstream ss;
	ss << std::hex << currentChar;
	currentCharTextbox->SetText(ss.str());
}

void FontEditor::PrevChar()
{
	if(currentChar > 0)
		currentChar--;
	UpdateCharNumber();
}

void FontEditor::NextChar()
{
	if(currentChar < 255)
		currentChar++;
	UpdateCharNumber();
}

void FontEditor::ShrinkChar()
{
	if(fontWidths[currentChar] > 0)
		fontWidths[currentChar]--;
	savedButton->SetToggleState(false);
}

void FontEditor::GrowChar()
{
	if(fontWidths[currentChar] < MAX_WIDTH - 1)
		fontWidths[currentChar]++;
	savedButton->SetToggleState(false);
}

void FontEditor::Render()
{
	PackData(fontWidths, fontPixels, fontData, fontPtrs);
	font_data = fontData.data();
	font_ptrs = fontPtrs.data();
}

void FontEditor::Save()
{
	std::vector<unsigned char> tmpFontData;
	std::vector<short> tmpFontPtrs;
	PackData(fontWidths, fontPixels, tmpFontData, tmpFontPtrs);
	WriteHeader(header, tmpFontData, tmpFontPtrs);
	savedButton->SetToggleState(true);
}
#endif
