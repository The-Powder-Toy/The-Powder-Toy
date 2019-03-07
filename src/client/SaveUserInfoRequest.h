#ifndef SAVEUSERINFOREQUEST2_H
#define SAVEUSERINFOREQUEST2_H

#include "Download.h"
#include "common/String.h"
#include "json/json.h"
#include "client/Client.h"
#include "client/APIRequest.h"

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

