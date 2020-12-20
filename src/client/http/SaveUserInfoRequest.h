#ifndef SAVEUSERINFOREQUEST2_H
#define SAVEUSERINFOREQUEST2_H

#include "APIRequest.h"

class UserInfo;

namespace http
{
	class SaveUserInfoRequest : public APIRequest
	{
	public:
		SaveUserInfoRequest(UserInfo &info);
		virtual ~SaveUserInfoRequest();

		bool Finish();
	};
}

#endif // SAVEUSERINFOREQUEST2_H

