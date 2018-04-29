#ifndef FONTEDITOR_H
#define FONTEDITOR_H

#include <vector>
#include <array>

#include "font.h"

#include "gui/interface/Window.h"
#include "gui/interface/Textbox.h"

#define MAX_WIDTH 64
class FontEditor: public ui::Window
{
private:
	std::string header;
	std::array<char, 256> fontWidths;
	std::array<std::array<std::array<char, MAX_WIDTH>, FONT_H>, 256> fontPixels;

	std::vector<unsigned char> fontData;
	std::vector<short> fontPtrs;

	std::string beforeFontData;
	std::string afterFontData;
	std::string afterFontPtrs;

	void ReadHeader(std::string header);
	void WriteHeader(std::string header, std::vector<unsigned char> const &fontData, std::vector<short> const &fontPtrs);
	static void PackData(
			std::array<char, 256> const &fontWidths,
			std::array<std::array<std::array<char, MAX_WIDTH>, FONT_H>, 256> const &fontPixels,
			std::vector<unsigned char> &fontData,
			std::vector<short> &fontPtrs);
	static void UnpackData(
			std::array<char, 256> &fontWidths,
			std::array<std::array<std::array<char, MAX_WIDTH>, FONT_H>, 256> &fontPixels,
			std::vector<unsigned char> const &fontData,
			std::vector<short> const &fontPtrs);

	ui::Textbox *currentCharTextbox;
	ui::Button *savedButton;
	ui::Label *outputPreview;

	int currentChar;
	int fgR, fgG, fgB;
	int bgR, bgG, bgB;

	int grid;
	int rulers;

	void UpdateCharNumber();
	void PrevChar();
	void NextChar();
	void ShrinkChar();
	void GrowChar();
	void Render();
	void Save();

public:
	FontEditor(std::string header);

	void OnDraw();
	void OnMouseDown(int x, int y, unsigned button);
	void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
};

#endif
