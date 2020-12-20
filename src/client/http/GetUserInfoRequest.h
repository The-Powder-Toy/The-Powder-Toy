#ifndef GETUSERINFOREQUEST2_H
#define GETUSERINFOREQUEST2_H

#include "APIRequest.h"

class UserInfo;

namespace http
{
	class GetUserInfoRequest : public APIRequest
	{
	public:
		GetUserInfoRequest(ByteString username);
		virtual ~GetUserInfoRequest();

		std::unique_ptr<UserInfo> Finish();
	};
}

#endif // GETUSERINFOREQUEST2_H

