#pragma once
#include "Config.h"
#include "SimulationConfig.h"
#include "common/String.h"

inline ByteString VersionInfo()
{
	ByteStringBuilder sb;
	sb << DISPLAY_VERSION[0] << "." << DISPLAY_VERSION[1];
	if constexpr (!SNAPSHOT)
	{
		sb << "." << APP_VERSION.build;
	}
	sb << " " << IDENT;
	if constexpr (MOD)
	{
		sb << " MOD " << MOD_ID << " UPSTREAM " << UPSTREAM_VERSION.build;
	}
	if constexpr (SNAPSHOT)
	{
		sb << " SNAPSHOT " << APP_VERSION.build;
	}
	if constexpr (LUACONSOLE)
	{
		sb << " LUACONSOLE";
	}
	if constexpr (NOHTTP)
	{
		sb << " NOHTTP";
	}
	else if constexpr (ENFORCE_HTTPS)
	{
		sb << " HTTPS";
	}
	if constexpr (DEBUG)
	{
		sb << " DEBUG";
	}
	return sb.Build();
}

inline ByteString IntroText()
{
	ByteStringBuilder sb;
	sb << "\bl\bU" << APPNAME << "\bU - Version " << DISPLAY_VERSION[0] << "." << DISPLAY_VERSION[1] << " - https://powdertoy.co.uk, irc.libera.chat #powder, https://tpt.io/discord\n"
	      "\n"
	      "\n"
	      "\bgPress \bo'F1'\bg to show or hide this text.\n"
	      "\n"
	      "\bgTo choose a material, hover over one of the icons on the right, it will show a selection of elements in that group.\n"
	      "Pick your material from the menu using \bomouse left/right\bg buttons.\n"
	      "Draw freeform lines by dragging your mouse left/right button across the drawing area.\n"
	      "\n"
	      "Use the \bomouse scroll wheel\bg, or \bo'['\bg and \bo']'\bg, to change the tool size for particles. Press \boTab\bg to change the brush shape.\n"
	      "\boMiddle click\bg or \boAlt+click\bg to \"sample\" the particles.\n"
	      "\boCtrl+C/V/X\bg are copy, paste and cut respectively.\n"
	      "When pasting, use \bo'R'\bg to rotate, \boShift+R\bg and \boShift+Ctrl+R\bg to mirror vertically or horizontally.\n"
	      "\boShift+drag\bg will create straight lines of particles. \boShift+Alt+drag\bg for horizontal, vertical and diagonal lines.\n"
	      "\boCtrl+drag\bg will result in filled rectangles. \boCtrl+Alt+drag\bg for filled squares. \boCtrl+Shift+click\bg will flood-fill a closed area.\n"
	      "\n"
	      "\boSpacebar\bg can be used to pause physics. Use \bo'F'\bg to step ahead by one frame, \bo'F5'\bg to reload simulation.\n"
	      "\boCtrl+Z\bg will act as undo, \boCtrl+Y\bg or \boCtrl+Shift+Z\bg as redo.\n"
	      "Use \bo'S'\bg to save parts of the window as 'stamps'. \bo'L'\bg loads the most recent stamp, \bo'K'\bg shows a library of stamps you saved.\n"
	      "\n"
	      "Use \bo0-9\bg to select a view mode.\n"
	      "Use \bo'H'\bg to toggle the HUD. Use \bo'D'\bg to toggle debug mode in the HUD.\n"
	      "Use \bo'Z'\bg for a zoom tool. Click to make the drawable zoom window stay around. Use the wheel to change the zoom strength.\n"
	      "Use \boCtrl+F\bg to highlight a selected element on the screen.\n"
	      "\n";
	if constexpr (BETA)
	{
		sb << "\brThis is a BETA, you cannot save things publicly, nor open local saves and stamps made with it in older versions.\n"
		      "\brIf you are planning on publishing any saves, use the release version.\n";
	}
	else
	{
		sb << "\bgTo use online features such as saving, you need to register at: \br" << SERVER << "/Register.html\n";
	}
	sb << "\n\bt" << VersionInfo();
	return sb.Build();
}
