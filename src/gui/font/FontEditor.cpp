#include <stdexcept>
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
unsigned short *font_ptrs;
unsigned int (*font_ranges)[2];

void FontEditor::ReadHeader(ByteString header)
{
	std::fstream file;
	file.open(header, std::ios_base::in);
	if(!file)
		throw std::runtime_error("Could not open " + header);
	file >> std::skipws;

	ByteString word;

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

	while(word != "font_ranges[][2]")
		file >> word;
	file >> word >> word;

	size_t startFontRanges = file.tellg();

	fontRanges.clear();
	while(true)
	{
		unsigned int value1, value2;
		file >> word >> std::hex >> value1 >> word >> std::hex >> value2 >> word;
		if(file.fail())
			break;
		fontRanges.push_back({value1, value2});
		if(!value2)
			break;
	}
	file.clear();

	size_t endFontRanges = file.tellg();

	do
	{
		file >> word;
	}
	while(!file.fail());
	file.clear();
	size_t eof = file.tellg();

	file.seekg(0);
	beforeFontData = ByteString(startFontData, 0);
	file.read(&beforeFontData[0], startFontData);

	file.seekg(endFontData);
	afterFontData = ByteString(startFontPtrs - endFontData, 0);
	file.read(&afterFontData[0], startFontPtrs - endFontData);

	file.seekg(endFontPtrs);
	afterFontPtrs = ByteString(startFontRanges - endFontPtrs, 0);
	file.read(&afterFontPtrs[0], startFontRanges - endFontPtrs);

	file.seekg(endFontRanges);
	afterFontRanges = ByteString(eof - endFontRanges, 0);
	file.read(&afterFontRanges[0], eof - endFontRanges);
	file.close();
}

void FontEditor::WriteHeader(ByteString header, std::vector<unsigned char> const &fontData, std::vector<unsigned short> const &fontPtrs, std::vector<std::array<unsigned int, 2> > const &fontRanges)
{
	std::fstream file;
	file.open(header, std::ios_base::out | std::ios_base::trunc);
	if(!file)
		throw std::runtime_error("Could not open " + header);

	file << std::setfill('0') << std::hex << std::uppercase;
	file << beforeFontData << std::endl;

	size_t pos = 0;
	size_t ptrpos = 0;
	while(pos < fontData.size())
	{
		file << "    " << "0x" << std::setw(2) << (unsigned int)fontData[pos] << ",  ";
		for(pos++; pos < fontData.size() && (ptrpos == fontPtrs.size() - 1 || pos < (size_t)fontPtrs[ptrpos + 1]); pos++)
			file << " " << "0x" << std::setw(2) << (unsigned int)fontData[pos] << ",";
		file << std::endl;
		ptrpos++;
	}
	file << afterFontData;

	pos = 0;
	for(size_t i = 0; pos < fontPtrs.size() && fontRanges[i][1]; i++)
	{
		bool first = true;
		for(String::value_type ch = fontRanges[i][0]; ch <= fontRanges[i][1]; ch++)
		{
			if(!(ch & 0x7) || first)
				file << std::endl << "    ";
			else
				file << " ";
			first = false;
			file << "0x" << std::setw(4) << (unsigned int)fontPtrs[pos++] << ",";
		}
		file << std::endl;
	}
	file << afterFontPtrs << std::endl;
	for(size_t i = 0; i < fontRanges.size() - 1; i++)
		file << "    { 0x" << std::setw(6) << (unsigned int)fontRanges[i][0] << ", 0x" << std::setw(6) << (unsigned int)fontRanges[i][1] << " }," << std::endl;
	file << "    { 0, 0 },";
	file << afterFontRanges;
	file.close();
}

void FontEditor::UnpackData(
		std::map<String::value_type, unsigned char> &fontWidths,
		std::map<String::value_type, std::array<std::array<char, MAX_WIDTH>, FONT_H> > &fontPixels,
		std::vector<unsigned char> const &fontData,
		std::vector<unsigned short> const &fontPtrs,
		std::vector<std::array<unsigned int, 2> > const &fontRanges)
{
	fontWidths.clear();
	fontPixels.clear();
	size_t pos = 0;
	for(size_t range = 0; fontRanges[range][1]; range++)
		for(String::value_type ch = fontRanges[range][0]; ch <= fontRanges[range][1]; ch++)
		{
			unsigned char const *pointer = &fontData[fontPtrs[pos]];
			int width = fontWidths[ch] = *(pointer++);
			int pixels = 0;
			int data = 0;
			for(int j = 0; j < FONT_H; j++)
				for(int i = 0; i < width; i++)
				{
					if(!pixels)
					{
						data = *(pointer++);
						pixels = 4;
					}
					fontPixels[ch][j][i] = data & 3;
					data >>= 2;
					pixels--;
				}
			pos++;
		}
}

void FontEditor::PackData(
		std::map<String::value_type, unsigned char> const &fontWidths,
		std::map<String::value_type, std::array<std::array<char, MAX_WIDTH>, FONT_H> > const &fontPixels,
		std::vector<unsigned char> &fontData,
		std::vector<unsigned short> &fontPtrs,
		std::vector<std::array<unsigned int, 2> > &fontRanges)
{
	fontData.clear();
	fontPtrs.clear();
	fontRanges.clear();
	bool first = true;
	String::value_type rangeStart;
	String::value_type prev;
	for(std::map<String::value_type, unsigned char>::const_iterator it = fontWidths.begin(); it != fontWidths.end(); it++)
	{
		String::value_type ch = it->first;
		if(first)
		{
			rangeStart = ch;
			first = false;
		}
		else
			if(ch != prev + 1)
			{
				fontRanges.push_back({rangeStart, prev});
				rangeStart = ch;
			}

		fontPtrs.push_back(fontData.size());
		fontData.push_back(it->second);

		int pixels = 0;
		int data = 0;
		for(int j = 0; j < FONT_H; j++)
			for(int i = 0; i < it->second; i++)
			{
				if(pixels == 4)
				{
					fontData.push_back(data);
					pixels = 0;
					data = 0;
				}
				data >>= 2;
				data |= fontPixels.at(ch)[j][i] << 6;
				pixels++;
			}
		if(pixels)
			fontData.push_back(data);

		prev = ch;
	}
	fontRanges.push_back({rangeStart, prev});
	fontRanges.push_back({0, 0});
}

#define FONT_SCALE 16
FontEditor::FontEditor(ByteString _header):
	ui::Window(ui::Point(0, 0), ui::Point(WINDOWW, WINDOWH)),
	header(_header),
	currentChar(0x80),
	fgR(255), fgG(255), fgB(255), bgR(0), bgG(0), bgB(0),
	grid(1),
	rulers(1)
{
	ReadHeader(header);
	UnpackData(fontWidths, fontPixels, fontData, fontPtrs, fontRanges);
	font_data = fontData.data();
	font_ptrs = fontPtrs.data();
	font_ranges = (unsigned int (*)[2])fontRanges.data();
	
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
	ui::Button *prev = new ui::Button(ui::Point(currentX, baseline), ui::Point(17, 17), 0xE016);
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
			unsigned int number = v->currentCharTextbox->GetText().ToNumber<unsigned int>(Format::Hex(), true);
			if(number <= 0x10FFFF)
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
	ui::Button *next = new ui::Button(ui::Point(currentX, baseline), ui::Point(17, 17), 0xE015);
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

	class AddCharAction : public ui::ButtonAction
	{
		FontEditor *v;
	public:
		AddCharAction(FontEditor *_v): v(_v) {}
		void ActionCallback(ui::Button *)
		{
			if(v->fontWidths.find(v->currentChar) == v->fontWidths.end())
			{
				v->savedButton->SetToggleState(false);
				v->fontWidths[v->currentChar] = 5;
				v->fontPixels[v->currentChar];
			}
		}
	};
	ui::Button *add = new ui::Button(ui::Point(currentX, baseline), ui::Point(36, 17), "Add");
	currentX += 37;
	add->SetActionCallback(new AddCharAction(this));
	AddComponent(add);

	class RemoveCharAction : public ui::ButtonAction
	{
		FontEditor *v;
	public:
		RemoveCharAction(FontEditor *_v): v(_v) {}
		void ActionCallback(ui::Button *)
		{
			if(v->fontWidths.find(v->currentChar) != v->fontWidths.end())
			{
				v->savedButton->SetToggleState(false);
				v->fontWidths.erase(v->currentChar);
				v->fontPixels.erase(v->currentChar);
			}
		}
	};
	ui::Button *remove = new ui::Button(ui::Point(currentX, baseline), ui::Point(36, 17), "Remove");
	currentX += 37;
	remove->SetActionCallback(new RemoveCharAction(this));
	AddComponent(remove);
	
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
			color = box->GetText().ToNumber<int>(true);
		}
	};
	int *refs[6] = {&fgR, &fgG, &fgB, &bgR, &bgG, &bgB};
	for(int i = 0; i < 6; i++)
	{
		ui::Textbox *colorComponent = new ui::Textbox(ui::Point(currentX, baseline), ui::Point(27, 17), String::Build(*refs[i]));
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
			String str = box->GetText();
			size_t at = 0;
			StringBuilder text;
			while(at < str.size())
			{
				unsigned int ch;
				if(str[at] != ' ')
					if(String::Split split = str.SplitNumber(ch, Format::Hex(), at))
					{
						text << String::value_type(ch);
						at = split.PositionAfter();
					}
					else
					{
						text << str[at++];
					}
				else
					at++;
			}
			v->outputPreview->SetText(text.Build());
		}
	};
	ui::Textbox *inputPreview = new ui::Textbox(ui::Point(0, baseline), ui::Point(Size.X, (Size.Y - baseline) * 3 / 5));
	inputPreview->SetMultiline(true);
	inputPreview->SetInputType(ui::Textbox::Multiline);
	inputPreview->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	inputPreview->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	inputPreview->SetActionCallback(new PreviewAction(this));

	StringBuilder input;
	input << Format::Hex() << Format::Width(2);
	for(unsigned int ch = 0x20; ch <= 0xFF; ch++)
	{
		if(!(ch & 0x3F))
			input << 0x20 << " ";
		input << ch << " ";
	}
	inputPreview->SetText(input.Build());
	PreviewAction(this).TextChangedCallback(inputPreview);
	AddComponent(inputPreview);
}

void FontEditor::OnDraw()
{
	Graphics *g = GetGraphics();
	
	if(fontWidths.find(currentChar) != fontWidths.end())
	{
		int width = fontWidths[currentChar];
		std::array<std::array<char, MAX_WIDTH>, FONT_H> const &pixels = fontPixels[currentChar];

		int areaWidth = 8 + width * FONT_SCALE + 8;
		g->fillrect(0, 0, areaWidth, 8 + FONT_H * FONT_SCALE + 4 + FONT_H + 4, bgR, bgG, bgB, 255);
		for(int j = 0; j < FONT_H; j++)
			for(int i = 0; i < width; i++)
				g->fillrect(8 + i * FONT_SCALE, 8 + j * FONT_SCALE, FONT_SCALE - grid, FONT_SCALE - grid, fgR, fgG, fgB, pixels[j][i] * 255 / 3);

		for(int j = 0; j < FONT_H; j++)
			for(int i = 0; i < width; i++)
				g->blendpixel(8 + i, 8 + FONT_H * FONT_SCALE + 4 + j, fgR, fgG, fgB, pixels[j][i] * 255 / 3);


		if(rulers)
		{
			g->draw_line(0, 7 + 0 * FONT_SCALE , areaWidth - 1, 7 + 0 * FONT_SCALE, 128, 128, 128, 255);
			g->draw_line(0, 7 + 2 * FONT_SCALE , areaWidth - 1, 7 + 2 * FONT_SCALE, 128, 128, 128, 255);
			g->draw_line(0, 7 + 4 * FONT_SCALE , areaWidth - 1, 7 + 4 * FONT_SCALE, 128, 128, 128, 255);
			g->draw_line(0, 7 + 9 * FONT_SCALE , areaWidth - 1, 7 + 9 * FONT_SCALE, 128, 128, 128, 255);
			g->draw_line(0, 7 + 12 * FONT_SCALE , areaWidth - 1, 7 + 12 * FONT_SCALE, 128, 128, 128, 255);

			g->draw_line(7, 8, 7, 7 + FONT_H * FONT_SCALE, 128, 128, 128, 255);
			g->draw_line(7 + width * FONT_SCALE, 8, 7 + width * FONT_SCALE, 7 + FONT_H * FONT_SCALE, 128, 128, 128, 255);
		}
	}
	else
	{
		g->drawtext(8, 8, "No character", 255, 0, 0, 255);
	}
}

void FontEditor::OnMouseDown(int x, int y, unsigned button)
{
	if(fontWidths.find(currentChar) != fontWidths.end())
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
		case 'c':
			clipboardWidth = fontWidths[currentChar];
			clipboardPixels = fontPixels[currentChar];
			break;
		case 'v':
			fontWidths[currentChar] = clipboardWidth;
			fontPixels[currentChar] = clipboardPixels;
			break;
		}
	}
}

void FontEditor::UpdateCharNumber()
{
	currentCharTextbox->SetText(String::Build(Format::Hex((unsigned int)currentChar)));
}

void FontEditor::PrevChar()
{
	if(currentChar > 0)
		currentChar--;
	UpdateCharNumber();
}

void FontEditor::NextChar()
{
	if(currentChar <= 0x10FFFF)
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
	PackData(fontWidths, fontPixels, fontData, fontPtrs, fontRanges);
	font_data = fontData.data();
	font_ptrs = fontPtrs.data();
	font_ranges = (unsigned int (*)[2])fontRanges.data();
}

void FontEditor::Save()
{
	std::vector<unsigned char> tmpFontData;
	std::vector<unsigned short> tmpFontPtrs;
	std::vector<std::array<unsigned int, 2> > tmpFontRanges;
	PackData(fontWidths, fontPixels, tmpFontData, tmpFontPtrs, tmpFontRanges);
	WriteHeader(header, tmpFontData, tmpFontPtrs, tmpFontRanges);
	savedButton->SetToggleState(true);
}
#endif
