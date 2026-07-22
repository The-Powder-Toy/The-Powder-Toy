#pragma once

#include <SDL.h>

struct Win32HDRRenderer;

// Creates a Windows D3D11 FP16/scRGB presenter for an existing SDL window.
// Returns nullptr when the required DXGI path is unavailable; the caller should
// then retain the normal SDL_Renderer fallback.
Win32HDRRenderer *Win32HDRCreate(
	SDL_Window *window,
	int logicalWidth,
	int logicalHeight,
	int simulationWidth,
	int simulationHeight
);

void Win32HDRDestroy(Win32HDRRenderer *renderer);

bool Win32HDRPresent(
	Win32HDRRenderer *renderer,
	const void *pixels,
	int sourcePitch,
	bool linearFiltering,
	bool integerScaling
);

void Win32HDRInvalidateOutput(Win32HDRRenderer *renderer);

void Win32HDRWindowToLogical(
	Win32HDRRenderer *renderer,
	int windowX,
	int windowY,
	int *logicalX,
	int *logicalY,
	bool integerScaling
);

void Win32HDRLogicalToWindow(
	Win32HDRRenderer *renderer,
	float logicalX,
	float logicalY,
	int *windowX,
	int *windowY,
	bool integerScaling
);

void Win32HDRGetMousePosition(
	Win32HDRRenderer *renderer,
	int *logicalX,
	int *logicalY,
	bool integerScaling
);
