#include "Platform.h"
#include "icon_cps_png.h"
#include "icon_exe_png.h"
#include "save_xml.h"
#include "powder_desktop.h"
#include "Config.h"
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#ifdef __FreeBSD__
# include <sys/sysctl.h>
#endif

static bool Run(const char *what, std::vector<ByteString> cmd, bool check)
{
	auto pid = fork();
	if (pid == -1)
	{
		std::cerr << what << ": fork failed: errno " << errno << ": " << strerror(errno) << std::endl;
		return false;
	}
	if (pid == 0)
	{
		std::vector<char *> argv;
		for (auto &p : cmd)
		{
			argv.push_back(const_cast<char *>(p.c_str())); // not really a problem hopefully
		}
		argv.push_back(nullptr);
		execvp(argv[0], argv.data());
		exit(1); // exec failed >_>
	}
	if (check)
	{
		int status;
		if (waitpid(pid, &status, 0) != pid)
		{
			std::cerr << what << ": child possibly failed: errno " << errno << ": " << strerror(errno) << std::endl;
			return false;
		}
		if (!WIFEXITED(status))
		{
			std::cerr << what << ": child possibly failed: waitpid status " << status << std::endl;
			return false;
		}
		if (WEXITSTATUS(status))
		{
			std::cerr << what << ": child failed: exit status " << WEXITSTATUS(status) << std::endl;
			return false;
		}
	}
	return true;
}

namespace Platform
{
void OpenURI(ByteString uri)
{
	Run("opening URI", { "xdg-open", uri }, false);
}

long unsigned int GetTime()
{
	struct timespec s;
	clock_gettime(CLOCK_MONOTONIC, &s);
	return s.tv_sec * 1000 + s.tv_nsec / 1000000;
}

ByteString ExecutableNameFirstApprox()
{
	if (Stat("/proc/self/exe"))
	{
		return "/proc/self/exe";
	}
#ifdef __FreeBSD__
	{
		int mib[4];
		mib[0] = CTL_KERN;
		mib[1] = KERN_PROC;
		mib[2] = KERN_PROC_PATHNAME;
		mib[3] = -1;
		std::array<char, 1000> buf;
		size_t cb = buf.size();
		if (!sysctl(mib, 4, buf.data(), &cb, NULL, 0))
		{
			return ByteString(buf.data(), buf.data() + cb);
		}
	}
#endif
	return "";
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
		auto data = powder_desktop.AsCharSpan();
		ByteString desktopData(data.begin(), data.end());
		auto exe = Platform::ExecutableName();
		auto path = Platform::GetCwd();
		desktopData = desktopData.Substitute("Exec=" + ByteString(APPEXE), "Exec=" + desktopEscapeString(desktopEscapeExec(exe)));
		desktopData += ByteString::Build("Path=", desktopEscapeString(path), "\n");
		ByteString file = ByteString::Build(APPVENDOR, "-", APPID, ".desktop");
		ok = ok && Platform::WriteFile(desktopData, file);
		ok = ok && Run("installing desktop file", { "xdg-desktop-menu", "install", file }, true);
		ok = ok && Run("associating save MIME type with desktop file", { "xdg-mime", "default", file, ByteString::Build("application/vnd.", APPVENDOR, ".save") }, true);
		ok = ok && Run("associating ptsave URL scheme with desktop file", { "xdg-mime", "default", file, "x-scheme-handler/ptsave" }, true);
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = ByteString(APPVENDOR) + "-save.xml";
		ok = ok && Platform::WriteFile(save_xml.AsCharSpan(), file);
		ok = ok && Run("associating saves and stamps with save MIME type with desktop file", { "xdg-mime", "install", file }, true);
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = ByteString(APPVENDOR) + "-cps.png";
		ok = ok && Platform::WriteFile(icon_cps_png.AsCharSpan(), file);
		ok = ok && Run("providing file icons for saves and stamps", { "xdg-icon-resource", "install", "--noupdate", "--context", "mimetypes", "--size", "64", file, ByteString::Build("application-vnd.", APPVENDOR, ".save") }, true);
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ByteString file = ByteString(APPVENDOR) + "-exe.png";
		auto vendoredName = ByteString::Build(APPVENDOR, "-", APPEXE);
		ok = ok && Platform::WriteFile(icon_exe_png.AsCharSpan(), file);
		ok = ok && Run("providing desktop file icon", { "xdg-icon-resource", "install", "--noupdate", "--size", "64", file, vendoredName }, true);
		Platform::RemoveFile(file);
	}
	if (ok)
	{
		ok = ok && Run("updating desktop icons", { "xdg-icon-resource", "forceupdate" }, true);
	}
	return ok;
}

void SetupCrt()
{
}
}
