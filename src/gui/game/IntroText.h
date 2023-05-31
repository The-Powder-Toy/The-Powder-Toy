#pragma once
#include "Config.h"
#include "common/String.h"

inline ByteString IntroText()
{
	ByteStringBuilder sb;
	sb << "\bl\bU" << APPNAME << "\bU - Version " << SAVE_VERSION << "." << MINOR_VERSION << " - https://powdertoy.co.uk, irc.libera.chat #powder, https://tpt.io/discord\n"
	      "\n"
	      "\n"
	      "\bgControl+C/V/X are Copy, Paste and cut respectively.\n"
	      "\bgTo choose a material, hover over one of the icons on the right, it will show a selection of elements in that group.\n"
	      "\bgPick your material from the menu using mouse left/right buttons.\n"
	      "Draw freeform lines by dragging your mouse left/right button across the drawing area.\n"
	      "Shift+drag will create straight lines of particles.\n"
	      "Ctrl+drag will result in filled rectangles.\n"
	      "Ctrl+Shift+click will flood-fill a closed area.\n"
	      "Use the mouse scroll wheel, or '[' and ']', to change the tool size for particles. Use 'Tab' to cycle brushes.\n"
	      "Middle click or Alt+Click to \"sample\" the particles.\n"
	      "Ctrl+Z will act as Undo.\n"
	      "\n\boUse 'Z' for a zoom tool. Click to make the drawable zoom window stay around. Use the wheel to change the zoom strength.\n"
	      "The spacebar can be used to pause physics. Use 'F' to step ahead by one frame.\n"
	      "Use 'S' to save parts of the window as 'stamps'. 'L' loads the most recent stamp, 'K' shows a library of stamps you saved.\n"
	      "Use 'P' to take a screenshot and save it into the current directory.\n"
	      "Use 'H' to toggle the HUD. Use 'D' to toggle debug mode in the HUD.\n"
	      "\n"
	      "Contributors: \bgStanislaw K Skowronek (Designed the original Powder Toy),\n"
	      "\bgSimon Robertshaw, Skresanov Savely, cracker64, Catelite, Victoria Hoyle, Nathan Cousins, jacksonmj,\n"
	      "\bgFelix Wallin, Lieuwe Mosch, Anthony Boot, Me4502, MaksProg, jacob1, mniip, LBPHacker\n"
	      "\n";
	if constexpr (BETA)
	{
		sb << "\brThis is a BETA, you cannot save things publicly, nor open local saves and stamps made with it in older versions.\n"
		      "\brIf you are planning on publishing any saves, use the release version.\n";
	}
	else
	{
		sb << "\bgTo use online features such as saving, you need to register at: \brhttps://powdertoy.co.uk/Register.html\n";
	}
	sb << "\n"
	   << "\bt" << SAVE_VERSION << "." << MINOR_VERSION << "." << BUILD_NUM << " " << IDENT;
	if constexpr (SNAPSHOT)
	{
		sb << " SNAPSHOT " << SNAPSHOT_ID;
	}
	else if constexpr (MOD)
	{
		sb << " MODVER " << SNAPSHOT_ID;
	}
	if constexpr (LUACONSOLE)
	{
		sb << " LUACONSOLE";
	}
#ifdef REALISTIC
	sb << " REALISTIC";
#endif
	if constexpr (NOHTTP)
	{
		sb << " NOHTTP";
	}
	if constexpr (DEBUG)
	{
		sb << " DEBUG";
	}
	if constexpr (ENFORCE_HTTPS)
	{
		sb << " HTTPS";
	}
	return sb.Build();
}
