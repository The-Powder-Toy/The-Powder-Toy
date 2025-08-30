#pragma once
#include "common/String.h"
#include <optional>

struct UserInfo
{
	int UserID;
	int age;
	ByteString username;
	String biography;
	String location;
	ByteString website;

	int saveCount;
	float averageScore;
	int highestScore;

	int topicCount;
	int topicReplies;
	int reputation;
	std::optional<time_t> registeredAt;
};
