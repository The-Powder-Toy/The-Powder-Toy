#include "UpdateActivity.h"
#include "client/http/Request.h"
#include "prefs/GlobalPrefs.h"
#include "common/platform/Platform.h"
#include "tasks/Task.h"
#include "tasks/TaskWindow.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Engine.h"
#include "Config.h"
#include <bzlib.h>
#include <memory>

class UpdateDownloadTask : public Task
{
public:
	UpdateDownloadTask(ByteString updateName, UpdateActivity * a) : a(a), updateName(updateName) {}
private:
	UpdateActivity * a;
	ByteString updateName;
	void notifyDoneMain() override {
		a->NotifyDone(this);
	}
	void notifyErrorMain() override
	{
		a->NotifyError(this);
	}
	bool doWork() override
	{
		auto &prefs = GlobalPrefs::Ref();

		auto niceNotifyError = [this](String error) {
			notifyError("Downloaded update is corrupted\n" + error);
			return false;
		};

		auto request = std::make_unique<http::Request>(updateName);
		request->Start();
		notifyStatus("Downloading update");
		notifyProgress(-1);
		while(!request->CheckDone())
		{
			int64_t total, done;
			std::tie(total, done) = request->CheckProgress();
			if (total == -1)
			{
				notifyProgress(-1);
			}
			else
			{
				notifyProgress(total ? done * 100 / total : 0);
			}
			Platform::Millisleep(1);
		}

		int status;
		ByteString data;
		try
		{
			std::tie(status, data) = request->Finish();
		}
		catch (const http::RequestError &ex)
		{
			return niceNotifyError("Could not download update: " + String::Build("Server responded with Status ", ByteString(ex.what()).FromAscii()));
		}
		if (status!=200)
		{
			return niceNotifyError("Could not download update: " + String::Build("Server responded with Status ", status));
		}
		if (!data.size())
		{
			return niceNotifyError("Server did not return any data");
		}

		notifyStatus("Unpacking update");
		notifyProgress(-1);

		unsigned int uncompressedLength;

		if(data.size()<16)
		{
			return niceNotifyError(String::Build("Unsufficient data, got ", data.size(), " bytes"));
		}
		if (data[0]!=0x42 || data[1]!=0x75 || data[2]!=0x54 || data[3]!=0x54)
		{
			return niceNotifyError("Invalid update format");
		}

		uncompressedLength  = (unsigned char)data[4];
		uncompressedLength |= ((unsigned char)data[5])<<8;
		uncompressedLength |= ((unsigned char)data[6])<<16;
		uncompressedLength |= ((unsigned char)data[7])<<24;

		std::vector<char> res(uncompressedLength);

		int dstate;
		dstate = BZ2_bzBuffToBuffDecompress(&res[0], (unsigned *)&uncompressedLength, &data[8], data.size()-8, 0, 0);
		if (dstate)
		{
			return niceNotifyError(String::Build("Unable to decompress update: ", dstate));
		}

		notifyStatus("Applying update");
		notifyProgress(-1);

		prefs.Set("version.update", true);
		if (!Platform::UpdateStart(res))
		{
			prefs.Set("version.update", false);
			Platform::UpdateCleanup();
			notifyError("Update failed - try downloading a new version.");
			return false;
		}

		return true;
	}
};

UpdateActivity::UpdateActivity(UpdateInfo info)
{
	updateDownloadTask = new UpdateDownloadTask(info.file, this);
	updateWindow = new TaskWindow("Downloading update...", updateDownloadTask, true);
}

void UpdateActivity::NotifyDone(Task * sender)
{
	if(sender->GetSuccess())
	{
		Exit();
	}
}

void UpdateActivity::Exit()
{
	updateWindow->Exit();
	ui::Engine::Ref().Exit();
	delete this;
}

void UpdateActivity::NotifyError(Task * sender)
{
	StringBuilder sb;
	if constexpr (USE_UPDATESERVER)
	{
		sb << "Please go online to manually download a newer version.\n";
	}
	else
	{
		sb << "Please visit the website to download a newer version.\n";
	}
	sb << "Error: " << sender->GetError();
	new ConfirmPrompt("Autoupdate failed", sb.Build(), { [this] {
		if constexpr (!USE_UPDATESERVER)
		{
			Platform::OpenURI(ByteString(SCHEME) + "powdertoy.co.uk/Download.html");
		}
		Exit();
	}, [this] { Exit(); } });
}


UpdateActivity::~UpdateActivity() {
}
