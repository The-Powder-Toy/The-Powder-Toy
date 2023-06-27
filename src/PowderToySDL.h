#pragma once
#include "common/String.h"
#include "graphics/Pixel.h"
#include "FpsLimit.h"
#include <cstdint>
#include <SDL.h>
#include <variant>

extern int desktopWidth;
extern int desktopHeight;
extern SDL_Window *sdl_window;
extern SDL_Renderer *sdl_renderer;
extern SDL_Texture *sdl_texture;
extern int scale;
extern bool fullscreen;
extern bool altFullscreen;
extern bool forceIntegerScaling;
extern bool resizable;
extern bool momentumScroll;
extern bool showAvatars;
extern uint64_t lastTick;
extern uint64_t lastFpsUpdate;
extern bool showLargeScreenDialog;
extern int mousex;
extern int mousey;
extern int mouseButton;
extern bool mouseDown;
extern bool calculatedInitialMouse;
extern bool hasMouseMoved;

void MainLoop();
void EngineProcess();
void StartTextInput();
void StopTextInput();
void SetTextInputRect(int x, int y, int w, int h);
void ClipboardPush(ByteString text);
ByteString ClipboardPull();
int GetModifiers();
unsigned int GetTicks();
void CalculateMousePosition(int *x, int *y);
void blit(pixel *vid);
void SDLOpen();
void SDLClose();
void SDLSetScreen(int scale_, bool resizable_, bool fullscreen_, bool altFullscreen_, bool forceIntegerScaling_);
void SetFpsLimit(FpsLimit newFpsLimit);
bool RecreateWindow();
void LoadWindowPosition();
void SaveWindowPosition();
void LargeScreenDialog();
void TickClient();
void EventProcess(const SDL_Event &event);
void UpdateFpsLimit();
