#include "Platform.h"
#include <ctime>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <atomic>
#include <iostream>

static std::atomic<bool> shouldSyncFs = false;
static bool syncFsInFlight = false;

EMSCRIPTEN_KEEPALIVE extern "C" void Platform_SyncFsDone()
{
	syncFsInFlight = false;
}

namespace Platform
{
void OpenURI(ByteString uri)
{
	EM_ASM({
		open(UTF8ToString($0));
	}, uri.c_str());
}

void DoRestart()
{
	EM_ASM({
		location.reload();
	});
}

long unsigned int GetTime()
{
	struct timespec s;
	clock_gettime(CLOCK_MONOTONIC, &s);
	return s.tv_sec * 1000 + s.tv_nsec / 1000000;
}

ByteString ExecutableNameFirstApprox()
{
	return "powder.wasm"; // bogus
}

bool CanUpdate()
{
	return false;
}

bool Install()
{
	return false;
}

void Atexit(ExitFunc exitFunc)
{
}

void Exit(int code)
{
}

ByteString DefaultDdir()
{
	return "/powder";
}

int InvokeMain(int argc, char *argv[])
{
	EM_ASM({
		FS.syncfs(true, () => {
			Module.ccall('MainJs', 'number', [ 'number', 'number' ], [ $0, $1 ]);
		});
	}, argc, argv);
	return 0;
}

void MarkPresentable()
{
	EM_ASM({
		try {
			window.mark_presentable();
		} catch (e) {
		}
	});
}

void MaybeTriggerSyncFs()
{
	if (!syncFsInFlight && shouldSyncFs.exchange(false, std::memory_order_relaxed))
	{
		std::cerr << "invoking FS.syncfs" << std::endl;
		syncFsInFlight = true;
		EM_ASM({
			FS.syncfs(false, err => {
				if (err) {
					console.error(err);
				}
				Module.ccall('Platform_SyncFsDone', null, [], []);
			});
		});
	}
}

ByteString ExecutableName()
{
	return DefaultDdir() + "/" + ExecutableNameFirstApprox(); // bogus
}

bool UpdateStart(const std::vector<char> &data)
{
	return false;
}

bool UpdateFinish()
{
	return false;
}

void UpdateCleanup()
{
}
}

EMSCRIPTEN_KEEPALIVE extern "C" int MainJs(int argc, char *argv[])
{
	return Main(argc, argv);
}

EMSCRIPTEN_KEEPALIVE extern "C" void Platform_ShouldSyncFs()
{
	shouldSyncFs.store(true, std::memory_order_relaxed);
}

namespace Platform
{
	void SetupCrt()
	{
		EM_ASM({
			// If we don't castrate assert here, we get a crash from within the emscripten port of
			// SDL2 having to do with registering a callback that reports to the main thread, ever
			// since this "fix" https://github.com/emscripten-core/emscripten/pull/19691/files
			// TODO: Review later.
			assert = () => {};

			let ddir = UTF8ToString($0);
			let prefix = ddir + '/';
			let shouldSyncFs = Module.cwrap(
				'Platform_ShouldSyncFs',
				null,
				[]
			);
			FS.trackingDelegate['onMovePath'] = function(oldpath, newpath) {
				if (oldpath.startsWith(prefix) || newpath.startsWith(prefix)) {
					shouldSyncFs();
				}
			};
			FS.trackingDelegate['onDeletePath'] = function(path) {
				if (path.startsWith(prefix)) {
					shouldSyncFs();
				}
			};
			FS.trackingDelegate['onWriteToFile'] = function(path, bytesWritten) {
				if (path.startsWith(prefix)) {
					shouldSyncFs();
				}
			};
			FS.trackingDelegate['onMakeDirectory'] = function(path, mode) {
				if (path.startsWith(prefix)) {
					shouldSyncFs();
				}
			};
			FS.mkdir(ddir);
			FS.mount(IDBFS, {}, ddir);
		}, DefaultDdir().c_str());
	}
}
