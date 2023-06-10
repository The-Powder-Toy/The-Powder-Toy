#pragma once
#include "User.h"
#include "ServerNotification.h"
#include <vector>

struct LoginInfo
{
	User user;
	std::vector<ServerNotification> notifications;
};
