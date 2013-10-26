static const char *introTextData =
	"\blThe Powder Toy - Version " MTOS(SAVE_VERSION) "." MTOS(MINOR_VERSION) " - http://powdertoy.co.uk, irc.freenode.net #powder\n"
	"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\n"
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
	"\n\boUse 'Z' for a zoom tool. Click to make the drawable zoom window stay around. Use the wheel to change the zoom strength\n"
	"The spacebar can be used to pause physics.\n"
	"Use 'S' to save parts of the window as 'stamps'.\n"
	"'L' will load the most recent stamp, 'K' shows a library of stamps you saved.\n"
	"'P' will take a screenshot and save it into the current directory.\n"
	"\n"
	"Contributors: \bgStanislaw K Skowronek (Designed the original Powder Toy),\n"
	"\bgSimon Robertshaw, Skresanov Savely, cracker64, Catelite, Bryan Hoyle, Nathan Cousins, jacksonmj,\n"
	"\bgFelix Wallin, Lieuwe Mosch, Anthony Boot, Matthew \"me4502\", MaksProg, jacob1, mniip\n"
	"\n"
#ifndef BETA
	"\bgTo use online features such as saving, you need to register at: \brhttp://powdertoy.co.uk/Register.html\n"
#else
	"\brThis is a BETA, you cannot save things publicly. If you are planning on publishing any saves, use the release version\n"
#endif
	"\n"
	"\bt" MTOS(SAVE_VERSION) "." MTOS(MINOR_VERSION) "." MTOS(BUILD_NUM) " " IDENT_PLATFORM " "
#ifdef X86
	"X86 "
#endif
#ifdef X86_SSE
	"X86_SSE "
#endif
#ifdef X86_SSE2
	"X86_SSE2 "
#endif
#ifdef X86_SSE3
	"X86_SSE3 "
#endif
#ifdef MACOSX
	"MACOSX "
#endif
#ifdef LUACONSOLE
	"LUACONSOLE "
#endif
#ifdef GRAVFFT
	"GRAVFFT "
#endif
#ifdef REALISTIC
	"REALISTIC"
#endif
	;
