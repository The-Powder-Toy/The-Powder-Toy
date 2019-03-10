#ifndef FONTEDITOR_H
#define FONTEDITOR_H

#include <vector>
#include <array>
#include <map>

#include "font.h"

#include "gui/interface/Window.h"
#include "gui/interface/Textbox.h"

#define MAX_WIDTH 64
class FontEditor: public ui::Window
{
private:
	ByteString header;
	std::map<String::value_type, unsigned char> fontWidths;
	std::map<String::value_type, std::array<std::array<char, MAX_WIDTH>, FONT_H> > fontPixels;

	std::vector<unsigned char> fontData;
	std::vector<unsigned short> fontPtrs;
	std::vector<std::array<unsigned int, 2> > fontRanges;

	ByteString beforeFontData;
	ByteString afterFontData;
	ByteString afterFontPtrs;
	ByteString afterFontRanges;

	void ReadHeader(ByteString header);
	void WriteHeader(ByteString header, std::vector<unsigned char> const &fontData, std::vector<unsigned short> const &fontPtrs, std::vector<std::array<unsigned int, 2> > const &fontRanges);
	static void PackData(
			std::map<String::value_type, unsigned char> const &fontWidths,
			std::map<String::value_type, std::array<std::array<char, MAX_WIDTH>, FONT_H> > const &fontPixels,
			std::vector<unsigned char> &fontData,
			std::vector<unsigned short> &fontPtrs,
			std::vector<std::array<unsigned int, 2> > &fontRanges);
	static void UnpackData(
			std::map<String::value_type, unsigned char> &fontWidths,
			std::map<String::value_type, std::array<std::array<char, MAX_WIDTH>, FONT_H> > &fontPixels,
			std::vector<unsigned char> const &fontData,
			std::vector<unsigned short> const &fontPtrs,
			std::vector<std::array<unsigned int, 2> > const &fontRanges);

	ui::Textbox *currentCharTextbox;
	ui::Button *savedButton;
	ui::Label *outputPreview;

	String::value_type currentChar;
	int fgR, fgG, fgB;
	int bgR, bgG, bgB;

	int grid;
	int rulers;

	unsigned char clipboardWidth;
	std::array<std::array<char, MAX_WIDTH>, FONT_H> clipboardPixels;

	void UpdateCharNumber();
	void PrevChar();
	void NextChar();
	void ShrinkChar();
	void GrowChar();
	void Render();
	void Save();

public:
	FontEditor(ByteString header);

	void OnDraw() override;
	void OnMouseDown(int x, int y, unsigned button) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
};

#endif
