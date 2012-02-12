#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <time.h>

#include "Config.h"
#include "Client.h"
#include "MD5.h"
#include "Graphics.h"
#include "Misc.h"

#include "interface/Point.h"

#include "search/Save.h"

Client::Client():
	authUser(0, "")
{
	int i = 0;
	std::string proxyString("");
	for(i = 0; i < THUMB_CACHE_SIZE; i++)
	{
		thumbnailCache[i] = NULL;
	}
	for(i = 0; i < IMGCONNS; i++)
	{
		activeThumbRequests[i] = NULL;
		activeThumbRequestTimes[i] = 0;
		activeThumbRequestCompleteTimes[i] = 0;
	}

	//Read config
	std::ifstream configFile;
	configFile.open("powder.pref", ios::binary);
	if(configFile)
	{
		int fsize = configFile.tellg();
		configFile.seekg(0, std::ios::end);
		fsize = configFile.tellg() - fsize;
		configFile.seekg(0, ios::beg);
		if(fsize)
		{
			json::Reader::Read(configDocument, configFile);
			try
			{
				authUser.ID = ((json::Number)(configDocument["User"]["ID"])).Value();
				authUser.SessionID = ((json::String)(configDocument["User"]["SessionID"])).Value();
				authUser.SessionKey = ((json::String)(configDocument["User"]["SessionKey"])).Value();
				authUser.Username = ((json::String)(configDocument["User"]["Username"])).Value();
			}
			catch (json::Exception &e)
			{
				authUser = User(0, "");
				std::cerr << "Error: Client [Read User data from pref] " << e.what() << std::endl;
			}
			try
			{
				proxyString = ((json::String)(configDocument["Proxy"])).Value();
			}
			catch (json::Exception &e)
			{
				proxyString = "";
				std::cerr << "Error: Client [Read Proxy from pref] " << e.what() << std::endl;
			}
		}
		configFile.close();
	}

	if(proxyString.length())
	{
		http_init((char *)proxyString.c_str());
	}
	else
	{
		http_init(NULL);
	}
}

Client::~Client()
{
	ClearThumbnailRequests();
	http_done();

	//Save config
	std::ofstream configFile;
	configFile.open("powder.pref", ios::trunc);
	if(configFile)
	{
		if(authUser.ID)
		{
			configDocument["User"]["ID"] = json::Number(authUser.ID);
			configDocument["User"]["SessionID"] = json::String(authUser.SessionID);
			configDocument["User"]["SessionKey"] = json::String(authUser.SessionKey);
			configDocument["User"]["Username"] = json::String(authUser.Username);
		}
		else
		{
			configDocument["User"] = json::Null();
		}
		json::Writer::Write(configDocument, configFile);
		configFile.close();
	}
}


void Client::SetAuthUser(User user)
{
	authUser = user;
}

User Client::GetAuthUser()
{
	return authUser;
}

RequestStatus Client::UploadSave(Save * save)
{
	lastError = "";
	int dataStatus;
	char * data;
	int dataLength = 0;
	std::stringstream userIDStream;
	userIDStream << authUser.ID;
	if(authUser.ID)
	{
		char * postNames[] = { "Name", "Description", "Data:save.bin", "Publish", NULL };
		char * postDatas[] = { (char *)(save->name.c_str()), (char *)(save->Description.c_str()), (char *)(save->GetData()), (char *)(save->Published?"Public":"Private") };
		int postLengths[] = { save->name.length(), save->Description.length(), save->GetDataLength(), save->Published?6:7 };
		//std::cout << postNames[0] << " " << postDatas[0] << " " << postLengths[0] << std::endl;
		data = http_multipart_post("http://" SERVER "/Save.api", postNames, postDatas, postLengths, (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	if(data && dataStatus == 200)
	{
		if(strncmp((const char *)data, "OK", 2)!=0)
		{
			free(data);
			lastError = std::string((const char *)data);
			return RequestFailure;
		}
		else
		{
			int tempID;
			std::stringstream saveIDStream((char *)(data+3));
			saveIDStream >> tempID;
			if(!tempID)
			{
				lastError = "Server did not return Save ID";
				return RequestFailure;
			}
			else
			{
				save->id = tempID;
			}
		}
		free(data);
		return RequestOkay;
	}
	else if(data)
	{
		free(data);
	}
	return RequestFailure;
}

RequestStatus Client::ExecVote(int saveID, int direction)
{
	lastError = "";
	int dataStatus;
	char * data;
	int dataLength = 0;
	std::stringstream idStream;
	idStream << saveID;
	std::string directionS;
	if(direction==1)
	{
		directionS = "Up";
	}
	else
	{
		directionS = "Down";
	}
	std::stringstream userIDStream;
	userIDStream << authUser.ID;
	if(authUser.ID)
	{
		char * postNames[] = { "ID", "Action", NULL };
		char * postDatas[] = { (char*)(idStream.str().c_str()), (char*)(directionS.c_str()) };
		int postLengths[] = { idStream.str().length(), directionS.length() };
		//std::cout << postNames[0] << " " << postDatas[0] << " " << postLengths[0] << std::endl;
		data = http_multipart_post("http://" SERVER "/Vote.api", postNames, postDatas, postLengths, (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		lastError = "Not authenticated";
		return RequestFailure;
	}
	if(data && dataStatus == 200)
	{
		if(strncmp((const char *)data, "OK", 2)!=0)
		{
			free(data);
			lastError = std::string((const char *)data);
			return RequestFailure;
		}
		free(data);
		return RequestOkay;
	}
	else if(data)
	{
		free(data);
	}
	lastError = http_ret_text(dataStatus);
	return RequestFailure;
}

unsigned char * Client::GetSaveData(int saveID, int saveDate, int & dataLength)
{
	lastError = "";
	int dataStatus;
	unsigned char * data;
	dataLength = 0;
	std::stringstream urlStream;
	if(saveDate)
	{
		urlStream << "http://" << STATICSERVER << "/" << saveID << "_" << saveDate << ".cps";
	}
	else
	{
		urlStream << "http://" << STATICSERVER << "/" << saveID << ".cps";
	}
	std::cout << urlStream.str() << std::endl;
	data = (unsigned char *)http_simple_get((char *)urlStream.str().c_str(), &dataStatus, &dataLength);
	if(data && dataStatus == 200)
	{
		return data;
	}
	else if(data)
	{
		free(data);
	}
	return NULL;
}

LoginStatus Client::Login(string username, string password, User & user)
{
	lastError = "";
	std::stringstream urlStream;
	std::stringstream hashStream;
	char passwordHash[33];
	char totalHash[33];

	user.ID = 0;
	user.Username = "";
	user.SessionID = "";
	user.SessionKey = "";

	//Doop
	md5_ascii(passwordHash, (const unsigned char *)password.c_str(), password.length());
	passwordHash[32] = 0;
	hashStream << username << "-" << passwordHash;
	md5_ascii(totalHash, (const unsigned char *)(hashStream.str().c_str()), hashStream.str().length());
	totalHash[32] = 0;

	char * data;
	int dataStatus, dataLength;
	char * postNames[] = { "Username", "Hash", NULL };
	char * postDatas[] = { (char*)username.c_str(), totalHash };
	int postLengths[] = { username.length(), 32 };
	data = http_multipart_post("http://" SERVER "/Login.json", postNames, postDatas, postLengths, NULL, NULL, NULL, &dataStatus, &dataLength);
	//data = http_auth_get("http://" SERVER "/Login.json", (char*)username.c_str(), (char*)password.c_str(), NULL, &dataStatus, &dataLength);
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);
			json::Number tempStatus = objDocument["Status"];

			free(data);
			if(tempStatus.Value() == 1)
			{
				json::Number userIDTemp = objDocument["UserID"];
				json::String sessionIDTemp = objDocument["SessionID"];
				json::String sessionKeyTemp = objDocument["SessionKey"];
				user.Username = username;
				user.ID = userIDTemp.Value();
				user.SessionID = sessionIDTemp.Value();
				user.SessionKey = sessionKeyTemp.Value();
				return LoginOkay;
			}
			else
			{
				json::String tempError = objDocument["Error"];
				lastError = tempError.Value();
				return LoginError;
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Server responded with crap";
			return LoginError;
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	if(data)
	{
		free(data);
	}
	return LoginError;
}

Save * Client::GetSave(int saveID, int saveDate)
{
	lastError = "";
	std::stringstream urlStream;
	urlStream << "http://" << SERVER  << "/Browse/View.json?ID=" << saveID;
	if(saveDate)
	{
		urlStream << "&Date=" << saveDate;
	}
	char * data;
	int dataStatus, dataLength;
	//Save(int _id, int _votesUp, int _votesDown, string _userName, string _name, string description_, string date_, bool published_):
	if(authUser.ID)
	{
		std::stringstream userIDStream;
		userIDStream << authUser.ID;
		data = http_auth_get((char *)urlStream.str().c_str(), (char *)(userIDStream.str().c_str()), NULL, (char *)(authUser.SessionID.c_str()), &dataStatus, &dataLength);
	}
	else
	{
		data = http_simple_get((char *)urlStream.str().c_str(), &dataStatus, &dataLength);
	}
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			json::Number tempID = objDocument["ID"];
			json::Number tempScoreUp = objDocument["ScoreUp"];
			json::Number tempScoreDown = objDocument["ScoreDown"];
			json::Number tempMyScore = objDocument["ScoreMine"];
			json::String tempUsername = objDocument["Username"];
			json::String tempName = objDocument["Name"];
			json::String tempDescription = objDocument["Description"];
			json::Number tempDate = objDocument["Date"];
			json::Boolean tempPublished = objDocument["Published"];
			return new Save(
					tempID.Value(),
					tempDate.Value(),
					tempScoreUp.Value(),
					tempScoreDown.Value(),
					tempMyScore.Value(),
					tempUsername.Value(),
					tempName.Value(),
					tempDescription.Value(),
					tempPublished.Value()
					);
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
			return NULL;
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	return NULL;
}

Thumbnail * Client::GetPreview(int saveID, int saveDate)
{
	std::stringstream urlStream;
	urlStream << "http://" << STATICSERVER  << "/" << saveID;
	if(saveDate)
	{
		urlStream << "_" << saveDate;
	}
	urlStream << "_large.pti";
	pixel * thumbData;
	char * data;
	int status, data_size, imgw, imgh;
	data = http_simple_get((char *)urlStream.str().c_str(), &status, &data_size);
	if (status == 200 && data)
	{
		thumbData = Graphics::ptif_unpack(data, data_size, &imgw, &imgh);
		if(data)
		{
			free(data);
		}
		if(thumbData)
		{
			return new Thumbnail(saveID, saveDate, thumbData, ui::Point(imgw, imgh));
		}
		else
		{
			return new Thumbnail(saveID, saveDate, (pixel *)malloc((128*128) * PIXELSIZE), ui::Point(128, 128));
		}
	}
	else
	{
		if(data)
		{
			free(data);
		}
		return new Thumbnail(saveID, saveDate, (pixel *)malloc((128*128) * PIXELSIZE), ui::Point(128, 128));
	}
}

std::vector<Comment*> * Client::GetComments(int saveID, int start, int count)
{
	lastError = "";
	std::vector<Comment*> * commentArray = new std::vector<Comment*>();

	std::stringstream urlStream;
	char * data;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse/View.json?ID=" << saveID << "&Mode=Comments&Start=" << start << "&Count=" << count;
	data = http_simple_get((char *)urlStream.str().c_str(), &dataStatus, &dataLength);
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Array commentsArray;
			json::Reader::Read(commentsArray, dataStream);

			for(int j = 0; j < commentsArray.Size(); j++)
			{
				json::Number tempUserID = commentsArray[j]["UserID"];
				json::String tempUsername = commentsArray[j]["Username"];
				json::String tempComment = commentsArray[j]["Text"];
				commentArray->push_back(
							new Comment(
								tempUserID.Value(),
								tempUsername.Value(),
								tempComment.Value()
								)
							);
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	if(data)
		free(data);
	return commentArray;
}

std::vector<Save*> * Client::SearchSaves(int start, int count, string query, string sort, bool showOwn, int & resultCount)
{
	lastError = "";
	resultCount = 0;
	std::vector<Save*> * saveArray = new std::vector<Save*>();
	std::stringstream urlStream;
	char * data;
	int dataStatus, dataLength;
	urlStream << "http://" << SERVER << "/Browse.json?Start=" << start << "&Count=" << count;
	if(query.length() || sort.length())
	{
		urlStream << "&Search_Query=";
		if(query.length())
			urlStream << URLEscape(query);
		if(sort == "date")
		{
			if(query.length())
				urlStream << URLEscape(" ");
			urlStream << URLEscape("sort:") << URLEscape(sort);
		}
		if(showOwn && authUser.ID)
		{
			if(query.length())
				urlStream << URLEscape(" ");
			urlStream << URLEscape("user:") << URLEscape(authUser.Username);
		}

	}
	data = http_simple_get((char *)urlStream.str().c_str(), &dataStatus, &dataLength);
	if(dataStatus == 200 && data)
	{
		try
		{
			std::istringstream dataStream(data);
			json::Object objDocument;
			json::Reader::Read(objDocument, dataStream);

			json::Number tempCount = objDocument["Count"];
			resultCount = tempCount.Value();
			json::Array savesArray = objDocument["Saves"];
			for(int j = 0; j < savesArray.Size(); j++)
			{
				json::Number tempID = savesArray[j]["ID"];
				json::Number tempDate = savesArray[j]["Date"];
				json::Number tempScoreUp = savesArray[j]["ScoreUp"];
				json::Number tempScoreDown = savesArray[j]["ScoreDown"];
				json::String tempUsername = savesArray[j]["Username"];
				json::String tempName = savesArray[j]["Name"];
				saveArray->push_back(
							new Save(
								tempID.Value(),
								tempDate.Value(),
								tempScoreUp.Value(),
								tempScoreDown.Value(),
								tempUsername.Value(),
								tempName.Value()
								)
							);
			}
		}
		catch (json::Exception &e)
		{
			lastError = "Could not read response";
		}
	}
	else
	{
		lastError = http_ret_text(dataStatus);
	}
	if(data)
		free(data);
	return saveArray;
}

void Client::ClearThumbnailRequests()
{
	for(int i = 0; i < IMGCONNS; i++)
	{
		if(activeThumbRequests[i])
		{
			http_async_req_close(activeThumbRequests[i]);
			activeThumbRequests[i] = NULL;
			activeThumbRequestTimes[i] = 0;
			activeThumbRequestCompleteTimes[i] = 0;
		}
	}
}

Thumbnail * Client::GetThumbnail(int saveID, int saveDate)
{
	std::stringstream urlStream;
	std::stringstream idStream;
	int i = 0, currentTime = time(NULL);
	//Check active requests for any "forgotten" requests
	for(i = 0; i < IMGCONNS; i++)
	{
		//If the request is active, and we've recieved a response
		if(activeThumbRequests[i] && http_async_req_status(activeThumbRequests[i]))
		{
			//If we haven't already, mark the request as completed
			if(!activeThumbRequestCompleteTimes[i])
			{
				activeThumbRequestCompleteTimes[i] = time(NULL);
			}
			else if(activeThumbRequestCompleteTimes[i] < (currentTime-2)) //Otherwise, if it completed more than 2 seconds ago, destroy it.
			{
				http_async_req_close(activeThumbRequests[i]);
				activeThumbRequests[i] = NULL;
				activeThumbRequestTimes[i] = 0;
				activeThumbRequestCompleteTimes[i] = 0;
			}
		}
	}
	for(i = 0; i < THUMB_CACHE_SIZE; i++)
	{
		if(thumbnailCache[i] && thumbnailCache[i]->ID == saveID && thumbnailCache[i]->Datestamp == saveDate)
			return thumbnailCache[i];
	}
	urlStream << "http://" << STATICSERVER  << "/" << saveID;
	if(saveDate)
	{
		urlStream << "_" << saveDate;
	}
	urlStream << "_small.pti";
	idStream << saveID << ":" << saveDate;
	std::string idString = idStream.str();
	bool found = false;
	for(i = 0; i < IMGCONNS; i++)
	{
		if(activeThumbRequests[i] && activeThumbRequestIDs[i] == idString)
		{
			found = true;
			if(http_async_req_status(activeThumbRequests[i]))
			{
				pixel * thumbData;
				char * data;
				int status, data_size, imgw, imgh;
				data = http_async_req_stop(activeThumbRequests[i], &status, &data_size);
				free(activeThumbRequests[i]);
				activeThumbRequests[i] = NULL;
				if (status == 200 && data)
				{
					thumbData = Graphics::ptif_unpack(data, data_size, &imgw, &imgh);
					if(data)
					{
						free(data);
					}
					thumbnailCacheNextID %= THUMB_CACHE_SIZE;
					if(thumbnailCache[thumbnailCacheNextID])
					{
						delete thumbnailCache[thumbnailCacheNextID];
					}
					if(thumbData)
					{
						thumbnailCache[thumbnailCacheNextID] = new Thumbnail(saveID, saveDate, thumbData, ui::Point(imgw, imgh));
					}
					else
					{
						thumbnailCache[thumbnailCacheNextID] = new Thumbnail(saveID, saveDate, (pixel *)malloc((128*128) * PIXELSIZE), ui::Point(128, 128));
					}
					return thumbnailCache[thumbnailCacheNextID++];
				}
				else
				{
					if(data)
					{
						free(data);
					}
					thumbnailCacheNextID %= THUMB_CACHE_SIZE;
					if(thumbnailCache[thumbnailCacheNextID])
					{
						delete thumbnailCache[thumbnailCacheNextID];
					}
					thumbnailCache[thumbnailCacheNextID] = new Thumbnail(saveID, saveDate, (pixel *)malloc((128*128) * PIXELSIZE), ui::Point(128, 128));
					return thumbnailCache[thumbnailCacheNextID++];
				}
			}
		}
	}
	if(!found)
	{
		for(i = 0; i < IMGCONNS; i++)
		{
			if(!activeThumbRequests[i])
			{
				activeThumbRequests[i] = http_async_req_start(NULL, (char *)urlStream.str().c_str(), NULL, 0, 1);
				activeThumbRequestTimes[i] = currentTime;
				activeThumbRequestCompleteTimes[i] = 0;
				activeThumbRequestIDs[i] = idString;
				return NULL;
			}
		}
	}
	//http_async_req_start(http, urlStream.str().c_str(), NULL, 0, 1);
	return NULL;
}
