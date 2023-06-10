#pragma once
#include "User.h"

struct Comment
{
	ByteString authorName;
	User::Elevation authorElevation;
	bool authorIsSelf;
	bool authorIsBanned;
	String content;
};
