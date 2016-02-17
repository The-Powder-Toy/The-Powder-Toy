static const wchar_t *introTextData =
	L"\blThe Powder Toy - Version " MTOS(SAVE_VERSION) "." MTOS(MINOR_VERSION) " - http://powdertoy.co.uk, irc.freenode.net #powder\n"
	"\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\n"
	"\n"
	"\bgCtrl+C/V/X 复制，粘贴，剪切\n"
	"\bg鼠标移至右侧元素栏中某一个类别可以显示该类下的所有元素\n"
	"\bg利用鼠标左/右键点选你需要的元素\n"
	"使用鼠标左/右键绘制\n"
	"按住Shift键以绘制直线\n"
	"按住Ctrl键以绘制矩形\n"
	"按住Ctrl+Shift以填充\n"
	"利用鼠标滚轮或'['、']'键调整笔刷大小\n"
	"点击鼠标中键或按住Alt键以使用吸管工具\n"
	"Ctrl+Z 撤销\n"
	"\n\bo按住‘Z’键打开放大镜工具，利用鼠标滚轮调整放大倍数，鼠标左键单击可以固定放大区域，你可以直接在放大镜上绘制。\n"
	"按下空格键以暂停，按下‘F’可以帧进\n"
	"按下S键选择区域保存到剪贴板。按下L键加载最近保存的对象。按下K键浏览你的剪贴板\n"
	"按下P键截图，截图将被保存到游戏根目录下\n"
	"按下H键打开/关闭HUD显示。按下D键打开/关闭调试模式\n"
	"\n"
	"原作者： \bgStanislaw K Skowronek (Designed the original Powder Toy),\n"
	"\bgSimon Robertshaw, Skresanov Savely, cracker64, Catelite, Bryan Hoyle, Nathan Cousins, jacksonmj,\n"
	"\bgFelix Wallin, Lieuwe Mosch, Anthony Boot, Matthew \"me4502\", MaksProg, jacob1, mniip\n"
	"\n"
#ifndef BETA
	"\bg某些联网功能需要注册账号，转到： \brhttp://powdertoy.co.uk/Register.html了解更多\n"
#else
	"\br当前使用的是测试版本，如果你需要上传存档，请使用正式版本\n"
#endif
	"\n"
	"\bt" MTOS(SAVE_VERSION) "." MTOS(MINOR_VERSION) "." MTOS(BUILD_NUM) " " IDENT_PLATFORM " "
#ifdef SNAPSHOT
	"SNAPSHOT " MTOS(SNAPSHOT_ID) " "
#endif
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
