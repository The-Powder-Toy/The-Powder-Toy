#if defined(RENDERER)

#include <time.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>

#include "Config.h"
#include "Format.h"
#include "interface/Engine.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

#include "client/GameSave.h"
#include "simulation/Simulation.h"


void EngineProcess() {}
void ClipboardPush(char * text) {}
char * ClipboardPull() {}
int GetModifiers() { return 0; }

void readFile(std::string filename, std::vector<char> & storage)
{
	std::ifstream fileStream;
	fileStream.open(std::string(filename).c_str(), std::ios::binary);
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

void writeFile(std::string filename, std::vector<char> & fileData)
{
	std::ofstream fileStream;
	fileStream.open(std::string(filename).c_str(), std::ios::binary);
	if(fileStream.is_open())
	{
		fileStream.write(&fileData[0], fileData.size());
		fileStream.close();
	}
}

int main(int argc, char *argv[])
{	
	ui::Engine * engine;
	std::string outputPrefix, inputFilename;
	std::vector<char> inputFile;
	std::string ppmFilename, ptiFilename, ptiSmallFilename, pngFilename, pngSmallFilename;
	std::vector<char> ppmFile, ptiFile, ptiSmallFile, pngFile, pngSmallFile;

	inputFilename = std::string(argv[1]);
	outputPrefix = std::string(argv[2]);

	ppmFilename = outputPrefix+".ppm";
	ptiFilename = outputPrefix+".pti";
	ptiSmallFilename = outputPrefix+"-small.pti";
	pngFilename = outputPrefix+".png";
	pngSmallFilename = outputPrefix+"-small.png";

	readFile(inputFilename, inputFile);

	ui::Engine::Ref().g = new Graphics();
	
	engine = &ui::Engine::Ref();
	engine->Begin(XRES+BARSIZE, YRES+MENUSIZE);

	GameSave * gameSave = new GameSave(inputFile);

	Simulation * sim = new Simulation();
	Renderer * ren = new Renderer(ui::Engine::Ref().g, sim);

	sim->Load(gameSave);


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
