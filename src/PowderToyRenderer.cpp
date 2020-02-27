#if defined(RENDERER)

#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>

#include "common/String.h"
#include "Config.h"
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

void readFile(ByteString filename, std::vector<char> & storage)
{
	std::ifstream fileStream;
	fileStream.open(filename.c_str(), std::ios::binary);
	if(fileStream.is_open())
	{
		fileStream.seekg(0, std::ios::end);
		size_t fileSize = fileStream.tellg();
		fileStream.seekg(0);

		unsigned char * tempData = new unsigned char[fileSize];
		fileStream.read((char *)tempData, fileSize);
		fileStream.close();

		std::vector<unsigned char> fileData;
		storage.clear();
		storage.insert(storage.end(), tempData, tempData+fileSize);
		delete[] tempData;
	}
}

void writeFile(ByteString filename, std::vector<char> & fileData)
{
	std::ofstream fileStream;
	fileStream.open(filename.c_str(), std::ios::binary);
	if(fileStream.is_open())
	{
		fileStream.write(&fileData[0], fileData.size());
		fileStream.close();
	}
}

int main(int argc, char *argv[])
{
	ui::Engine * engine;
	ByteString outputPrefix, inputFilename;
	std::vector<char> inputFile;
	ByteString ppmFilename, ptiFilename, ptiSmallFilename, pngFilename, pngSmallFilename;
	std::vector<char> ppmFile, ptiFile, ptiSmallFile, pngFile, pngSmallFile;

	inputFilename = argv[1];
	outputPrefix = argv[2];

	ppmFilename = outputPrefix+".ppm";
	ptiFilename = outputPrefix+".pti";
	ptiSmallFilename = outputPrefix+"-small.pti";
	pngFilename = outputPrefix+".png";
	pngSmallFilename = outputPrefix+"-small.png";

	readFile(inputFilename, inputFile);

	ui::Engine::Ref().g = new Graphics();

	engine = &ui::Engine::Ref();
	engine->Begin(WINDOWW, WINDOWH);

	GameSave * gameSave = NULL;
	try
	{
		gameSave = new GameSave(inputFile);
	}
	catch (ParseException &e)
	{
		//Render the save again later or something? I don't know
		if (ByteString(e.what()).FromUtf8() == "Save from newer version")
			throw e;
	}

	Simulation * sim = new Simulation();
	Renderer * ren = new Renderer(ui::Engine::Ref().g, sim);

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
	//ppmFile = format::VideoBufferToPPM(screenBuffer);
	ptiFile = format::VideoBufferToPTI(screenBuffer);
	pngFile = format::VideoBufferToPNG(screenBuffer);

	screenBuffer.Resize(1.0f/3.0f, true);
	ptiSmallFile = format::VideoBufferToPTI(screenBuffer);
	pngSmallFile = format::VideoBufferToPNG(screenBuffer);



	//writeFile(ppmFilename, ppmFile);
	writeFile(ptiFilename, ptiFile);
	writeFile(ptiSmallFilename, ptiSmallFile);
	writeFile(pngFilename, pngFile);
	writeFile(pngSmallFilename, pngSmallFile);
}

#endif
