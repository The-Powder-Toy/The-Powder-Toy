#include <bzlib.h>
#include <sstream>
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/interface/Engine.h"
#include "UpdateActivity.h"
#include "tasks/Task.h"
#include "client/HTTP.h"
#include "client/Client.h"
#include "Update.h"
#include "Platform.h"


class UpdateDownloadTask : public Task
{
public:
	UpdateDownloadTask(std::string updateName, UpdateActivity * a) : a(a), updateName(updateName) {}
private:
	UpdateActivity * a;
	std::string updateName;
	virtual void notifyDoneMain(){
		a->NotifyDone(this);
	}
	virtual void notifyErrorMain()
	{
		a->NotifyError(this);
	}
	virtual bool doWork()
	{
		std::stringstream errorStream;
		void * request = http_async_req_start(NULL, (char*)updateName.c_str(), NULL, 0, 0);
		notifyStatus("Downloading update");
		notifyProgress(-1);
		while(!http_async_req_status(request))
		{
			int total, done;
			http_async_get_length(request, &total, &done);
			notifyProgress((float(done)/float(total))*100.0f);
		}

		char * data;
		int dataLength, status;
		data = http_async_req_stop(request, &status, &dataLength);
		if (status!=200)
		{
			free(data);
			errorStream << "Server responded with Status " << status;
			notifyError("Could not download update: " + errorStream.str());
			return false;
		}
		if (!data)
		{
			errorStream << "Server responded with nothing";
			notifyError("Server did not return any data");
			return false;
		}

		notifyStatus("Unpacking update");
		notifyProgress(-1);

		unsigned int uncompressedLength;

		if(dataLength<16)
		{
			errorStream << "Unsufficient data, got " << dataLength << " bytes";
			goto corrupt;
		}
		if (data[0]!=0x42 || data[1]!=0x75 || data[2]!=0x54 || data[3]!=0x54)
		{
			errorStream << "Invalid update format";
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
			errorStream << "Unable to allocate " << uncompressedLength << " bytes of memory for decompression";
			goto corrupt;
		}

		int dstate;
		dstate = BZ2_bzBuffToBuffDecompress((char *)res, (unsigned *)&uncompressedLength, (char *)(data+8), dataLength-8, 0, 0);
		if (dstate)
		{
			errorStream << "Unable to decompress update: " << dstate;
			free(res);
			goto corrupt;
		}

		free(data);

		notifyStatus("Applying update");
		notifyProgress(-1);

		Client::Ref().SetPref("version.update", true);
		Client::Ref().WritePrefs();
		if (update_start(res, uncompressedLength))
		{
			Client::Ref().SetPref("version.update", false);
			update_cleanup();
			notifyError("Update failed - try downloading a new version.");
			return false;
		}

		return true;

	corrupt:
		notifyError("Downloaded update is corrupted\n" + errorStream.str());
		free(data);
		return false;
	}
};

UpdateActivity::UpdateActivity() {
	std::stringstream file;
#ifdef UPDATESERVER
	file << "http://" << UPDATESERVER << Client::Ref().GetUpdateInfo().File;
#else
	file << "http://" << SERVER << Client::Ref().GetUpdateInfo().File;
#endif
	updateDownloadTask = new UpdateDownloadTask(file.str(), this);
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
	class ErrorMessageCallback: public ConfirmDialogueCallback
	{
		UpdateActivity * a;
	public:
		ErrorMessageCallback(UpdateActivity * a_) {	a = a_;	}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
			{
#ifndef UPDATESERVER
				Platform::OpenURI("http://powdertoy.co.uk/Download.html");
#endif
			}
			a->Exit();
		}
		virtual ~ErrorMessageCallback() { }
	};
#ifdef UPDATESERVER
	new ConfirmPrompt("Autoupdate failed", "Please go online to manually download a newer version.\nError: " + sender->GetError(), new ErrorMessageCallback(this));
#else
	new ConfirmPrompt("Autoupdate failed", "Please visit the website to download a newer version.\nError: " + sender->GetError(), new ErrorMessageCallback(this));
#endif
}


UpdateActivity::~UpdateActivity() {
}

