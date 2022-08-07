#include "Config.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>

#include "common/String.h"
#include "Format.h"
#include "gui/interface/Engine.h"

#include "client/GameSave.h"
#include "simulation/Simulation.h"


void EngineProcess() {}
void ClipboardPush(ByteString) {}
ByteString ClipboardPull() { return ""; }
int GetModifiers() { return 0; }
void SetCursorEnabled(int enabled) {}
unsigned int GetTicks() { return 0; }

static bool ReadFile(std::vector<char> &fileData, ByteString filename)
{
	std::ifstream f(filename, std::ios::binary);
	if (f) f.seekg(0, std::ios::end);
	if (f) fileData.resize(f.tellg());
	if (f) f.seekg(0);
	if (f) f.read(&fileData[0], fileData.size());
	if (!f)
	{
		std::cerr << "ReadFile: " << filename << ": " << strerror(errno) << std::endl;
		return false;
	}
	return true;
}

int main(int argc, char *argv[])
{
	if (!argv[1] || !argv[2]) {
		std::cout << "Usage: " << argv[0] << " <inputFilename> <outputPrefix>" << std::endl;
		return 1;
	}
	auto inputFilename = ByteString(argv[1]);
	auto outputFilename = ByteString(argv[2]) + ".png";

	std::vector<char> fileData;
	if (!ReadFile(fileData, inputFilename))
	{
		return 1;
	}

	GameSave * gameSave = NULL;
	try
	{
		gameSave = new GameSave(fileData);
	}
	catch (ParseException &e)
	{
		//Render the save again later or something? I don't know
		if (ByteString(e.what()).FromUtf8() == "Save from newer version")
			throw e;
	}

	Simulation * sim = new Simulation();
	Renderer * ren = new Renderer(new Graphics(), sim);

	if (gameSave)
	{
		sim->Load(gameSave, true);

		//Render save
		ren->decorations_enable = true;
		ren->blackDecorations = true;

		int frame = 15;
		while(frame)
		{
			frame--;
			ren->render_parts();
			ren->render_fire();
			ren->clearScreen(1.0f);
		}
	}
	else
	{
		int w = Graphics::textwidth("Save file invalid")+16, x = (XRES-w)/2, y = (YRES-24)/2;
		ren->drawrect(x, y, w, 24, 192, 192, 192, 255);
		ren->drawtext(x+8, y+8, "Save file invalid", 192, 192, 240, 255);
	}

	ren->RenderBegin();
	ren->RenderEnd();

	VideoBuffer screenBuffer = ren->DumpFrame();
	screenBuffer.WritePNG(outputFilename);
}
