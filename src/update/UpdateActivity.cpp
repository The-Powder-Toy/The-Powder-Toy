/*
 * UpdateActivity.cpp
 *
 *  Created on: Jun 20, 2012
 *      Author: Simon
 */

#include <bzlib.h>
#include <sstream>
#include "interface/Engine.h"
#include "UpdateActivity.h"
#include "tasks/Task.h"
#include "client/HTTP.h"
#include "Update.h"


class UpdateDownloadTask : public Task
{
public:
	UpdateDownloadTask(std::string updateName, UpdateActivity * a) : updateName(updateName), a(a) {};
private:
	UpdateActivity * a;
	std::string updateName;
	virtual void notifyDone()
	{
		a->NotifyDone(this);
	}
	virtual void doWork()
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
			if (data)
				free(data);
			errorStream << "Server responded with Status " << status;
			notifyError("Could not download update");
			return;
		}
		if (!data)
		{
			errorStream << "Server responded with nothing";
			notifyError("Server did not return any data");
			return;
		}

		notifyStatus("Unpacking update");
		notifyProgress(-1);

		int uncompressedLength;

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

		if (update_start(res, uncompressedLength))
		{
			update_cleanup();
			notifyError("Update failed - try downloading a new version.");
		}

		return;

	corrupt:
		notifyError("Downloaded update is corrupted\n" + errorStream.str());
		free(data);
		return;
	}
};

UpdateActivity::UpdateActivity() {
	char my_uri[] = "http://" SERVER "/Update.api?Action=Download&Architecture="
	#if defined WIN32
	                "Windows32"
	#elif defined LIN32
	                "Linux32"
	#elif defined LIN64
	                "Linux64"
	#elif defined MACOSX
	                "MacOSX"
	#else
	                "Unknown"
	#endif
	                "&InstructionSet="
	#if defined X86_SSE3
	                "SSE3"
	#elif defined X86_SSE2
	                "SSE2"
	#elif defined X86_SSE
	                "SSE"
	#else
	                "SSE"
	#endif
	                ;
	updateDownloadTask = new UpdateDownloadTask(my_uri, this);
	updateWindow = new TaskWindow("Downloading update...", updateDownloadTask, true);
}

void UpdateActivity::NotifyDone(Task * sender)
{
	updateWindow->Exit();
	ui::Engine::Ref().Exit();
	delete this;
}

UpdateActivity::~UpdateActivity() {
	// TODO Auto-generated destructor stub
}

