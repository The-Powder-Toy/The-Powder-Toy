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
	if constexpr (LATENTHEAT)
	{
		sb << " LATENTHEAT";
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
	      "\bgControl+C/V/X are Copy, Paste and cut respectively.\n"
	      "\bgTo choose a material, hover over one of the icons on the right, it will show a selection of elements in that group.\n"
	      "\bgPick your material from the menu using mouse left/right buttons.\n"
	      "Draw freeform lines by dragging your mouse left/right button across the drawing area.\n"
	      "Shift+drag will create straight lines of particles.\n"
	      "Ctrl+drag will result in filled rectangles.\n"
	      "Ctrl+Shift+click will flood-fill a closed area.\n"
	      "Use the mouse scroll wheel, or '[' and ']', to change the tool size for particles.\n"
	      "Middle click or Alt+Click to \"sample\" the particles.\n"
	      "Ctrl+Z will act as Undo.\n"
	      "\n\boUse 'Z' for a zoom tool. Click to make the drawable zoom window stay around. Use the wheel to change the zoom strength.\n"
	      "The spacebar can be used to pause physics. Use 'F' to step ahead by one frame.\n"
	      "Use 'S' to save parts of the window as 'stamps'. 'L' loads the most recent stamp, 'K' shows a library of stamps you saved.\n"
	      "Use 'P' to take a screenshot and save it into the current directory.\n"
	      "Use 'H' to toggle the HUD. Use 'D' to toggle debug mode in the HUD.\n"
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
