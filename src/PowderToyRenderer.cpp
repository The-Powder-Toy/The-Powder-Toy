#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "common/String.h"
#include "common/tpt-rand.h"
#include "Format.h"
#include "gui/interface/Engine.h"
#include "client/GameSave.h"
#include "simulation/Simulation.h"
#include "common/platform/Platform.h"
#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>

int main(int argc, char *argv[])
{
	if (!argv[1] || !argv[2]) {
		std::cout << "Usage: " << argv[0] << " <inputFilename> <outputPrefix>" << std::endl;
		return 1;
	}
	auto inputFilename = ByteString(argv[1]);
	auto outputFilename = ByteString(argv[2]) + ".png";

	std::vector<char> fileData;
	if (!Platform::ReadFile(fileData, inputFilename))
	{
		return 1;
	}

	GameSave * gameSave = NULL;
	try
	{
		gameSave = new GameSave(fileData, false);
	}
	catch (ParseException &e)
	{
		//Render the save again later or something? I don't know
		if (ByteString(e.what()).FromUtf8() == "Save from newer version")
			throw e;
	}

	auto rng = std::make_unique<RNG>();
	Simulation * sim = new Simulation();
	Renderer * ren = new Renderer(sim);

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
			ren->clearScreen();
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

	if (auto data = ren->DumpFrame().ToPNG())
		Platform::WriteFile(*data, outputFilename);
}
