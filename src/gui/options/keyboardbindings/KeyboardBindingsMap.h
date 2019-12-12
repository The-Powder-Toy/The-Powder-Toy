#ifndef KEYBOARDBINDINGSMAP_H
#define KEYBOARDBINDINGSMAP_H

#include "common/String.h"

typedef struct KeyboardBindingMap
{
	int id;
	String description;
	int functionId;
} KeyboardBindingMap;

typedef struct DefaultKeyboardBindingMap
{
	ByteString keyCombo;
	int bindingId; // KeyboardBindingMap id
} DefaultKeyboardBindingMap;

static KeyboardBindingMap keyboardBindingFunctionMap[] = 
{
	{ 0x00, "Reload Simulation", 0 },
	{ 0x01, "Open Element Search", 1 },
	{ 0x02, "Toggle Air Mode", 2 },
	{ 0x03, "Toggle Heat", 3 },
	{ 0x04, "Toggle Newtonian Gravity", 4 },
	{ 0x05, "Open Stamps", 5 },
	{ 0x06, "Invert Air Simulation", 6 },
	{ 0x07, "Pause Simulation", 7 },
	{ 0x08, "Enable Zoom", 8 },
	{ 0x09, "Undo", 9 },
	{ 0x0A, "Redo", 10 },
	{ 0x0B, "Property Tool", 11 },
	{ 0x0C, "Property Tool", 11 },
	{ 0x0D, "Screenshot", 12 },
	{ 0x0E, "Toggle Debug HUD", 13 },
	{ 0x0F, "Save Authorship Info", 14 },
	{ 0x10, "Reload Simulation", 0 },
	{ 0x11, "Frame Step", 15 },
	{ 0x12, "Find Mode", 16 },
	{ 0x13, "Show Gravity Grid", 17 },
	{ 0x14, "Increase Gravity Grid Size", 18 },
	{ 0x15, "Decrease Gravity Grid Size", 19 },
	{ 0x16, "Toggle Intro Text", 20 },
	{ 0x17, "Toggle Intro Text", 20 },
	{ 0x18, "Toggle HUD", 21 },
	{ 0x19, "Toggle Decorations Layer", 22 },
	{ 0x1A, "Toggle Decoration Tool", 23 },
	{ 0x1B, "Redo", 10 },
	{ 0x1C, "Quit", 24 },
	{ 0x1D, "Quit", 24 },
	{ 0x1E, "Reset Spark", 25 },
	{ 0x1F, "Reset Air", 26 },
	{ 0x20, "Copy", 27 },
	{ 0x21, "Cut", 28 },
	{ 0x22, "Paste", 29 },
	{ 0x23, "Stamp Tool", 30 },
	{ 0x24, "Increase Brush Size", 31 },
	{ 0x25, "Decrease Brush Size", 32 },
	{ 0x26, "Install Game", 33 },
	{ 0x27, "Toggle Replace Mode", 34 },
	{ 0x28, "Toggle Specific Delete Mode", 35 },
	{ 0x29, "Toggle Console", 36 }
};

enum KeyboardBindingFunction 
{
	RELOAD_SIMULATION,
	OPEN_ELEMENT_SEARCH,
	TOGGLE_AIR_MODE,
	TOGGLE_HEAT,
	TOGGLE_NEWTONIAN_GRAVITY,
	OPEN_STAMPS,
	INVERT_AIR_SIMULATION,
	PAUSE_SIMULATION,
	ENABLE_ZOOM,
	UNDO,
	REDO,
	PROPERTY_TOOL,
	SCREENSHOT,
	TOGGLE_DEBUG_HUD,
	SAVE_AUTHORSHIP_INFO,
	FRAME_STEP,
	FIND_MODE,
	SHOW_GRAVITY_GRID,
	INCREASE_GRAVITY_GRID_SIZE,
	DECREASE_GRAVITY_GRID_SIZE,
	TOGGLE_INTRO_TEXT,
	TOGGLE_HUD,
	TOGGLE_DECORATIONS_LAYER,
	TOGGLE_DECORATION_TOOL,
	QUIT,
	RESET_SPARK,
	RESET_AIR,
	COPY,
	CUT,
	PASTE,
	STAMP_TOOL,
	INCREASE_BRUSH_SIZE,
	DECREASE_BRUSH_SIZE,
	INSTALL_GAME,
	TOGGLE_REPLACE_MODE,
	TOGGLE_SPECIFIC_DELETE_MODE,
	TOGGLE_CONSOLE
};

static DefaultKeyboardBindingMap defaultKeyboardBindingMapArray[] = 
{
	{ "0+62", 0x00 },
	{ "0+8", 0x01 },
	{ "0+28", 0x02 },
	{ "0+24", 0x03 },
	{ "0+17", 0x04 },
	{ "0+14", 0x05 },
	{ "0+12", 0x06 },
	{ "0+44", 0x07 },
	{ "0+29", 0x08 },
	{ "1+29", 0x09 },
	{ "5+29", 0x0A },
	{ "5+69", 0x0B },
	{ "1+69", 0x0C },
	{ "0+69", 0x0D },
	{ "0+60", 0x0E },
	{ "0+4", 0x0F },
	{ "1+21", 0x10 },
	{ "0+9", 0x11 },
	{ "1+9", 0x12 },
	{ "1+10", 0x13 },
	{ "0+10", 0x14 },
	{ "4+10", 0x15 },
	{ "0+58", 0x16 },
	{ "1+11", 0x17 },
	{ "0+11", 0x18 },
	{ "1+5", 0x19 },
	{ "0+5", 0x1A },
	{ "1+28", 0x1B },
	{ "0+41", 0x1C },
	{ "0+20", 0x1D },
	{ "1+46", 0x1E },
	{ "0+46", 0x1F },
	{ "1+6", 0x20 },
	{ "1+27", 0x21 },
	{ "1+25", 0x22 },
	{ "0+15", 0x23 },
	{ "0+48", 0x24 },
	{ "0+47", 0x25 },
	{ "1+12", 0x26 },
	{ "1+51", 0x27 },
	{ "0+51", 0x28 },
	{ "0+53", 0x29 }
};

#endif
