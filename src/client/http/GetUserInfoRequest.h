#ifndef GETUSERINFOREQUEST2_H
#define GETUSERINFOREQUEST2_H

#include "Request.h"
#include "common/String.h"
#include "json/json.h"
#include "client/Client.h"
#include "APIRequest.h"

#include <memory>
#include <map>

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

