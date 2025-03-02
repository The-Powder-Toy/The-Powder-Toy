#pragma once
#include "common/String.h"
#include "graphics/Pixel.h"
#include "gui/WindowFrameOps.h"
#include "FpsLimit.h"
#include <cstdint>
#include <SDL.h>
#include <variant>
#include <optional>

extern int desktopWidth;
extern int desktopHeight;
extern SDL_Window *sdl_window;
extern SDL_Renderer *sdl_renderer;
extern SDL_Texture *sdl_texture;
extern bool momentumScroll;
extern bool showAvatars;
extern bool showLargeScreenDialog;
extern int mousex;
extern int mousey;
extern int mouseButton;
extern bool mouseDown;
extern bool calculatedInitialMouse;
extern bool hasMouseMoved;

void MainLoop();
std::optional<uint64_t> EngineProcess();
void StartTextInput();
void StopTextInput();
void SetTextInputRect(int x, int y, int w, int h);
void ClipboardPush(ByteString text);
ByteString ClipboardPull();
int GetModifiers();
unsigned int GetTicks();
uint64_t GetNowNs();
void blit(pixel *vid);
void SDLOpen();
void SDLClose();
void SDLSetScreen();
void LoadWindowPosition();
void SaveWindowPosition();
void LargeScreenDialog();
void TickClient();
void ApplyFpsLimit();
