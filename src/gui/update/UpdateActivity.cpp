#include "UpdateActivity.h"

#include "client/http/Request.h"
#include "bzip2/bzlib.h"

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
		http::Request *request = new http::Request(updateName);
		request->Start();
		notifyStatus("Downloading update");
		notifyProgress(-1);
		while(!request->CheckDone())
		{
			int total, done;
			request->CheckProgress(&total, &done);
			notifyProgress(total ? done * 100 / total : 0);
			Platform::Millisleep(1);
		}

		int status;
		ByteString data = request->Finish(&status);
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
	ByteString file;
#ifdef UPDATESERVER
	file = ByteString::Build(SCHEME, UPDATESERVER, Client::Ref().GetUpdateInfo().File);
#else
	file = ByteString::Build(SCHEME, SERVER, Client::Ref().GetUpdateInfo().File);
#endif
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
#ifdef UPDATESERVER
# define FIRST_LINE "Please go online to manually download a newer version.\n"
#else
# define FIRST_LINE "Please visit the website to download a newer version.\n"
#endif
	new ConfirmPrompt("Autoupdate failed", FIRST_LINE "Error: " + sender->GetError(), { [this] {
#ifndef UPDATESERVER
		Platform::OpenURI(SCHEME "powdertoy.co.uk/Download.html");
#endif
		Exit();
	}, [this] { Exit(); } });
#undef FIRST_LINE
}


UpdateActivity::~UpdateActivity() {
}

