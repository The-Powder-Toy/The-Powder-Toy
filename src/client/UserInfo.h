#ifndef USERINFO_H_
#define USERINFO_H_

#include <string>
#include <utility>

class UserInfo
{
public:
	int UserID;
	int age;
	std::string username;
	std::string biography;
	std::string location;
	std::string website;

	int saveCount;
	float averageScore;
	int highestScore;

	int topicCount;
	int topicReplies;
	int reputation;

	UserInfo(int id, int age, std::string username, std::string biography, std::string location, std::string website, int saveCount, float averageScore, int highestScore, int topicCount, int topicReplies, int reputation):
		UserID(id),
		age(age),
		username(std::move(username)),
		biography(std::move(biography)),
		location(std::move(location)),
		website(std::move(website)),
		saveCount(saveCount),
		averageScore(averageScore),
		highestScore(highestScore),
		topicCount(topicCount),
		topicReplies(topicReplies),
		reputation(reputation)
	{ }
	UserInfo() = default;
};


#endif /* USER_H_ */
