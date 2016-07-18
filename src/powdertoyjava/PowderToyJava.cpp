#if defined(USE_JNI)

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include "Config.h"
#include "PowderToyJava.h"
#include "graphics/Graphics.h"

#include "game/GameController.h"

using namespace std;

GameController * gameController;
ui::Engine * engine;

int elapsedTime = 0, currentTime = 0, lastTime = 0, currentFrame = 0;
float fps = 0, delta = 1.0f;

JNIEXPORT void JNICALL Java_PowderToy_initialise(JNIEnv * env, jobject canvas)
{	
	ui::Engine::Ref().g = new Graphics();
	
	engine = &ui::Engine::Ref();
	engine->Begin(WINDOWW, WINDOWH);
	
	gameController = new GameController();
	engine->ShowWindow(gameController->GetView());
	engine->SetFps(fps);
}

JNIEXPORT void JNICALL Java_PowderToy_tick(JNIEnv * env, jobject canvas)
{
	engine->Tick();
}

JNIEXPORT void JNICALL Java_PowderToy_draw(JNIEnv * env, jobject canvas)
{
	engine->Draw();
	engine->g->Finalise();
}

JNIEXPORT void JNICALL Java_PowderToy_finish(JNIEnv * env, jobject canvas)
{
	ui::Engine::Ref().CloseWindow();
	delete gameController;
	delete ui::Engine::Ref().g;
}

JNIEXPORT jint JNICALL Java_PowderToy_getWidth(JNIEnv * env, jobject canvas)
{
	return WINDOWW;
}

JNIEXPORT jint JNICALL Java_PowderToy_getHeight(JNIEnv * env, jobject canvas)
{
	return WINDOWH;
}

JNIEXPORT void JNICALL Java_PowderToy_mousePressed(JNIEnv * env, jobject canvas, jint mouseX, jint mouseY, jint mouseButton)
{
	engine->onMouseClick(mouseX, mouseY, mouseButton);
}

JNIEXPORT void JNICALL Java_PowderToy_mouseReleased(JNIEnv * env, jobject canvas, jint mouseX, jint mouseY, jint mouseButton)
{
	engine->onMouseUnclick(mouseX, mouseY, mouseButton);
}

JNIEXPORT void JNICALL Java_PowderToy_mouseMoved(JNIEnv * env, jobject canvas, jint mouseX, jint mouseY)
{
	engine->onMouseMove(mouseX, mouseY);
}

#endif
