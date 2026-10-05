#pragma once
#include "User.h"
#include <ctime>

struct Comment
{
	ByteString authorName;
	User::Elevation authorElevation;
	bool authorIsSelf;
	bool authorIsBanned;
	String content;
	time_t timestamp;
};
