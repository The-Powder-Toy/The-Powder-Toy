#include "client/http/Request.h" // includes curl.h, needs to come first to silence a warning on windows

#include "UpdateActivity.h"

#include <bzlib.h>
#include <memory>

#include "Config.h"
#include "Update.h"

#include "client/Client.h"
#include "common/Platform.h"
#include "tasks/Task.h"
#include "tasks/TaskWindow.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Engine.h"

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
		String error;
		auto request = std::make_unique<http::Request>(updateName);
		request->Start();
		notifyStatus("Downloading update");
		notifyProgress(-1);
		while(!request->CheckDone())
		{
			int total, done;
			std::tie(total, done) = request->CheckProgress();
			notifyProgress(total ? done * 100 / total : 0);
			Platform::Millisleep(1);
		}

		auto [ status, data ] = request->Finish();
		if (status!=200)
		{
			error = String::Build("Server responded with Status ", status);
			notifyError("Could not download update: " + error);
			return false;
		}
		if (!data.size())
		{
			error = "Server responded with nothing";
			notifyError("Server did not return any data");
			return false;
		}

		notifyStatus("Unpacking update");
		notifyProgress(-1);

		unsigned int uncompressedLength;

		if(data.size()<16)
		{
			error = String::Build("Unsufficient data, got ", data.size(), " bytes");
			goto corrupt;
		}
		if (data[0]!=0x42 || data[1]!=0x75 || data[2]!=0x54 || data[3]!=0x54)
		{
			error = "Invalid update format";
			goto corrupt;
		}

		uncompressedLength  = (unsigned char)data[4];
		uncompressedLength |= ((unsigned char)data[5])<<8;
		uncompressedLength |= ((unsigned char)data[6])<<16;
		uncompressedLength |= ((unsigned char)data[7])<<24;

		char * res;
		res = (char *)malloc(uncompressedLength);
		if (!res)
		{
			error = String::Build("Unable to allocate ", uncompressedLength, " bytes of memory for decompression");
			goto corrupt;
		}

		int dstate;
		dstate = BZ2_bzBuffToBuffDecompress((char *)res, (unsigned *)&uncompressedLength, &data[8], data.size()-8, 0, 0);
		if (dstate)
		{
			error = String::Build("Unable to decompress update: ", dstate);
			free(res);
			goto corrupt;
		}

		notifyStatus("Applying update");
		notifyProgress(-1);

		Client::Ref().SetPref("version.update", true);
		if (update_start(res, uncompressedLength))
		{
			Client::Ref().SetPref("version.update", false);
			update_cleanup();
			notifyError("Update failed - try downloading a new version.");
			return false;
		}

		return true;

	corrupt:
		notifyError("Downloaded update is corrupted\n" + error);
		return false;
	}
};

UpdateActivity::UpdateActivity() {
	ByteString file = ByteString::Build(SCHEME, USE_UPDATESERVER ? UPDATESERVER : SERVER, Client::Ref().GetUpdateInfo().File);
	updateDownloadTask = new UpdateDownloadTask(file, this);
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

