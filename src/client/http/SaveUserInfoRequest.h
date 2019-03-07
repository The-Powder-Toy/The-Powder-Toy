#ifndef SAVEUSERINFOREQUEST2_H
#define SAVEUSERINFOREQUEST2_H

#include "Request.h"
#include "common/String.h"
#include "json/json.h"
#include "client/Client.h"
#include "APIRequest.h"

#include <memory>
#include <map>

namespace http
{
	class SaveUserInfoRequest : public APIRequest
	{
	public:
		SaveUserInfoRequest(UserInfo info);
		virtual ~SaveUserInfoRequest();

		bool Finish();
	};
}

#endif // SAVEUSERINFOREQUEST2_H

