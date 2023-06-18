#include "Platform.h"
#include "icon_cps.png.h"
#include "icon_exe.png.h"
#include "save.xml.h"
#include "powder.desktop.h"
#include "Config.h"
#include <cstring>
#include <ctime>

namespace Platform
{
void OpenURI(ByteString uri)
{
	if (system(("xdg-open \"" + uri + "\"").c_str()))
	{
		fprintf(stderr, "cannot open URI: system(...) failed\n");
	}
}

long unsigned int GetTime()
{
	struct timespec s;
	clock_gettime(CLOCK_MONOTONIC, &s);
	return s.tv_sec * 1000 + s.tv_nsec / 1000000;
}

ByteString ExecutableNameFirstApprox()
{
	return "/proc/self/exe";
}

bool CanUpdate()
{
	return true;
}

bool Install()
{
	bool ok = true;
	auto desktopEscapeString = [](ByteString str) {
		ByteString escaped;
		for (auto ch : str)
		{
			auto from = " " "\n" "\t" "\r" "\\";
			auto to   = "s"  "n"  "t"  "r" "\\";
			if (auto off = strchr(from, ch))
			{
				escaped.append(1, '\\');
				escaped.append(1, to[off - from]);
			}
			else
			{
				escaped.append(1, ch);
			}
		}
		return escaped;
	};
	auto desktopEscapeExec = [](ByteString str) {
		ByteString escaped;
		for (auto ch : str)
		{
			if (strchr(" \t\n\"\'\\><~|&;$*?#()`", ch))
			{
				escaped.append(1, '\\');
			}
			escaped.append(1, ch);
		}
		return escaped;
	};

	if (ok)
	{
		ByteString desktopData(powder_desktop, powder_desktop + powder_desktop_size);
		auto exe = Platform::ExecutableName();
		auto path = exe.SplitFromEndBy('/').Before();
		desktopData = desktopData.Substitute("Exec=" + ByteString(APPEXE), "Exec=" + desktopEscapeString(desktopEscapeExec(exe)));
		desktopData += ByteString::Build("Path=", desktopEscapeString(path), "\n");
		ByteString file = ByteString::Build(APPVENDOR, "-", APPID, ".desktop");
		ok = ok && Platform::WriteFile(std::vector<char>(desktopData.begin(), desktopData.end()), file);
		ok = ok && !system(ByteString::Build("xdg-desktop-menu install ", file).c_str());
		ok = ok && !system(ByteString::Build("xdg-mime default ", file, " application/vnd.powdertoy.save").c_str());
		ok = ok && !system(ByteString::Build("xdg-mime default ", file, " x-scheme-handler/ptsave").c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = ByteString(APPVENDOR) + "-save.xml";
		ok = ok && Platform::WriteFile(std::vector<char>(save_xml, save_xml + save_xml_size), file);
		ok = ok && !system(ByteString::Build("xdg-mime install ", file).c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = ByteString(APPVENDOR) + "-cps.png";
		ok = ok && Platform::WriteFile(std::vector<char>(icon_cps_png, icon_cps_png + icon_cps_png_size), file);
		ok = ok && !system(ByteString::Build("xdg-icon-resource install --noupdate --context mimetypes --size 64 ", file, " application-vnd.powdertoy.save").c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = ByteString(APPVENDOR) + "-exe.png";
		ok = ok && Platform::WriteFile(std::vector<char>(icon_exe_png, icon_exe_png + icon_exe_png_size), file);
		ok = ok && !system(ByteString::Build("xdg-icon-resource install --noupdate --size 64 ", file, " ", APPVENDOR, "-", APPEXE).c_str());
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ok = ok && !system("xdg-icon-resource forceupdate");
	}
	return ok;
}

void SetupCrt()
{
}
}
